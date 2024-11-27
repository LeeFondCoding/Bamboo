#pragma once

#include "control/DatabaseManager.h"

namespace bamboo {
class ClientSession {
public:
  ClientSession(DatabaseManager *dbManager)
      : db_manager_(dbManager), current_db_index_(0) {}

  int getCurrentDbIndex() const { return current_db_index_; }
  void setCurrentDbIndex(int index);

  std::string getErrorMessage() const { return error_message_; }

  std::string processCommand(const std::string &cmd, const std::string &args);

private:
  DatabaseManager *db_manager_;
  int current_db_index_;
  std::string error_message_;

  std::string showHelp();
};
} // namespace bamboo