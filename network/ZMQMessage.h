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

    @file ZMQMessage.h
    @author Stan Kladko
    @date 2021
*/

#pragma once


#include <memory>
#include <vector>

#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rand.h>




#include "sgxwallet/abstractstubserver.h"
#include "thirdparty/lrucache.hpp"
#include "thirdparty/rapidjson/document.h"
#include "thirdparty/rapidjson/stringbuffer.h"
#include "thirdparty/rapidjson/writer.h"

#include "SkaleCommon.h"
#include "Log.h"

#include "exceptions/NetworkProtocolException.h"


#define ZMQ_SERVER_ERROR -89
#define ZMQ_COULD_NOT_PARSE -90
#define ZMQ_INVALID_MESSAGE -91
#define ZMQ_COULD_NOT_GET_SOCKOPT -92
#define ZMQ_INVALID_MESSAGE_SIZE -93
#define ZMQ_NO_TYPE_IN_MESSAGE -94
#define ZMQ_NO_SIG_IN_MESSAGE -95
#define ZMQ_NO_CERT_IN_MESSAGE -96
#define ZMQ_COULD_NOT_VERIFY_SIG -97

using namespace std;

class ZMQMessage {

    shared_ptr<rapidjson::Document> d;

    static cache::lru_cache<string, pair<EVP_PKEY*, X509*>> verifiedCerts;

protected:

public:

    static constexpr const char *BLS_SIGN_REQ = "BLSSignReq";
    static constexpr const char *BLS_SIGN_RSP = "BLSSignRsp";
    static constexpr const char *ECDSA_SIGN_REQ = "ECDSASignReq";
    static constexpr const char *ECDSA_SIGN_RSP = "ECDSASignRsp";

    explicit ZMQMessage(shared_ptr<rapidjson::Document> &_d) : d(_d) {
    };

    string getStringRapid(const char *_name);

    uint64_t getUint64Rapid(const char *_name);

    uint64_t getStatus() {
        return getUint64Rapid("status");
    }

    static shared_ptr <ZMQMessage> parse(const char* _msg, size_t _size);

    static shared_ptr<ZMQMessage> buildResponse(string& type, shared_ptr<rapidjson::Document> _d);

    virtual Json::Value process() = 0;

};