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

    @file BLSSignature.cpp
    @author Stan Kladko
    @date 2019
*/


#include "../SkaleCommon.h"
#include "../thirdparty/json.hpp"
#include "../Log.h"
#include "../network/Utils.h"
#include "../crypto/bls_include.h"

#include "BLSSignature.h"
#include "ConsensusBLSSignature.h"


ConsensusBLSSignature::ConsensusBLSSignature(ptr<string> _s, block_id _blockID) : BLSSignature(_s),
        blockId(_blockID) {


}

block_id ConsensusBLSSignature::getBlockId() const {
    return blockId;
}


ConsensusBLSSignature::ConsensusBLSSignature(ptr<libff::alt_bn128_G1> &_s, block_id _blockID) :
    BLSSignature(_s), blockId(_blockID)  {};
