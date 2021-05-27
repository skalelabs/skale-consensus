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

    @file CryptoManager.h
    @author Stan Kladko
    @date 2019
*/

#ifndef SKALED_CRYPTOMANAGER_H
#define SKALED_CRYPTOMANAGER_H


#include "messages/NetworkMessage.h"
#include "openssl/ec.h"

#include "sgxclient/SgxZmqClient.h"

#define USER_SPACE 1

#include "thirdparty/lru_ordered_cache.hpp"
#include "thirdparty/lrucache.hpp"

class Schain;
class BLAKE3Hash;
class ConsensusBLSSigShare;
class ThresholdSigShareSet;
class ThresholdSigShare;
class BlockProposal;
class ThresholdSignature;
class StubClient;
class ECP;
class BLSPublicKey;

namespace CryptoPP {
class ECP;
template < class EC, class H >
struct ECDSA;
}  // namespace CryptoPP

class ECDSAVerify;

namespace jsonrpc {
class HttpClient;
}

class MPZNumber {
public:
    MPZNumber();
    ~MPZNumber();
    mpz_t number;
};


class OpenSSLECDSAKey;
class OpenSSLEdDSAKey;

class CryptoManager {

    static list<uint64_t> ecdsaSignTimes;
    static recursive_mutex ecdsaSignMutex;
    static atomic<uint64_t> ecdsaSignTotal;

    static list<uint64_t> blsSignTimes;
    static recursive_mutex blsSignMutex;
    static atomic<uint64_t> blsSignTotal;

    static atomic<uint64_t> blsCounter;
    static atomic<uint64_t> ecdsaCounter;


    cache::lru_cache< uint64_t, tuple< ptr< OpenSSLEdDSAKey >, string, string > >
        sessionKeys;                                               // tsafe
    cache::lru_ordered_cache< string, string > sessionPublicKeys;  // tsafe
    recursive_mutex sessionKeysLock;
    recursive_mutex publicSessionKeysLock;

    map< uint64_t, ptr< jsonrpc::HttpClient > > httpClients;  // tsafe
    map< uint64_t, ptr< StubClient > > sgxClients;            // tsafe


    ptr< SgxZmqClient > zmqClient = nullptr;

    recursive_mutex clientsLock;

    map< uint64_t, string > ecdsaPublicKeyMap;  // tsafe
    recursive_mutex ecdsaPublicKeyMapLock;

    map< uint64_t, ptr< vector< string > > > blsPublicKeyMap;  // tsafe

    ptr< vector< ptr< vector< string > > > > sgxBLSPublicKeys;  // tsafe

    ptr< vector< string > > sgxECDSAPublicKeys;  // tsafe

    uint64_t totalSigners;
    uint64_t requiredSigners;

    bool isSGXEnabled = false;
    bool isHTTPSEnabled = true;
    bool isSSLCertEnabled = false;

    string sgxSSLKeyFileFullPath;
    string sgxSSLCertFileFullPath;
    string sgxECDSAKeyName;
    string sgxBlsKeyName;


    ptr< BLSPublicKey > sgxBLSPublicKey;
    ptr< BLSPublicKey > blsPublicKeyObj = nullptr;

    Schain* sChain = nullptr;

    static bool retryHappened;

    static string sgxURL;

    string sgxDomainName;
    uint16_t sgxPort = 0;

    uint64_t doesServerSupportZMQ = 0;



private:

    ptr< StubClient > getSgxClient();

    tuple< ptr< OpenSSLEdDSAKey >, string > localGenerateFastKey();

    string sign( const ptr< BLAKE3Hash >& _hash );

    tuple< string, string, string > sessionSign(
        const ptr< BLAKE3Hash >& _hash, block_id _blockId );

    bool verifyECDSASig( const ptr< BLAKE3Hash >& _hash, const string& _sig, node_id _nodeId );

    ptr< ThresholdSigShare > signSigShare(
        const ptr< BLAKE3Hash >& _hash, block_id _blockId, bool _forceMockup );
    ptr< ThresholdSigShare > signDAProofSigShare(
        const ptr< BLAKE3Hash >& _hash, block_id _blockId, bool _forceMockup );

    void initSGXClient();

    static pair<string, uint64_t> parseSGXDomainAndPort( const string& _url );



public:

    static bool isRetryHappened();

    static void setRetryHappened( bool retryHappened );

    bool sessionVerifyEdDSASig(
        const ptr< BLAKE3Hash >& _hash, const string& _sig, const string& _publicKey );
    // This constructor is used for testing
    CryptoManager( uint64_t _totalSigners, uint64_t _requiredSigners, bool _isSGXEnabled,
        string _sgxURL = "", string _sgxSslKeyFileFullPath = "",
        string _sgxSslCertFileFullPath = "", string _sgxEcdsaKeyName = "",
        ptr< vector< string > > _sgxEcdsaPublicKeys = nullptr );

    explicit CryptoManager( Schain& sChain );

    Schain* getSchain() const;


    void verifyDAProofSigShare( ptr< ThresholdSigShare > _sigShare, schain_index _schainIndex,
        ptr< BLAKE3Hash > _hash, node_id _nodeId, bool _forceMockup );

    ptr< ThresholdSignature > verifyDAProofThresholdSig(
        const ptr< BLAKE3Hash >& _hash, const string& _signature, block_id _blockId );

    ptr< ThresholdSigShareSet > createSigShareSet( block_id _blockId );
    ptr< ThresholdSigShareSet > createDAProofSigShareSet( block_id _blockId );

