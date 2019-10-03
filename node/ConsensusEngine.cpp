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

    @file ConsensusEngine.cpp
    @author Stan Kladko
    @date 2018
*/




#include "../SkaleCommon.h"
#include "../Log.h"
#include "../exceptions/FatalError.h"
#include "../Agent.h"
#include "../thirdparty/json.hpp"
#include "../threads/GlobalThreadRegistry.h"

#include "zmq.h"


#pragma GCC diagnostic push
// Suppress warnings: "unknown option after ‘#pragma GCC diagnostic’ kind [-Wpragmas]".
// This is necessary because not all the compilers have the same warning options.
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wdeprecated-register"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wmismatched-tags"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wtautological-compare"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#pragma GCC diagnostic ignored "-Wunused-private-field"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#pragma GCC diagnostic ignored "-Wchar-subscripts"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wreorder"

#include "bls.h"

#pragma GCC diagnostic pop

#include <boost/multiprecision/cpp_int.hpp>

#include "../chains/Schain.h"
#include "../libBLS/bls/BLSSignature.h"
#include "../libBLS/bls/BLSPublicKey.h"
#include "../libBLS/bls/BLSPrivateKeyShare.h"
#include "../exceptions/EngineInitException.h"
#include "../json/JSONFactory.h"
#include "../network/Sockets.h"
#include "../network/Utils.h"
#include "../network/ZMQServerSocket.h"
#include "../protocols/InstanceGarbageCollectorAgent.h"
#include "../protocols/ProtocolKey.h"
#include "../protocols/binconsensus/BinConsensusInstance.h"

#include "../exceptions/FatalError.h"

#include "ConsensusEngine.h"

using namespace boost::filesystem;


void ConsensusEngine::parseFullConfigAndCreateNode(const string &configFileContents) {
    nlohmann::json j = nlohmann::json::parse(configFileContents);

    std::set<node_id> dummy;

    Node *node = JSONFactory::createNodeFromJsonObject(j["skaleConfig"]["nodeInfo"], dummy, this);

    JSONFactory::createAndAddSChainFromJsonObject(
            *node, j["skaleConfig"]["sChain"], this);

    nodes[node->getNodeID()] = node;
}

Node *ConsensusEngine::readNodeConfigFileAndCreateNode(
        const fs_path &path, set<node_id> &nodeIDs) {
    try {
        fs_path nodeFileNamePath(path);

        nodeFileNamePath /= string(SkaleCommon::NODE_FILE_NAME);

        checkExistsAndFile(nodeFileNamePath.string());

        fs_path schainDirNamePath(path);

        schainDirNamePath /= string(SkaleCommon::SCHAIN_DIR_NAME);

        checkExistsAndDirectory(schainDirNamePath.string());

        Node *node = JSONFactory::createNodeFromJson(nodeFileNamePath.string(), nodeIDs, this);


        if (node == nullptr) {
            return nullptr;
        }

        readSchainConfigFiles(*node, schainDirNamePath.string());

        ASSERT(nodes.count(node->getNodeID()) == 0);

        nodes[node->getNodeID()] = node;
        return node;

    } catch (Exception &e) {
        Exception::logNested(e);
        throw;
    }
}


void ConsensusEngine::readSchainConfigFiles(Node &_node, const fs_path &_dirPath) {

    try {

        checkExistsAndDirectory(_dirPath);

        directory_iterator itr(_dirPath);

        auto end = directory_iterator();


        // cycle through the directory

        for (; itr != end; ++itr) {
            fs_path jsonFile = itr->path();
            LOG(debug, "Parsing file:" + jsonFile.string());
            JSONFactory::createAndAddSChainFromJson(_node, jsonFile, this);
            break;
        }

    } catch (...) {
        throw_with_nested(FatalError(__FUNCTION__, __CLASS_NAME__));
    }

}


void ConsensusEngine::checkExistsAndDirectory(const fs_path &_dirPath) {
    if (!exists(_dirPath)) {
        BOOST_THROW_EXCEPTION(FatalError("Not found: " + _dirPath.string()));
    }

    if (!is_directory(_dirPath)) {
        BOOST_THROW_EXCEPTION(FatalError("Not a directory: " + _dirPath.string()));

    }
}


