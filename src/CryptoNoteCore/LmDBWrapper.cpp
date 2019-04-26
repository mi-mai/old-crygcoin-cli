// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "LmDBWrapper.h"

#include "DataBaseErrors.h"
#include <Common/FileSystemShim.h>

#include <iostream>
#include <fstream>
#include <string_view>

using namespace CryptoNote;
using namespace Logging;

namespace
{
    const std::string DB_NAME = "DB";
    const std::string TESTNET_DB_NAME = "testnet_DB";
    const uint64_t MAX_DIRTY = 200000;
} // namespace

LmDBWrapper::LmDBWrapper(std::shared_ptr<Logging::ILogger> logger) : logger(logger, "LmDBWrapper"), state(NOT_INITIALIZED)
{
}

LmDBWrapper::~LmDBWrapper()
{
}

void LmDBWrapper::init(const DataBaseConfig &config)
{
    if (state.load() != NOT_INITIALIZED)
    {
        throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::ALREADY_INITIALIZED));
    }

    logger(INFO) << "Initializing DB using lmdb backend";

    /* set m_dbDir & m_dbFile fore easy reuse */
    setDataDir(config);

    /* create db dir if  not already exists (lmbd not automatically create it) */
    if (!fs::exists(m_dbDir) || !fs::is_directory(m_dbDir))
    {
        if (!fs::create_directory(m_dbDir))
        {
            logger(ERROR) << "Failed to create db directory";
            throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::INTERNAL_ERROR));
        }
    }

    if (!std::ifstream(m_dbFile))
    {
        std::ofstream file(m_dbFile);
        if (!file)
        {
            logger(ERROR) << "Failed to create db file";
            throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::INTERNAL_ERROR));
        }
        file.close();
    }

    /* set initial mapsize */
    uint64_t mapsize = 0;
    try
    {
        mapsize = fs::file_size(m_dbFile);
    }
    catch (...)
    {
    }
    if (mapsize == 0)
    {
        // starts with 512M
        mapsize += 512ULL * 1024 * 1024;
    }

    logger(DEBUGGING) << "Initial DB mapsize: " << mapsize << " bytes";
    m_db.set_mapsize(mapsize);
    
    logger(INFO) << "Opening DB in " << m_dbDir;
    try
    {
        m_db.open(m_dbDir.c_str(), MDB_NOSYNC | MDB_WRITEMAP | MDB_MAPASYNC | MDB_NORDAHEAD, 0664);
    }
    catch (const std::exception &e)
    {
        logger(ERROR) << "Failed to open database: " << e.what();
        throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::INTERNAL_ERROR));
    }

    /* read only tx handle, we can reuse it to save some mallocs */
    lmdb::txn_begin(m_db, nullptr, MDB_RDONLY, &rotxn);
    rodbi = lmdb::dbi::open(rotxn, nullptr);

    /* resize mapsize if needed.
       on init, do this before initializing write tx handle */
    checkResize(true);

    /* write tx handle */
    lmdb::txn_begin(m_db, nullptr, 0, &rwtxn);
    rwdbi = lmdb::dbi::open(rwtxn, nullptr);

    /* initialize tx dirty counter */
    m_dirty = 0;

    // mark as initialized
    state.store(INITIALIZED);
}

void LmDBWrapper::shutdown()
{
    if (state.load() != INITIALIZED)
    {
        throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::NOT_INITIALIZED));
    }

    logger(INFO) << "Finalizing DB Write.";
    m_db.sync();
    logger(INFO) << "Closing DB.";
    m_db.close();
    state.store(NOT_INITIALIZED);
}

/* this one is untested */
void LmDBWrapper::destroy(const DataBaseConfig &config)
{
    if (state.load() != NOT_INITIALIZED)
    {
        throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::ALREADY_INITIALIZED));
    }

    logger(WARNING) << "Destroying DB in " << m_dbDir;
    {
        try
        {
            rwdbi.drop(rwtxn, 0);
        }
        catch (const std::exception &e)
        {
            logger(ERROR) << "DB Error. DB can't be destroyed in " << m_dbDir << ". Error: " << e.what();
            throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::INTERNAL_ERROR));
        }
    }
    /* renew write tx handle, force fsync */
    renewRwTxHandle(true);
}

