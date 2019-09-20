//
// Created by kladko on 19.09.19.
//

#include "../SkaleCommon.h"
#include "../Log.h"
#include "../exceptions/ParsingException.h"

#include "CommittedBlockFragment.h"

CommittedBlockFragment::CommittedBlockFragment(const block_id &_blockId, const uint64_t _totalFragments,
                                               const fragment_index &_fragmentIndex, const ptr<vector<uint8_t>>& _data) :
                                               blockId(_blockId), totalFragments(_totalFragments),
                                               fragmentIndex(_fragmentIndex), data(_data) {
    CHECK_ARGUMENT(_totalFragments > 0);
    CHECK_ARGUMENT(_fragmentIndex <= _totalFragments);
    CHECK_ARGUMENT(data != nullptr);
    CHECK_ARGUMENT(_blockId > 0);
    CHECK_ARGUMENT(_data->size() > 0);
    if (_data->size() < 3) {
        BOOST_THROW_EXCEPTION(ParsingException("Data fragment too short:" +
         to_string(_data->size()), __CLASS_NAME__));
    }

    if(_data->front() != '<') {
        BOOST_THROW_EXCEPTION(ParsingException("Data fragment does not start with <", __CLASS_NAME__));
    }

    if(_data->back() != '>') {
        BOOST_THROW_EXCEPTION(ParsingException("Data fragment does not end with >", __CLASS_NAME__));
    }
}


block_id CommittedBlockFragment::getBlockId() const {
    return blockId;
}

uint64_t CommittedBlockFragment::getTotalFragments() const {
    return totalFragments;
}

fragment_index CommittedBlockFragment::getIndex() const {
    return fragmentIndex;
}

ptr<vector<uint8_t>> CommittedBlockFragment::getData() const {
    return data;
}