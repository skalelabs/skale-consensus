/*
    Copyright (C) 2019 SKALE Labs

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

    @file DAProofDB.cpp
    @author Stan Kladko
    @date 2019
*/

#include "../SkaleCommon.h"
#include "../Agent.h"
#include "../Log.h"
#include "../exceptions/FatalError.h"
#include "../thirdparty/json.hpp"

#include "../crypto/ConsensusBLSSigShare.h"
#include "../crypto/ConsensusBLSSignature.h"

#include "../abstracttcpserver/ConnectionStatus.h"
#include "../chains/Schain.h"
#include "../node/Node.h"
#include "../pendingqueue/PendingTransactionsAgent.h"
#include "../crypto/ConsensusBLSSigShare.h"
#include "../crypto/ConsensusSigShareSet.h"
#include "../crypto/CryptoManager.h"
#include "../crypto/SHAHash.h"
#include "../datastructures/BlockProposal.h"


#include "leveldb/db.h"

#include "SigDB.h"
#include "DAProofDB.h"
#include "BLSSigShare.h"
#include "BLSSignature.h"
#include "BLSSigShareSet.h"
#include "../crypto/ThresholdSigShare.h"
#include "../datastructures/DAProof.h"


using namespace std;


DAProofDB::DAProofDB(string &_dirName, string &_prefix, node_id _nodeId, uint64_t _maxDBSize, Schain &_sChain) :
CacheLevelDB(_dirName, _prefix, _nodeId, _maxDBSize, _sChain.getTotalSigners(), _sChain.getRequiredSigners()){
    this->sChain = &_sChain;
};

const string DAProofDB::getFormatVersion() {
    return "1.0";
}


// return not-null if _sigShare completes sig, null otherwise (both if not enough and too much)
ptr<DAProof> DAProofDB::addAndMergeSigShareAndVerifySig(ptr<ThresholdSigShare> _sigShare,
                                                           ptr<BlockProposal> _proposal) {

    ASSERT(_sigShare);

    LOCK(sigShareMutex)

    LOG(trace, "Adding sigshare");

    auto result = this->writeStringToSet(*_sigShare->toString(),
            _sigShare->getBlockId(), _sigShare->getSignerIndex());

    if (result != nullptr) {

        auto set = sChain->getCryptoManager()->createSigShareSet(_sigShare->getBlockId(),
                                                                 sChain->getTotalSigners(),
                                                                 sChain->getRequiredSigners());

        for (auto && entry : *result) {
            auto share = sChain->getCryptoManager()->createSigShare(
                    entry.second, sChain->getSchainID(),
                    _proposal->getBlockID(), entry.first,
                    totalSigners, requiredSigners);

            set->addSigShare(share);
        }


        LOG(trace, "Merged signature");
        auto sig = set->mergeSignature();
        sChain->getCryptoManager()->verifyThresholdSig(
                _proposal->getHash(), sig->toString(), _sigShare->getBlockId());
        auto proof = make_shared<DAProof>(_proposal, sig);
        return proof;
    }

    return nullptr;
}



