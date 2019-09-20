//
// Created by kladko on 19.09.19.
//

#ifndef SKALED_COMMITTEDBLOCKFRAGMENTLIST_H
#define SKALED_COMMITTEDBLOCKFRAGMENTLIST_H

class CommittedBlockFragment;

class CommittedBlockFragmentList {

    bool isSerialized = false;

    recursive_mutex listMutex;

    const block_id blockID;

    const uint64_t  totalFragments;

    map<fragment_index, ptr<vector<uint8_t>>> fragments;

    void checkSanity();

public:
    CommittedBlockFragmentList(const block_id &_blockId, const uint64_t _totalFragments);

    bool addFragment(ptr<CommittedBlockFragment> _fragment);

    bool isComplete();

    ptr<vector<uint8_t >> serialize();

};


#endif //SKALED_COMMITTEDBLOCKFRAGMENTLIST_H