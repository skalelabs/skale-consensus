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

    @file BlockProposalFragment.h
    @author Stan Kladko
    @date 2019
*/
#ifndef SKALED_BLOCKPROPOSALFRAGMENT_H
#define SKALED_BLOCKPROPOSALFRAGMENT_H



class BlockProposalFragment {

    const block_id blockId;
    const uint64_t blockSize;
    ptr<string> blockHash;


    const uint64_t totalFragments;
    const fragment_index fragmentIndex;

    const ptr<vector<uint8_t>> data;


public:

    BlockProposalFragment(const block_id &blockId, const uint64_t totalFragments, const fragment_index &fragmentIndex,
                          const ptr<vector<uint8_t>> &data, uint64_t _blockSize, ptr<string> _blockHash);

    block_id getBlockId() const;

    uint64_t getTotalFragments() const;

    fragment_index getIndex() const;

    ptr<vector<uint8_t>> serialize() const;

    uint64_t getBlockSize() const;

    ptr<string> getBlockHash() const;

};


#endif //SKALED_BLOCKPROPOSALFRAGMENT_H
