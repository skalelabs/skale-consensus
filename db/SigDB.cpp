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

    @file SigDB.cpp
    @author Stan Kladko
    @date 2019
*/


#include "../SkaleCommon.h"
#include "../Log.h"

#include "../crypto/ConsensusBLSSignature.h"
#include "../crypto/ConsensusBLSSigShare.h"
#include "../datastructures/CommittedBlock.h"

#include "SigDB.h"




SigDB::SigDB(string& filename, node_id _nodeId ) : LevelDB( filename, _nodeId ) {}


const string SigDB::getFormatVersion() {
    return "1.0";
}


ptr<string>  SigDB::createKey(const block_id _blockId) {
    return make_shared<string>(getFormatVersion() + ":" + to_string( nodeId ) + ":"
                                + to_string( _blockId ));
}

void SigDB::addSignature(block_id _blockId, ptr<ThresholdSignature> _sig) {
    auto key = createKey( _blockId );
    if (readString( *key ) == nullptr )
        writeString( *key, *_sig->toString() );
}


