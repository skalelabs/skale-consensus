/*
    Copyright (C) 2018-2019 SKALE Labs

    This file is part of skale-consensus.

    skale-consensus is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    skale-consensus is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with skale-consensus.  If not, see <https://www.gnu.org/licenses/>.

    @file TransportNetwork.cpp
    @author Stan Kladko
    @date 2018
*/



#include <db/MsgDB.h>
#include "SkaleCommon.h"
#include "Log.h"
#include "thirdparty/json.hpp"
#include "abstracttcpserver/ConnectionStatus.h"
#include "blockproposal/pusher/BlockProposalClientAgent.h"
#include "db/BlockProposalDB.h"
#include "blockproposal/server/BlockProposalWorkerThreadPool.h"
#include "chains/Schain.h"
#include "crypto/ConsensusBLSSigShare.h"
#include "crypto/SHAHash.h"
#include "datastructures/BlockProposal.h"
#include "exceptions/FatalError.h"
#include "messages/NetworkMessage.h"
#include "node/Node.h"
#include "node/NodeInfo.h"
#include "protocols/binconsensus/AUXBroadcastMessage.h"
#include "protocols/binconsensus/BVBroadcastMessage.h"
#include "protocols/blockconsensus/BlockSignBroadcastMessage.h"
#include "thirdparty/json.hpp"

#include "unordered_set"


#include "exceptions/ExitRequestedException.h"
#include "exceptions/InvalidMessageFormatException.h"
#include "exceptions/InvalidSchainException.h"
#include "exceptions/InvalidSourceIPException.h"
#include "messages/Message.h"
#include "protocols/blockconsensus/BlockConsensusAgent.h"

#include "messages/NetworkMessageEnvelope.h"
#include "threads/GlobalThreadRegistry.h"
#include "network/Sockets.h"
#include "network/ZMQServerSocket.h"
#include "Buffer.h"
#include "TransportNetwork.h"

TransportType TransportNetwork::transport = TransportType::ZMQ;


void TransportNetwork::addToDeferredMessageQueue(ptr<NetworkMessageEnvelope> _me) {
    CHECK_ARGUMENT(_me);

    auto msg = dynamic_pointer_cast<NetworkMessage>(_me->getMessage());

    getSchain()->getNode()->getIncomingMsgDB()->saveMsg(msg);

    auto _blockID = _me->getMessage()->getBlockID();

    ptr<vector<ptr<NetworkMessageEnvelope> > > messageList;

    {
        lock_guard<recursive_mutex> l(deferredMessageMutex);

        if (deferredMessageQueue.count(_blockID) == 0) {
            messageList = make_shared<vector<ptr<NetworkMessageEnvelope>>>();
            deferredMessageQueue[_blockID] = messageList;
        } else {
            messageList = deferredMessageQueue[_blockID];
        };

        messageList->push_back(_me);
    }
}

ptr<vector<ptr<NetworkMessageEnvelope> > > TransportNetwork::pullMessagesForCurrentBlockID() {





    LOCK(deferredMessageMutex);


    block_id currentBlockID = sChain->getLastCommittedBlockID() + 1;


    auto returnList = make_shared<vector<ptr<NetworkMessageEnvelope>>>();


    for (auto it = deferredMessageQueue.cbegin();
         it != deferredMessageQueue.cend() /* not hoisted */;
        /* no increment */ ) {
        if (it->first <= currentBlockID) {
            for (auto &&msg : *(it->second)) {
                returnList->push_back(msg);
            }

            it = deferredMessageQueue.erase(it);
        } else {
            ++it;
        }
    }

    return returnList;
}

