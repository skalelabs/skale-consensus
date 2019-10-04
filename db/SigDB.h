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

    @file SigDB.h
    @author Stan Kladko
    @date 2019
*/


class ConsensusBLSSignature;

#ifndef SKALED_SIGDB_H
#define SKALED_SIGDB_H


#include "LevelDB.h"

class ThresholdSignature;

class SigDB : public LevelDB{

    node_id nodeId;

    const string getFormatVersion();

public:

    SigDB(string& filename, node_id nodeId );

    ptr<string>  createKey(block_id _blockId);

    void addSignature(block_id _blockId, ptr<ThresholdSignature> _sig);

    uint64_t  getCounter();

};



#endif //SKALED_BLOCKDB_H
