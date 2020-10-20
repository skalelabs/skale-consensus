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

    @file JSONFactory.h
    @author Stan Kladko
    @date 2018
*/

#pragma once

#include <json/value.h>
class Node;

class NodeInfo;

class ConsensusExtFace;

class ConsensusEngine;

class JSONFactory {
public:
    static ptr< Node > createNodeFromJsonFile(const ptr<string>& _sgxUrl, const fs_path& jsonFile, set< node_id >& nodeIDs,
        ConsensusEngine* _consensusEngine, bool _useSGX, const ptr<string>& _sgxSSLKeyFileFullPath,
        const ptr<string>& _sgxSSLCertFileFullPath, const ptr<string>& _ecdsaKeyName,
                                              const ptr< vector<string> >& _ecdsaPublicKeys, const ptr<string>& _blsKeyName,
                                              const ptr< vector< ptr< vector<string>>>>& _blsPublicKeys,
                                              const ptr< BLSPublicKey >& _blsPublicKey );

    static ptr< Node > createNodeFromJsonObject(
        const nlohmann::json& j, set< node_id >& nodeIDs, ConsensusEngine* _engine, bool _useSGX,
        const ptr<string>& _sgxURL,
        const ptr<string>& _sgxSSLKeyFileFullPath, const ptr<string>& _sgxSSLCertFileFullPath,
        const ptr<string>& _ecdsaKeyName, const ptr< vector<string> >& _ecdsaPublicKeys,
        const ptr<string>& _blsKeyName, const ptr< vector< ptr< vector<string>>>>& _blsPublicKeys,
        const ptr< BLSPublicKey >& _blsPublicKey

    );

    static void createAndAddSChainFromJson(
        const ptr< Node >& _node, const fs_path& _jsonFile, ConsensusEngine* _engine );

    static void createAndAddSChainFromJsonObject(
        const ptr< Node >& _node, const nlohmann::json& j, ConsensusEngine* _engine );

    static void parseJsonFile( nlohmann::json& j, const fs_path& configFile );

    static tuple< ptr< vector<string> >, ptr< vector<string> >, ptr< vector<string> >,
        ptr< vector< ptr< vector<string>>>>, ptr<BLSPublicKey>>
    parseTestKeyNamesFromJson(const ptr<string>& _sgxServer4Url,  const fs_path& configFile, uint64_t _totalNodes,
        uint64_t _requiredNodes);

    static pair< ptr< vector<string> >, ptr< vector<string> > > parseAllTestKeyNames(
        ptr<string>& _sgxServerUrl,
        const fs_path& _dir );

    static ptr<vector<string>> splitString(const string& str, const string& _delim = ":");

    static int64_t getInt64( Json::Value& _json, const char* key );

    static string getString( Json::Value& _json, const char* key );
};
