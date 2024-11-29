#include "controller/DatabaseManager.h"

#include <leveldb/db.h>
#include <sstream>

#include <dirent.h>
#include <sys/stat.h>

namespace bamboo {

DatabaseManager::DatabaseManager() {
  // Ensure the dbinstance directory exists
  if (!directoryExists("dbinstance")) {
    createDirectory("dbinstance");
  }

  for (int i = 0; i < 10; ++i) {
    leveldb::Options options;
    options.create_if_missing = true;
    std::ostringstream oss;
    oss << "dbinstance/testdb" << i;
    leveldb::Status status = leveldb::DB::Open(options, oss.str(), &dbs_[i]);
    assert(status.ok());
  }
}

DatabaseManager::~DatabaseManager() {
  for (auto &db : dbs_) {
    delete db;
  }
}

void DatabaseManager::selectDatabase(int dbIndex) {
  if (dbIndex >= 0 && dbIndex < 10) {
    current_db_index_ = dbIndex;
  } else {
    throw std::invalid_argument("Invalid database index");
  }
}

std::string DatabaseManager::get(const std::string &key) {
  std::string value;
  leveldb::Status s =
      dbs_[current_db_index_]->Get(leveldb::ReadOptions(), key, &value);
  if (s.ok()) {
    return value;
  } else {
    return "NOT FOUND";
  }
}

std::string DatabaseManager::set(const std::string &key,
                                 const std::string &value) {
  leveldb::Status s =
      dbs_[current_db_index_]->Put(leveldb::WriteOptions(), key, value);
  if (s.ok()) {
    return "OK";
  } else {
    return "ERROR";
  }
}

std::string DatabaseManager::del(const std::string &key) {
  leveldb::Status s = dbs_[current_db_index_]->Delete(leveldb::WriteOptions(),
                                                      key);
  if (s.ok()) {
    return "OK";
  } else {
    return "ERROR";
  }
}

std::string DatabaseManager::listAllKVs() {
  leveldb::Iterator *it =
      dbs_[current_db_index_]->NewIterator(leveldb::ReadOptions());
  std::string response;
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    response += it->key().ToString() + ": " + it->value().ToString() + "\r\n";
  }
  delete it;
  if (!response.empty()) {
    return response;
  } else {
    return "EMPTY DATABASE";
  }
}

bool DatabaseManager::directoryExists(const std::string &path) {
  struct stat st;
  if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
    return true;
  }
  return false;
}

void DatabaseManager::createDirectory(const std::string &path) {
  mkdir(path.c_str(), 0755);
}

} // namespace bamboo