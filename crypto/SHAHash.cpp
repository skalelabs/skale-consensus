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

    @file SHAHash.cpp
    @author Stan Kladko
    @date 2019
*/

#include "SkaleCommon.h"
#include "Log.h"
#include "thirdparty/json.hpp"

#include "headers/Header.h"
#include "network/Utils.h"
#include "exceptions/InvalidArgumentException.h"

#include "SHAHash.h"

void SHAHash::print() {
    CHECK_STATE(hash);
    for (size_t i = 0; i < SHA_HASH_LEN; i++) {
        cerr << to_string(hash->at(i));
    }
}


uint8_t SHAHash::at(uint32_t _position) {
    CHECK_STATE(hash);
    return hash->at(_position);
}


ptr<SHAHash> SHAHash::fromHex(ptr<string> _hex) {
    CHECK_ARGUMENT(_hex);
    auto result = make_shared<array<uint8_t, SHA_HASH_LEN>>();
    Utils::cArrayFromHex(*_hex, result->data(), SHA_HASH_LEN);
    return make_shared<SHAHash>(result);
}

ptr<string> SHAHash::toHex() {
    CHECK_STATE(hash);
    auto result = Utils::carray2Hex(hash->data(), SHA_HASH_LEN);
    CHECK_STATE(result);
    return result;
}


int SHAHash::compare(ptr<SHAHash> _hash2 ) {
    CHECK_ARGUMENT( _hash2 );
    CHECK_STATE(hash);

    for (size_t i = 0; i < SHA_HASH_LEN; i++) {
        if (hash->at(i) < _hash2->at(i))
            return -1;
        if (hash->at(i) > _hash2->at(i))
            return 1;
    }
    return 0;
}

SHAHash::SHAHash(ptr<array<uint8_t, SHA_HASH_LEN>> _hash) {
    CHECK_ARGUMENT(_hash);
    hash = _hash;
}

ptr<SHAHash> SHAHash::calculateHash(ptr<vector<uint8_t>> _data) {
    CHECK_ARGUMENT(_data);
    auto digest = make_shared<array<uint8_t, SHA_HASH_LEN> >();

    CryptoPP::SHA256 hashObject;

    hashObject.Update(_data->data(), _data->size());
    hashObject.Final(digest->data());

    auto hash = make_shared<SHAHash>(digest);
    return hash;
}

ptr<SHAHash> SHAHash::merkleTreeMerge(ptr<SHAHash> _left, ptr<SHAHash> _right) {
    CHECK_ARGUMENT(_left);
    CHECK_ARGUMENT(_right);

    auto concatenation = make_shared<vector<uint8_t>>();
    concatenation->reserve(2 * SHA_HASH_LEN);

    auto leftHash = _left->getHash();
    CHECK_STATE(leftHash);

    concatenation->insert(concatenation->end(), leftHash->begin(), leftHash->end());

    auto rightHash = _right->getHash();
    CHECK_STATE(rightHash);

    concatenation->insert(concatenation->end(), rightHash->begin(), rightHash->end());

    return calculateHash(concatenation);
}

ptr<array<uint8_t, SHA_HASH_LEN>> SHAHash::getHash() const {
    CHECK_STATE(hash);
    return hash;
}


