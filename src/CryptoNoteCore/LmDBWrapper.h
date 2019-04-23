// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <atomic>
#include <memory>
#include <string>


#include "lmdb.h"
#include "lmdbpp.h"

#include "IDataBase.h"
#include "DataBaseConfig.h"

#include <Logging/LoggerRef.h>
#include <Common/FileSystemShim.h>

namespace CryptoNote {

class LmDBWrapper : public IDataBase {
public:
  LmDBWrapper(std::shared_ptr<Logging::ILogger> logger);
  virtual ~LmDBWrapper();

  LmDBWrapper(const LmDBWrapper&) = delete;
  LmDBWrapper(LmDBWrapper&&) = delete;

  LmDBWrapper& operator=(const LmDBWrapper&) = delete;
  LmDBWrapper& operator=(LmDBWrapper&&) = delete;

  void init(const DataBaseConfig& config);
  void shutdown();

  void destroy(const DataBaseConfig& config); //Be careful with this method!

  std::error_code write(IWriteBatch& batch) override;
  std::error_code read(IReadBatch& batch) override;

private:
  void checkResize();
  bool needResize();
  //std::error_code write(IWriteBatch& batch, bool sync);

  //rocksdb::Options getDBOptions(const DataBaseConfig& config);
  //std::string getDataDir(const DataBaseConfig& config);
  void setDataDir(const DataBaseConfig& config);
  fs::path getDataDir(const DataBaseConfig& config);

  enum State {
    NOT_INITIALIZED,
    INITIALIZED
  };

  Logging::LoggerRef logger;
  std::atomic<State> state;
  fs::path m_dbDir;
  fs::path m_dbFile;
  lmdb::env m_db  = lmdb::env::create();
  std::atomic_uint m_dirty;
};
}