void TransportNetwork::broadcastMessage(ptr<NetworkMessage> _m) {


    if (_m->getBlockID() <= this->catchupBlocks) {
        return;
    }

    try {

        getSchain()->getNode()->getOutgoingMsgDB()->saveMsg(_m);

        unordered_set<uint64_t> sent;

        while (3 * (sent.size() + 1) < getSchain()->getNodeCount() * 2) {
            for (auto const &it : *getSchain()->getNode()->getNodeInfosByIndex()) {
                auto index = (uint64_t) it.second->getSchainIndex();
                if (index != (getSchain()->getSchainIndex()) && !sent.count(index)) {
                    ASSERT(it.second->getSchainIndex() != getSchain()->getSchainIndex());
                    if (sendMessage(it.second, _m)) {
                        sent.insert((uint64_t) it.second->getSchainIndex());
                    }
                }
            }
        }

        if (sent.size() + 1 < getSchain()->getNodeCount()) {
            for (auto const &it : *getSchain()->getNode()->getNodeInfosByIndex()) {
                auto index = (uint64_t) it.second->getSchainIndex();
                if (index != (getSchain()->getSchainIndex()) && !sent.count(index)) {
                    {
                        lock_guard<recursive_mutex> lock(delayedSendsLock);
                        delayedSends.at(index - 1).push_back({_m, it.second});
                        if (delayedSends.at(index - 1).size() > 256) {
                            delayedSends.at(index - 1).pop_front();
                        }
                    }
                }
            }
        }


        for (auto const &it : *getSchain()->getNode()->getNodeInfosByIndex()) {
            if (it.second->getSchainIndex() != getSchain()->getSchainIndex()) {
                confirmMessage(it.second);
            }
        }

    } catch (...) {
        throw_with_nested(InvalidStateException(__FUNCTION__, __CLASS_NAME__));
    }

}

void TransportNetwork::networkReadLoop() {
    setThreadName("NtwkRdLoop", getSchain()->getNode()->getConsensusEngine());
    waitOnGlobalStartBarrier();

    try {
        while (!sChain->getNode()->isExitRequested()) {
            try {
                ptr<NetworkMessageEnvelope> m = receiveMessage();

                if (!m)
                    continue;  // check exit again

                if (m->getMessage()->getBlockID() <= catchupBlocks) {
                    continue;
                }

                ASSERT(sChain);


                postDeferOrDrop(m);
            } catch (ExitRequestedException &) {
                return;
            } catch (FatalError &) {
                throw;
            } catch (exception &e) {
                if (sChain->getNode()->isExitRequested()) {
                    sChain->getNode()->getSockets()->consensusZMQSocket->closeReceive();
                    return;
                }
                Exception::logNested(e);
            }

        }  // while
    } catch (FatalError &e) {
        sChain->getNode()->exitOnFatalError(e.getMessage());
    }

    sChain->getNode()->getSockets()->consensusZMQSocket->closeReceive();
}

void TransportNetwork::postDeferOrDrop(ptr<NetworkMessageEnvelope> &m) {

    block_id currentBlockID = sChain->getLastCommittedBlockID() + 1;


    auto bid = m->getMessage()->getBlockID();

    if (bid > currentBlockID) {
        addToDeferredMessageQueue(m);
        return;
    }



    if (bid < currentBlockID) {
        if (bid + MAX_ACTIVE_CONSENSUSES <= currentBlockID) {
            // too old, drop
            return;
        }
        else {
            sChain->postMessage(m);
            return;

        }
    }


    // now  deal with the case of bid = currentBlockID

    auto msg = dynamic_pointer_cast<NetworkMessage>(m->getMessage());

    CHECK_STATE(msg);


    // ask consensus whether to defer

    if (sChain->getBlockConsensusInstance()->shouldPost(msg)) {
        sChain->postMessage(m);
    } else {
        addToDeferredMessageQueue(m);

    }

}

void TransportNetwork::deferredMessagesLoop() {
    setThreadName("DeferMsgLoop", getSchain()->getNode()->getConsensusEngine());

    auto nodeCount = getSchain()->getNodeCount();
    auto schainIndex = getSchain()->getSchainIndex();

    waitOnGlobalStartBarrier();

    while (!getSchain()->getNode()->isExitRequested()) {
        ptr<vector<ptr<NetworkMessageEnvelope> > > deferredMessages;

        deferredMessages = pullMessagesForCurrentBlockID();

        for (auto message : *deferredMessages) {
            postDeferOrDrop(message);
        }

        for (int i = 0; i < nodeCount; i++) {
            if (i != (schainIndex - 1)) {
                lock_guard<recursive_mutex> lock(delayedSendsLock);
                if (delayedSends.at(i).size() > 0) {
                    if (sendMessage(
                            delayedSends.at(i).front().second, delayedSends.at(i).front().first)) {
                        delayedSends.at(i).pop_front();
                    }
                }
            }
        }
        usleep(100000);
    }
}