std::error_code LmDBWrapper::write(IWriteBatch &batch)
{
    if (state.load() != INITIALIZED)
    {
        throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::NOT_INITIALIZED));
    }

    /* error code to be returned, if any */
    std::error_code errCode;
    /* things to be inserted */
    const std::vector<std::pair<std::string, std::string>> rawData(batch.extractRawDataToInsert());
    /* things to be removed */
    const std::vector<std::string> rawKeys(batch.extractRawKeysToRemove());

    /* resize if needed */
    checkResize(false);

    /* renew rw tx handle */
    renewRwTxHandle(false);

    /* insert */
    uint32_t num_inserted = 0;
    for (const std::pair<std::string, std::string> &kvPair : rawData)
    {
        if (rwdbi.put(rwtxn, kvPair.first, kvPair.second))
        {
            /* force commit if batch size > 200, avoid txn_full error */
            if (num_inserted >= 300)
            {
                logger(TRACE) << "DB FORCING COMMIT";
                renewRwTxHandle(false);
                num_inserted = 0;
            }

            num_inserted++;
            m_dirty++;
        }
        else
        {
            logger(ERROR) << "dbi.put failed";
            errCode = make_error_code(CryptoNote::error::DataBaseErrorCodes::INTERNAL_ERROR);
        }
    }

    /* delete */
    for (const std::string &key : rawKeys)
    {
        if (rwdbi.del(rwtxn, key))
        {
            m_dirty++;
        }
        else
        {
            logger(ERROR) << "dbi.del failed";
            errCode = make_error_code(CryptoNote::error::DataBaseErrorCodes::INTERNAL_ERROR);
        }
    }

    /* renew write tx, without sync */
    renewRwTxHandle(false);

    /* flush/fsync when dirty commits reached MAX_DIRTY */
    if (m_dirty >= MAX_DIRTY)
    {
        m_dirty = 0;
        logger(TRACE) << "Flushing dirty commits to disk";
        m_db.sync(0);
    }

    return errCode;
}

std::error_code LmDBWrapper::read(IReadBatch &batch)
{
    if (state.load() != INITIALIZED)
    {
        throw std::runtime_error("Not initialized.");
    }

    const std::vector<std::string> rawKeys(batch.getRawKeys());
    std::vector<bool> resultStates;
    std::vector<std::string> values;

    // renew ro tx handle
    renewRoTxHandle();

    {
        for (const std::string &key : rawKeys)
        {
            std::string_view val;
            if (rodbi.get(rotxn, key, val))
            {
                values.push_back(std::string(val));
                resultStates.push_back(true);
            }
            else
            {
                // @todo: get rid of this
                // rocksdb MultiGet compat: pass empty string if the key wasn't found
                values.push_back(std::string(""));
                resultStates.push_back(false);
            }
        }
    }

    batch.submitRawResult(values, resultStates);
    return std::error_code();
}

void LmDBWrapper::setDataDir(const DataBaseConfig &config)
{
    if (config.getTestnet())
    {
        m_dbDir = fs::path(config.getDataDir() + '/' + TESTNET_DB_NAME);
    }
    else
    {
        m_dbDir = fs::path(config.getDataDir() + '/' + DB_NAME);
    }
    m_dbFile = fs::path(m_dbDir / "data.mdb");
}

fs::path LmDBWrapper::getDataDir(const DataBaseConfig &config)
{
    return m_dbDir;
}

void LmDBWrapper::renewRoTxHandle()
{
    //logger(TRACE) << "DB_RENEW_RO...";
    try
    {
        lmdb::txn_reset(rotxn);
    }
    catch (const std::exception &e)
    {
        logger(TRACE) << "DB_RO_TXN_RESET_FAILED: " << e.what();
    }

    try
    {
        lmdb::txn_renew(rotxn);
    }
    catch (const std::exception &e)
    {
        logger(TRACE) << "DB_RO_TXN_RENEW_FAILED: " << e.what();
    }
}

void LmDBWrapper::renewRwTxHandle(bool sync)
{
    try
    {
        lmdb::txn_commit(rwtxn);
    }
    catch (const std::exception &e)
    {
        logger(ERROR) << "AH DB_RW_TXN_COMMIT_FAILED: " << e.what();
    }

    if (sync)
    {
        m_db.sync();
    }

    try
    {
        lmdb::txn_begin(m_db, nullptr, 0, &rwtxn);
    }
    catch (const std::exception &e)
    {
        logger(ERROR) << "DB_RW_TXN_BEGIN_FAILED: " << e.what();
    }

    //rwdbi = lmdb::dbi::open(rwtxn, nullptr);
}

void LmDBWrapper::checkResize(const bool init)
{
    //logger(TRACE) << "DB_CHECK_RESIZE...";
    const size_t min_avail = 512ULL * 1024 * 1024;
    size_t size_avail;
    size_t mapsize;

    // renew tx handle
    //renewRoTxHandle();

    {
        MDB_stat stat = rodbi.stat(rotxn);

        MDB_envinfo info;
        lmdb::env_info(m_db, &info);

        mapsize = info.me_mapsize;
        const size_t size_used = stat.ms_psize * info.me_last_pgno;
        size_avail = mapsize - size_used;
    }

    if (size_avail >= min_avail)
    {
        logger(TRACE) << "DB Resize: no resize required, size avail: " << size_avail << " bytes.";
        return;
    }

    /* for resizing, have to close all active rw tx, so, commit first */
    if (!init)
    {
        try
        {
            lmdb::txn_commit(rwtxn);
        }
        catch (const std::exception &e)
        {
            logger(ERROR) << "ttt DB_RW_TXN_COMMIT_FAILED: " << e.what();
        }
    }

    m_db.sync();

    const size_t extra = 1ULL << 30;
    mapsize += extra;

    logger(DEBUGGING) << "Resizing database. New mapsize: " << mapsize << " bytes.";
    try
    {
        m_db.set_mapsize(mapsize);
    }
    catch (const std::exception &e)
    {
        logger(ERROR) << "DB_RESIZE_FAILED_FAILED: " << e.what();
    }
}