void ConsensusEngine::checkExistsAndFile(const fs_path &_filePath) {
    if (!exists(_filePath)) {
        BOOST_THROW_EXCEPTION(FatalError("Not found: " + _filePath.string()));
    }

    if (is_directory(_filePath)) {
        BOOST_THROW_EXCEPTION(FatalError("Path is a direcotry, regular file is required " + _filePath.string()));
    }
}


void ConsensusEngine::parseConfigsAndCreateAllNodes(const fs_path &dirname) {
    try {

        checkExistsAndDirectory(dirname);


        // cycle through the directory

        uint64_t nodeCount = 0;

        directory_iterator itr(dirname);

        auto end = directory_iterator();


        for (; itr != end; itr++) {
            if (!is_directory(itr->path())) {
                BOOST_THROW_EXCEPTION(FatalError("Junk file found. Remove it: " + itr->path().string()));
            }

            nodeCount++;
        };


        directory_iterator itr2(dirname);

        for (; itr2 != end; itr2++) {
            if (!is_directory(itr2->path())) {
                BOOST_THROW_EXCEPTION(FatalError("Junk file found. Remove it: " + itr2->path().string()));
            }

            readNodeConfigFileAndCreateNode(itr2->path(), Node::nodeIDs);
        };

        if (nodes.size() == 0) {
            BOOST_THROW_EXCEPTION(FatalError("No valid node dirs found"));
        }

        ASSERT(nodeCount == nodes.size());

        LOG(trace, "Parsed configs and created " + to_string(ConsensusEngine::nodesCount()) +
                   " nodes");
    } catch (exception &e) {
        Exception::logNested(e);
        throw;
    }
}

void ConsensusEngine::startAll() {

    try {

        for (auto const it : nodes) {
            it.second->startServers();
            LOG(info, "Started servers" + to_string(it.second->getNodeID()));
        }


        for (auto const it : nodes) {
            it.second->startClients();
            LOG(info, "Started clients" + to_string(it.second->getNodeID()));
        }

        LOG(info, "Started all nodes");
    }

    catch (Exception &e) {

        Exception::logNested(e);

        for (auto const it : nodes) {
            if (!it.second->isExitRequested()) {
                it.second->exitOnFatalError(e.getMessage());
            }
        }

        spdlog::shutdown();

        throw_with_nested(EngineInitException("Start all failed", __CLASS_NAME__));
    }
}

void ConsensusEngine::slowStartBootStrapTest() {


    for (auto const it : nodes) {
        it.second->startServers();
    }

    for (auto const it : nodes) {
        it.second->startClients();
        it.second->getSchain()->bootstrap(lastCommittedBlockID, lastCommittedBlockTimeStamp);
    }

    LOG(info, "Started all nodes");
}

void ConsensusEngine::bootStrapAll() {
    try {


        for (auto const it : nodes) {
            LOG(trace, "Bootstrapping node");
            it.second->getSchain()->bootstrap(lastCommittedBlockID, lastCommittedBlockTimeStamp);
            LOG(trace, "Bootstrapped node");
        }
    } catch (Exception &e) {

        for (auto const it : nodes) {
            if (!it.second->isExitRequested()) {
                it.second->exitOnFatalError(e.getMessage());
            }
        }

        spdlog::shutdown();

        Exception::logNested(e);

        throw_with_nested(EngineInitException("Consensus engine bootstrap failed", __CLASS_NAME__));
    }
}


node_count ConsensusEngine::nodesCount() {
    return node_count(nodes.size());
}


ConsensusEngine::ConsensusEngine() : exitRequested(false) {
    try {
        signal(SIGPIPE, SIG_IGN);
        libff::init_alt_bn128_params();
        Log::init();
        init();
    } catch (Exception &e) {
        Exception::logNested(e);
        throw_with_nested(EngineInitException("Engine construction failed", __CLASS_NAME__));
    }
}

