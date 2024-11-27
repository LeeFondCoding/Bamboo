#include "control/ClientSession.h"

#include <stdexcept>

namespace bamboo {

void ClientSession::setCurrentDbIndex(int index) {
  try {
    db_manager_->selectDatabase(index);
    current_db_index_ = index;
  } catch (const std::invalid_argument &e) {
    error_message_ = e.what();
  }
}

std::string ClientSession::processCommand(const std::string &cmd,
                                          const std::string &args) {
  if (cmd == "SELECT") {
    int dbIndex = std::stoi(args);
    setCurrentDbIndex(dbIndex);
    if (!error_message_.empty()) {
      return error_message_ + "\r\n";
    }
    return "OK\r\n";
  } else if (cmd == "GET") {
    return db_manager_->get(args) + "\r\n";
  } else if (cmd == "SET") {
    size_t endPos = args.find(' ', 0);
    if (endPos != std::string::npos) {
      std::string key = args.substr(0, endPos);
      std::string value = args.substr(endPos + 1);
      return db_manager_->set(key, value) + "\r\n";
    }
  } else if (cmd == "DEL") {
    return db_manager_->del(args) + "\r\n";
  } else if (cmd == "LIST") {
    return db_manager_->listAllKVs() + "\r\n";
  } else if (cmd == "CURRENTDB") {
    return "Current Database Index: " + std::to_string(getCurrentDbIndex()) +
           "\r\n";
  } else if (cmd == "HELP") {
    return showHelp();
  } else {
    return "UNKNOWN COMMAND\r\n";
  }
  return "";
}

std::string ClientSession::showHelp() {
  return "Available commands:\r\n"
         "SELECT <index> - Select the database instance by index (0-9)\r\n"
         "GET <key>      - Get the value associated with the key in the "
         "current database\r\n"
         "SET <key> <value> - Set the value for the key in the current "
         "database\r\n"
         "LIST           - List all key-value pairs in the current database\r\n"
         "CURRENTDB      - Show the current selected database index\r\n"
         "HELP           - Show this help message\r\n";
}
} // namespace bamboo