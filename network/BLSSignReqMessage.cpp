//
// Created by kladko on 15.12.20.
//

#include "BLSSignReqMessage.h"
#include "SGXWalletServer.hpp"


Json::Value BLSSignReqMessage::process() {
    auto keyName = getStringRapid("keyShareName");
    auto hash = getStringRapid("messageHash");
    auto t = getUint64Rapid("t");
    auto n = getUint64Rapid("n");
    auto result =  SGXWalletServer::blsSignMessageHashImpl(keyName, hash, t, n);
    result["type"] = ZMQMessage::BLS_SIGN_RSP;
    return result;
}