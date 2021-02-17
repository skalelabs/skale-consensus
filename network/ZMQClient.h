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

    @file ZMQClient.h
    @author Stan Kladko
    @date 2021
*/




#ifndef SGXWALLET_ZMQCLIENT_H
#define SGXWALLET_ZMQCLIENT_H

#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#include <zmq.hpp>
#include "thirdparty/zguide/zhelpers.hpp"
#include <jsonrpccpp/client.h>
#include "ZMQMessage.h"


#define REQUEST_TIMEOUT     10000    //  msecs, (> 1000!)

class ZMQClient {

private:

    zmq::context_t ctx;

    EVP_PKEY* pkey;
    EVP_PKEY* pubkey;
    X509* x509Cert;

    bool sign = true;
    string certFileName;
    string certificate;
    string key;

    recursive_mutex mutex;

    string url;

    map<uint64_t , shared_ptr <zmq::socket_t>> clientSockets;

    shared_ptr <ZMQMessage> doRequestReply(Json::Value &_req);

    string doZmqRequestReply(string &_req);

    uint64_t getProcessID();

    static string readFileIntoString(const string& _fileName);

public:


    ZMQClient(const string &ip, uint16_t port, bool _sign, const string&  _certPathName,
              const string& _certKeyName);

    void reconnect() ;

    static pair<EVP_PKEY*, X509*>  readPublicKeyFromCertStr(const string& _cert);

    static string signString(EVP_PKEY* _pkey, const string& _str);

    string blsSignMessageHash(const std::string &keyShareName, const std::string &messageHash, int t, int n);

    string ecdsaSignMessageHash(int base, const std::string &keyName, const std::string &messageHash);

};



#endif //SGXWALLET_ZMQCLIENT_H