std::string ConsensusEngine::exec(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        BOOST_THROW_EXCEPTION(std::runtime_error("popen() failed!"));
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void ConsensusEngine::systemHealthCheck() {
    string ulimit;
    try {
        ulimit = exec("/bin/bash -c \"ulimit -n\"");
    } catch (...) {
        const char *errStr = "Execution of /bin/bash -c ulimit -n failed";
        cerr << errStr;
        throw_with_nested(EngineInitException(errStr, __CLASS_NAME__));
    }
    int noFiles = std::strtol(ulimit.c_str(), NULL, 10);

    auto noUlimitCheck = std::getenv("NO_ULIMIT_CHECK") != nullptr;
    auto onTravis = std::getenv("TRAVIS_BUILD_TYPE") != nullptr;

    if (noFiles < 65535 && !noUlimitCheck && !onTravis) {

        const char *error = "File descriptor limit (ulimit -n) is less than 65535. Set it to 65535 or more as described"
                            "in https://bugs.launchpad.net/ubuntu/+source/lightdm/+bug/1627769\n";
        cerr << error;
        BOOST_THROW_EXCEPTION(EngineInitException(error, __CLASS_NAME__));
    }

    Utils::checkTime();
}

void ConsensusEngine::init() {

    sigset_t sigpipe_mask;
    sigemptyset(&sigpipe_mask);
    sigaddset(&sigpipe_mask, SIGPIPE);
    sigset_t saved_mask;
    if (pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask) == -1) {
        BOOST_THROW_EXCEPTION(FatalError("Could not block SIGPIPE"));
    }

    systemHealthCheck();
    BinConsensusInstance::initHistory();
}


ConsensusEngine::ConsensusEngine(ConsensusExtFace &_extFace, uint64_t _lastCommittedBlockID,
                                 uint64_t _lastCommittedBlockTimeStamp, const string &_blsPrivateKey,
                                 const string &_blsPublicKey1, const string &_blsPublicKey2,
                                 const string &_blsPublicKey3,
                                 const string &_blsPublicKey4) : exitRequested(false),
                                                                 blsPublicKey1(_blsPublicKey1),
                                                                 blsPublicKey2(_blsPublicKey2),
                                                                 blsPublicKey3(_blsPublicKey3),
                                                                 blsPublicKey4(_blsPublicKey4),
                                                                 blsPrivateKey(_blsPrivateKey) {


    Log::init();

    try {

        ASSERT(_lastCommittedBlockTimeStamp < (uint64_t) 2 * MODERN_TIME);


        extFace = &_extFace;
        lastCommittedBlockID = block_id(_lastCommittedBlockID);

        ASSERT2((_lastCommittedBlockTimeStamp >= MODERN_TIME || _lastCommittedBlockID == 0),
                "Invalid last committed block time stamp ");


        lastCommittedBlockTimeStamp = _lastCommittedBlockTimeStamp;

        init();

    } catch (Exception &e) {
        Exception::logNested(e);
        throw_with_nested(EngineInitException("Engine construction failed", __CLASS_NAME__));
    }
};


ConsensusExtFace *ConsensusEngine::getExtFace() const {
    return extFace;
}


void ConsensusEngine::exitGracefully() {

    auto previouslyCalled = exitRequested.exchange(true);

    if (previouslyCalled) {
        return;
    }


    for (auto const it : nodes) {
        it.second->exit();
    }


    GlobalThreadRegistry::joinAll();


    for (auto const it : nodes) {
        if (it.second->getSockets())
            it.second->getSockets()->getConsensusZMQSocket()->terminate();
    }


}


ConsensusEngine::~ConsensusEngine() {
    for (auto &n : nodes) {
        delete n.second;
    }
}

const string &ConsensusEngine::getBlsPublicKey1() const {
    return blsPublicKey1;
}

const string &ConsensusEngine::getBlsPublicKey2() const {
    return blsPublicKey2;
}

const string &ConsensusEngine::getBlsPublicKey3() const {
    return blsPublicKey3;
}

const string &ConsensusEngine::getBlsPublicKey4() const {
    return blsPublicKey4;
}

const string &ConsensusEngine::getBlsPrivateKey() const {
    return blsPrivateKey;
}

block_id ConsensusEngine::getLargestCommittedBlockID() {

    block_id id = 0;

    for (auto &&item: nodes) {
        auto id2 = item.second->getSchain()->getLastCommittedBlockID();

        if (id2 > id) {
            id = id2;
        }

    }

    return id;
}

u256 ConsensusEngine::getPriceForBlockId(uint64_t _blockId) const {

    ASSERT(nodes.size() == 1);

    for (auto &&item: nodes) {
        return item.second->getSchain()->getPriceForBlockId(_blockId);
    }

    return 0; // never happens
}

