// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (     0, uint256("0xae1717d30616a6d18e0c87d7a6a3ff17bc8b85147aef8bf60bf0f7177a5bc097"))
        (    10, uint256("0x5136b7b1c97bb8c8e35d8b2d9d69fea0f1b9dc35e99656ba9e28dec0700978f9"))
        (    50, uint256("0xa068d4581832117882944efadf259f59484062af24a3a48f2c696b06c0348bf6"))
        (   100, uint256("0x114da40b5d198f8959ccead79155855223ac16a7e6bc51ba750dd6fba1ba6e77"))
        (   500, uint256("0x9f852fa08f4a52987f67b4f4a3512ca80b68da800fb350f1aadebac817612860"))
        (  1000, uint256("0x752fbca9c94f5d51b173edbaec4ba9f4b2a03f26c37e69200046caa342c98176"))
        (  2000, uint256("0x6b7bc4cd374de46b197d261425a8d8344e5f0fb1474716c4e4cfad05f68f8a2f"))
        (  3000, uint256("0xa5e06eec307d8f9a4b95e7fb5f5ed48c954b351277f46e86892f0c5f099438c8"))
        (  4000, uint256("0xdbfa0b06b1df7f77d28328b02052519882b78ad204a6d3997e281f10cdebdc15"))
        (  5000, uint256("0xe3ab9fc546bd6e694cba091d089c0797565c26b25463a6653b2a3cbfae4b698c"))
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        (     0, uint256("0xae1717d30616a6d18e0c87d7a6a3ff17bc8b85147aef8bf60bf0f7177a5bc097"))
        (    10, uint256("0x5136b7b1c97bb8c8e35d8b2d9d69fea0f1b9dc35e99656ba9e28dec0700978f9"))
        (    50, uint256("0xa068d4581832117882944efadf259f59484062af24a3a48f2c696b06c0348bf6"))
        (   100, uint256("0x114da40b5d198f8959ccead79155855223ac16a7e6bc51ba750dd6fba1ba6e77"))
        (   500, uint256("0x9f852fa08f4a52987f67b4f4a3512ca80b68da800fb350f1aadebac817612860"))
        (  1000, uint256("0x752fbca9c94f5d51b173edbaec4ba9f4b2a03f26c37e69200046caa342c98176"))
        (  2000, uint256("0x6b7bc4cd374de46b197d261425a8d8344e5f0fb1474716c4e4cfad05f68f8a2f"))
        (  3000, uint256("0xa5e06eec307d8f9a4b95e7fb5f5ed48c954b351277f46e86892f0c5f099438c8"))
        (  4000, uint256("0xdbfa0b06b1df7f77d28328b02052519882b78ad204a6d3997e281f10cdebdc15"))
        (  5000, uint256("0xe3ab9fc546bd6e694cba091d089c0797565c26b25463a6653b2a3cbfae4b698c"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        0,
        0,
        0
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
