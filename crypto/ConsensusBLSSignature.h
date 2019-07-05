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

    @file BLSSignature.h
    @author Stan Kladko
    @date 2019
*/

#ifndef SKALED_CONSENSUSBLSSIGNATURE_H
#define SKALED_CONSENSUSBLSSIGNATURE_H

//constexpr uint64_t  MAX_BLS_SIGNATURE_SIZE = 64;


#include "BLSSignature.h"
class ConsensusBLSSignature : public BLSSignature {
    block_id blockId;

public:
    ConsensusBLSSignature(ptr<string> _s, block_id _blockID);

    ConsensusBLSSignature(ptr<libff::alt_bn128_G1>& _s, block_id _blockID);

   block_id getBlockId() const;
};


#endif //SKALED_CONSENSUSBLSSIGNATURE_H

