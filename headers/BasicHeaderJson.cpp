/*
    Copyright (C) 2018-2019 SKALE Labs

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

    @file BasicHeader.cpp
    @author Stan Kladko
    @date 2018
*/

#include "SkaleCommon.h"
#include "Log.h"
#include "exceptions/FatalError.h"
#include "exceptions/InvalidArgumentException.h"
#include "exceptions/NetworkProtocolException.h"
#include "network/Buffer.h"
#include "BasicHeaderJson.h"

bool BasicHeaderJson::isComplete() const {
    return complete;
}

string BasicHeaderJson::serializeToString() {
    ASSERT(complete)
    nlohmann::json j;

    CHECK_STATE(type != nullptr)

    j["type"] = type;

    addFields(j);

    string s(j.dump());

    CHECK_STATE(s.size() > 16)

    return s;
}

int64_t BasicHeaderJson::getTotalObjects() {
    return totalObjects;
}

ptr< Buffer > BasicHeaderJson::toBuffer() {

    auto s = serializeToString();

    uint64_t len  = s.size();

    auto buf = make_shared<Buffer>(len + sizeof(len));
    buf->write(&len, sizeof(len));
    buf->write((void *) s.data(), len);
    CHECK_STATE(buf->getCounter() >= 10);
    return buf;
}



void BasicHeaderJson::nullCheck(nlohmann::json &js, const char * _name ) {
    CHECK_ARGUMENT( _name );
    if (js.find( _name ) == js.end()) {
        BOOST_THROW_EXCEPTION(NetworkProtocolException("Null " + string( _name ) + " in json", __CLASS_NAME__));
    }
};

uint64_t BasicHeaderJson::getUint64(nlohmann::json &_js, const char *_name) {
    CHECK_ARGUMENT(_name);
    nullCheck(_js, _name);
    uint64_t result = _js[_name];
    return result;
};




int32_t BasicHeaderJson::getInt32(nlohmann::json &_js, const char *_name) {
    CHECK_ARGUMENT(_name);
    nullCheck(_js, _name);
    int32_t result = _js[_name];
    return result;
};



uint32_t BasicHeaderJson::getUint32(nlohmann::json &_js, const char *_name) {
    CHECK_ARGUMENT(_name);
    nullCheck(_js, _name);
    uint32_t result = _js[_name];
    return result;
};

string BasicHeaderJson::getString(nlohmann::json &_js, const char *_name) {
    CHECK_ARGUMENT(_name);
    nullCheck(_js, _name);
    string result = _js[_name];
    return result;
}

BasicHeaderJson::BasicHeaderJson(const char *_type) : type(_type)  {
    CHECK_ARGUMENT(_type);
    totalObjects++;
}


BasicHeaderJson::~BasicHeaderJson() {
    if (totalObjects > 0)
        totalObjects--;
}


atomic<int64_t>  BasicHeaderJson::totalObjects(1);