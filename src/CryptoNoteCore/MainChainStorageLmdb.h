// Copyright (c) 2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "IMainChainStorage.h"
#include <Common/FileSystemShim.h>

#include "Currency.h"
#include "lmdb.h"
#include "lmdbpp.h"
#include <atomic>

namespace CryptoNote
{
    class MainChainStorageLmdb : public IMainChainStorage
    {
        public:
            MainChainStorageLmdb(const std::string &blocksFilename, const std::string &indexesFilename);

            virtual ~MainChainStorageLmdb();

            virtual void pushBlock(const RawBlock &rawBlock) override;
            virtual void popBlock() override;

            virtual RawBlock getBlockByIndex(const uint32_t index) override;
            virtual uint32_t getBlockCount() const override;

            virtual void clear() override;

        private:
            void initializeBlockCount();
            void checkResize();
            void renewRoTxn();
            void renewRwTxn(bool sync);

            lmdb::env m_db = lmdb::env::create();
            mutable MDB_txn *rtxn;
            mutable MDB_txn *wtxn;
            lmdb::dbi rodbi;
            lmdb::dbi rwdbi;
            mutable std::atomic_int m_blockcount;
            mutable std::atomic_int m_dirty;
            fs::path m_dbpath;
    };

    std::unique_ptr<IMainChainStorage> createSwappedMainChainStorageLmdb(const std::string &dataDir, const Currency &currency);
}