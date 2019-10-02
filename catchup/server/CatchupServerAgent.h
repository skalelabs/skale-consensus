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

    @file CatchupServerAgent.h
    @author Stan Kladko
    @date 2018
*/

#pragma once





#include <mutex>
#include <queue>
#include <condition_variable>
#include "../../abstracttcpserver/ConnectionStatus.h"
#include "../../abstracttcpserver/AbstractServerAgent.h"

#include "../../headers/Header.h"
#include "../../network/ServerConnection.h"
#include "../../datastructures/PartialHashesList.h"
#include "../../headers/Header.h"

#include "CatchupWorkerThreadPool.h"
#include "../../Agent.h"

class CommittedBlock;
class CommittedBlockList;
class CatchupResponseHeader;
class BlockFinalizeResponseHeader;

class CatchupServerAgent : public AbstractServerAgent {

   ptr<CatchupWorkerThreadPool> catchupWorkerThreadPool;


    ptr<vector<uint8_t>>createBlockCatchupResponse( nlohmann::json _jsonRequest,
                                                         ptr<CatchupResponseHeader> _responseHeader, block_id _blockID);


    ptr<vector<uint8_t>>createBlockFinalizeResponse( nlohmann::json _jsonRequest,
                                                    ptr<BlockFinalizeResponseHeader> _responseHeader, block_id _blockID);


public:
    CatchupServerAgent(Schain &_schain, ptr<TCPServerSocket> _s);
    ~CatchupServerAgent() override;

    CatchupWorkerThreadPool *getCatchupWorkerThreadPool() const;

    ptr<vector<uint8_t>> createResponseHeaderAndBinary(ptr<ServerConnection> _connectionEnvelope,
                                                       nlohmann::json _jsonRequest, ptr<Header>& _responseHeader);

    void processNextAvailableConnection(ptr<ServerConnection> _connection) override;

    ptr<vector<uint8_t>> getSerializedBlock(uint64_t i) const;
};
