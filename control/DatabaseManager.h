#pragma once

#include <array>
#include <string>

namespace leveldb {
class DB;
}

namespace bamboo {
class DatabaseManager {
public:
  DatabaseManager();

  ~DatabaseManager();

  void selectDatabase(int dbIndex);

  int getCurrentDatabaseIndex() const { return current_db_index_; }

  std::string get(const std::string &key);

  std::string set(const std::string &key, const std::string &value);

  std::string del(const std::string &key);

  std::string listAllKVs();

private:
  bool directoryExists(const std::string &path);

  void createDirectory(const std::string &path);

  std::array<leveldb::DB *, 10> dbs_;
  int current_db_index_ = 0;
};

} // namespace bamboo