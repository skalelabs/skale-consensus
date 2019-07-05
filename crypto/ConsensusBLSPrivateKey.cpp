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

    @file ConsensusBLSPrivateKey.cpp
    @author Stan Kladko
    @date 2019
*/


#include "../Log.h"
#include "../SkaleCommon.h"
#include "../exceptions/FatalError.h"
#include "../exceptions/InvalidArgumentException.h"
#include "../network/Utils.h"
#include "../thirdparty/json.hpp"
#include "SHAHash.h"

#include "../crypto/bls_include.h"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>


#include "BLSPrivateKey.h"
#include "BLSSigShare.h"
#include "ConsensusBLSPrivateKey.h"
#include "ConsensusBLSSigShare.h"


ConsensusBLSPrivateKey::ConsensusBLSPrivateKey( const string& k, node_count _nodeCount ) :
BLSPrivateKey(k, size_t(_nodeCount), size_t(_nodeCount))
{

    if (totalSigners == 1 || totalSigners == 2)
        requiredSigners = totalSigners;
    else {
        requiredSigners = 2 * totalSigners / 3 + 1;
    }

}


ptr<ConsensusBLSSigShare>
ConsensusBLSPrivateKey::sign(ptr<string> _msg, schain_id _schainId, block_id _blockId, schain_index _signerIndex,
                    node_id _signerNodeId) {
    ptr<signatures::Bls> obj;

    obj = make_shared<signatures::Bls>( signatures::Bls( requiredSigners, totalSigners ) );

    libff::alt_bn128_G1 hash = obj->Hashing( *_msg );

    auto ss = make_shared<libff::alt_bn128_G1>( obj->Signing( hash, *sk ) );

    ss->to_affine_coordinates();

    auto s = make_shared<ConsensusBLSSigShare>( ss, _schainId,  _blockId, _signerIndex, _signerNodeId );

    auto ts = s->getBlsSigShare()->toString();

    auto sig2 = make_shared<ConsensusBLSSigShare>( ts, _schainId, _blockId, _signerIndex, _signerNodeId );

    ASSERT( *s->getBlsSigShare()->getSigShare() == *sig2->getBlsSigShare()->getSigShare() );


    return s;
}




