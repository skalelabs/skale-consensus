/*
  Copyright (C) 2018-2019 SKALE Labs

  This file is part of libBLS.

  libBLS is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  libBLS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with libBLS.  If not, see <https://www.gnu.org/licenses/>.

  @file ECDSASignReqMessage.cpp
  @author Stan Kladko
  @date 2020
*/


#include "SGXWalletServer.hpp"

#include "ECDSASignReqMessage.h"



Json::Value ECDSASignReqMessage::process() {
    auto base = getUint64Rapid("base");
    auto keyName = getStringRapid("keyName");
    auto hash = getStringRapid("messageHash");
    auto result =  SGXWalletServer::ecdsaSignMessageHashImpl(base, keyName, hash);
    result["type"] = ZMQMessage::ECDSA_SIGN_RSP;
    return result;
}