    ptr< ThresholdSigShare > createSigShare( const string& _sigShare, schain_id _schainID,
        block_id _blockID, schain_index _signerIndex, bool _forceMockup );

    ptr< ThresholdSigShare > createDAProofSigShare( const string& _sigShare, schain_id _schainID,
        block_id _blockID, schain_index _signerIndex, bool _forceMockup );

    void signProposal( BlockProposal* _proposal );

    bool verifyProposalECDSA(
        const ptr< BlockProposal >& _proposal, const string& _hashStr, const string& _signature );

    tuple< ptr< ThresholdSigShare >, string, string, string > signDAProof(
        const ptr< BlockProposal >& _p );

    ptr< ThresholdSigShare > signBinaryConsensusSigShare(
        const ptr< BLAKE3Hash >& _hash, block_id _blockId, uint64_t _round );

    ptr< ThresholdSigShare > signBlockSigShare( const ptr< BLAKE3Hash >& _hash, block_id _blockId );

    tuple< string, string, string > signNetworkMsg( NetworkMessage& _msg );

    bool verifyNetworkMsg( NetworkMessage& _msg );

    static ptr< void > decodeSGXPublicKey( const string& _keyHex );

    static pair< string, string > generateSGXECDSAKey( const ptr< StubClient >& _c );

    static string getSGXEcdsaPublicKey( const string& _keyName, const ptr< StubClient >& _c );

    static void generateSSLClientCertAndKey( string& _fullPathToDir );
    static void setSGXKeyAndCert( string& _keyFullPath, string& _certFullPath, uint64_t _sgxPort );


    string sgxSignECDSA( const ptr< BLAKE3Hash >& _hash, string& _keyName );

    tuple< string, string, string > sessionSignECDSA(
        const ptr< BLAKE3Hash >& _hash, block_id _blockID );

    bool verifyECDSA(
        const ptr< BLAKE3Hash >& _hash, const string& _sig, const string& _publicKey );


    ptr< BLSPublicKey > getSgxBlsPublicKey();

    string getSgxBlsKeyName();

    static const string& getSgxUrl();
    static void setSgxUrl( const string& sgxUrl );


    static ptr< BLAKE3Hash > calculatePublicKeyHash( string publicKey, block_id _blockID );

    bool sessionVerifySigAndKey( ptr< BLAKE3Hash >& _hash, const string& _sig,
        const string& _publicKey, const string& pkSig, block_id _blockID, node_id _nodeId );

    void exitZMQClient();



    static void addECDSASignStats(uint64_t _time);
    static void addBLSSignStats(uint64_t _time);

    static  uint64_t getEcdsaStats() {
        return ecdsaSignTotal / LEVELDB_STATS_HISTORY;
    }
    static  uint64_t getBLSStats() {
        return blsSignTotal / LEVELDB_STATS_HISTORY;
    }

    static uint64_t getECDSAs() {
        return ecdsaCounter;
    }

    static uint64_t getBLSs() {
        return blsCounter;
    }

    static uint64_t getECDSATotals() {
        return ecdsaCounter;
    }

    static uint64_t getBLSTotals() {
        return blsCounter;
    }

    void checkZMQStatusIfUnknownECDSA(const string &_keyName);

    void checkZMQStatusIfUnknownBLS();
};

#define RETRY_BEGIN \
    CryptoManager::setRetryHappened(false); \
    while ( true ) {                      \
        try {
#define RETRY_END                                                                                  \
    ;                                                                                              \
    if ( CryptoManager::isRetryHappened() ) {                                                          \
        LOG( info, "Successfully reconnected to SGX server:" + CryptoManager::getSgxUrl() );            \
        CryptoManager::setRetryHappened(false);                                                      \
    }                                                                                              \
    break;                                                                                         \
    }                                                                                              \
    catch ( const std::exception& e ) {                                                            \
        if ( e.what() && ( string( e.what() ).find( "Could not connect" ) != string::npos ||       \
                             string( e.what() ).find( "libcurl error: 56" ) != string::npos ||     \
                             string( e.what() ).find( "libcurl error: 35" ) != string::npos ||     \
                             string( e.what() ).find( "libcurl error: 52" ) != string::npos ||     \
                             string( e.what() ).find( "timed out" ) != string::npos ) ) {          \
            if ( string( e.what() ).find( "libcurl error: 52" ) != string::npos ) {                \
                LOG( err,                                                                          \
                    "Got libcurl error 52. You may be trying to connect with http to https "       \
                    "server" );                                                                    \
            };                                                                                     \
            if (!CryptoManager::isRetryHappened())                                                                                       \
                LOG( err, "Could not connect to sgx server: " + CryptoManager::getSgxUrl() +                \
                          ", retrying each five seconds ... \n" + string( e.what() ) );            \
            CryptoManager::setRetryHappened(true);                                                   \
            sleep( 5 );                                                                            \
        } else {                                                                                   \
            LOG( err, "Could not connect to sgx server: " + CryptoManager::getSgxUrl() );               \
            LOG( err, e.what() );                                                                  \
            throw;                                                                                 \
        }                                                                                          \
    }                                                                                              \
    catch ( ... ) {                                                                                \
        LOG( err, "FATAL Unknown error while connecting to sgx server:" + CryptoManager::getSgxUrl() ); \
        throw;                                                                                     \
    }                                                                                              \
    }

#endif  // SKALED_CRYPTOMANAGER_H