void TransportNetwork::startThreads() {
    networkReadThread =
            make_shared<thread>(std::bind(&TransportNetwork::networkReadLoop, this));
    deferredMessageThread =
            make_shared<thread>(std::bind(&TransportNetwork::deferredMessagesLoop, this));

    auto reg = getSchain()->getNode()->getConsensusEngine()->getThreadRegistry();

    reg->add(networkReadThread);
    reg->add(deferredMessageThread);
}

bool TransportNetwork::validateIpAddress(ptr<string> &ip) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip.get()->c_str(), &(sa.sin_addr));
    return result != 0;
}


void TransportNetwork::waitUntilExit() {
    networkReadThread->join();
    deferredMessageThread->join();
}

ptr<string> TransportNetwork::ipToString(uint32_t _ip) {
    char *ip = (char *) &_ip;
    return make_shared<string>(
            to_string((uint8_t) ip[0]) + "." + to_string((uint8_t) ip[1]) + "." +
            to_string((uint8_t) ip[2]) + "." + to_string((uint8_t) ip[3]));
}

ptr<NetworkMessageEnvelope> TransportNetwork::receiveMessage() {
    auto buf = make_shared<Buffer>(MAX_CONSENSUS_MESSAGE_LEN);
    uint64_t readBytes = readMessageFromNetwork(buf);

    auto msg = make_shared<string>((const char *) buf->getBuf()->data(), readBytes);

    auto mptr = NetworkMessage::parseMessage(msg, getSchain());

    ptr<NodeInfo> realSender = sChain->getNode()->getNodeInfoByIndex(mptr->getSrcSchainIndex());

    if (realSender == nullptr) {
        BOOST_THROW_EXCEPTION(InvalidStateException("NetworkMessage from unknown sender schain index",
                                                    __CLASS_NAME__));
    }

    ptr<ProtocolKey> key = mptr->createDestinationProtocolKey();

    if (key == nullptr) {
        BOOST_THROW_EXCEPTION(InvalidMessageFormatException(
                                      "Network Message with corrupt protocol key", __CLASS_NAME__ ));
    };

    return
            make_shared<NetworkMessageEnvelope>(mptr, realSender
            );
};


void TransportNetwork::setTransport(TransportType _transport) {
    TransportNetwork::transport = _transport;
}

TransportType TransportNetwork::getTransport() {
    return transport;
}

uint32_t TransportNetwork::getPacketLoss() const {
    return packetLoss;
}

void TransportNetwork::setPacketLoss(uint32_t _packetLoss) {
    TransportNetwork::packetLoss = _packetLoss;
}

void TransportNetwork::setCatchupBlocks(uint64_t _catchupBlocks) {
    TransportNetwork::catchupBlocks = _catchupBlocks;
}

uint64_t TransportNetwork::getCatchupBlock() const {
    return catchupBlocks;
}


TransportNetwork::TransportNetwork(Schain &_sChain)
        : Agent(_sChain, false), delayedSends((uint64_t) _sChain.getNodeCount()) {
    auto cfg = _sChain.getNode()->getCfg();

    if (cfg.find("catchupBlocks") != cfg.end()) {
        uint64_t catchupBlock = cfg.at("catchupBlocks").get<uint64_t>();
        setCatchupBlocks(catchupBlock);
    }

    if (cfg.find("packetLoss") != cfg.end()) {
        uint32_t pl = cfg.at("packetLoss").get<uint64_t>();
        ASSERT(pl <= 100);
        setPacketLoss(pl);
    }
}

void TransportNetwork::confirmMessage(const ptr<NodeInfo> &) {

};

TransportNetwork::~TransportNetwork() {
}
