#pragma once

#include <cstring>

#include <fstream>
#include <memory>
#include <mutex>
#include <string>

namespace bamboo {
namespace db {

const char *dump_file = "store/dumpfile";
std::string delimiter = ":";
std::mutex mtx;

template <typename K, typename V> class Node {
public:
  Node() = default;

  Node(K k, V v, int level);

  ~Node();

  K getKey() const;

  V getValue() const;

  void setValue(V v);

  // get pointer of next node
  Node<K, V> *next() const;

  // a pointer to array which contains next node pointers
  Node<K, V> **forward_;

  int level_;

private:
  K key_;

  V value_;
};

template <typename K, typename V> class SkipList {
public:
  explicit SkipList(int max_level);

  ~SkipList();

  int getRandomLevel();

  Node<K, V> *createNode(K key, V value, int level);

  bool insertElement(K key, V value);

  void displayList();

  V searchElement(K key);

  void deleteElement(K key);

  void dumpFile();

  void loadFile();

  int size();

  Node<K, V> *front() const;

  class Iterator {
  public:
    explicit Iterator(const SkipList *list) : list_(list) {
      node_ = list->front();
    }

    Iterator() = default;

    bool operator==(const Iterator &rhs) const noexcept {
      return node_ == rhs.node_;
    }

    bool operator!=(const Iterator &rhs) const noexcept {
      return node_ != rhs.node_;
    }

    Iterator &operator++() noexcept {
      node_ = node_->next();
      return *this;
    }

    Node<K, V> *operator*() const { return *node_; }

    Node<K, V> *operator->() const { return node_; }

    Node<K, V> *get() { return node_; }

    void seekFirst() { node_ = list_->front(); }

  private:
    const SkipList<K, V> *list_;

    Node<K, V> *node_;
  };

private:
  void getKeyValueFromString(const std::string &str, std::string &key,
                             std::string &value);

  bool isValidString(const std::string &str);

  int max_level_;
  int list_level_{0};
  // skiplist totol element count
  int element_count_{0};

  Node<K, V> *header_;

  std::ofstream file_writer_;
  std::ifstream file_reader_;
};

template <typename K, typename V>
Node<K, V>::Node(K k, V v, int level) : key_(k), value_(v), level_(level) {
  forward_ = new Node<K, V> *[level + 1];
  memset(forward_, 0, sizeof(Node<K, V> *) * (level_ + 1));
}

template <typename K, typename V> Node<K, V>::~Node() { delete[] forward_; }

template <typename K, typename V> K Node<K, V>::getKey() const { return key_; }

template <typename K, typename V> V Node<K, V>::getValue() const {
  return value_;
}

template <typename K, typename V> void Node<K, V>::setValue(V v) { value_ = v; }

template <typename K, typename V> Node<K, V> *Node<K, V>::next() const {
  return forward_[0];
}

template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level) : max_level_(max_level) {
  K k;
  V v;
  header_ = new Node<K, V>(k, v, max_level_);
  srand(time(nullptr));
}

template <typename K, typename V> SkipList<K, V>::~SkipList() {
  if (file_reader_.is_open()) {
    file_reader_.close();
  }
  if (file_writer_.is_open()) {
    file_writer_.close();
  }
  delete header_;
}

template <typename K, typename V> int SkipList<K, V>::getRandomLevel() {
  int level = 1;
  while (rand() % 2) {
    level++;
  }
  return level < max_level_ ? level : max_level_;
}

template <typename K, typename V>
Node<K, V> *SkipList<K, V>::createNode(K key, V value, int level) {
  return new Node<K, V>(key, value, level);
}

template <typename K, typename V>
bool SkipList<K, V>::insertElement(K key, V value) {
  std::lock_guard<std::mutex> lck{mtx};
  Node<K, V> *cur = header_;

  Node<K, V> *update[max_level_ + 1];
  memset(update, 0, sizeof(Node<K, V> *) * (max_level_ + 1));

  for (int i = list_level_; i >= 0; i--) {
    while (cur->forward_[i] != nullptr && cur->forward_[i]->getKey() < key) {
      cur = cur->forward_[i];
    }
    update[i] = cur;
  }

  cur = cur->forward_[0];
  if (cur != nullptr && cur->getKey() == key) {
    return false;
  }

  if (cur == nullptr || cur->getKey() != key) {
    int random_level = getRandomLevel();
    if (random_level > list_level_) {
      for (int i = list_level_ + 1; i < random_level + 1; i++) {
        update[i] = header_;
      }
      list_level_ = random_level;
    }
    Node<K, V> *insert_node = createNode(key, value, random_level);
    for (int i = 0; i <= random_level; i++) {
      insert_node->forward_[i] = update[i]->forward_[i];
      update[i]->forward_[i] = insert_node;
    }
    ++element_count_;
  }
  return true;
}

template <typename K, typename V> void SkipList<K, V>::displayList() {
  // std::cout << "\n*****Skip List*****"<<"\n";
  for (int i = 0; i <= list_level_; i++) {
    Node<K, V> *node = this->_header->forward_[i];
    // std::cout << "Level " << i << ": ";
    while (node != NULL) {
      // std::cout << node->get_key() << ":" << node->get_value() << ";";
      node = node->forward_[i];
    }
    // std::cout << std::endl;
  }
}

template <typename K, typename V> V SkipList<K, V>::searchElement(K key) {
  Node<K, V> *cur = header_;
  for (int i = list_level_; i >= 0; i--) {
    while (cur->forward_[i] && cur->forward_[i]->getKey() < key) {
      cur = cur->forward_[i];
    }
  }
  cur = cur->forward_[0];

  if (cur != nullptr && cur->getKey() == key) {
    return cur->getValue();
  }
  return nullptr;
}

template <typename K, typename V> void SkipList<K, V>::deleteElement(K key) {
  std::lock_guard<std::mutex> lck{mtx};
  Node<K, V> *cur = header_;
  Node<K, V> *update[max_level_ + 1];
  memset(update, 0, sizeof(Node<K, V> *) * (max_level_ + 1));

  for (int i = list_level_; i >= 0; i--) {
    while (cur->forward_[i] && cur->forward_[i]->getKey() < key) {
      cur = cur->forward_[i];
    }
    update[i] = cur;
  }

  cur = cur->forward_[0];
  if (cur != nullptr && cur->getKey() == key) {
    for (int i = 0; i <= list_level_; i++) {
      if (update[i]->forward_[i] != cur) {
        break;
      }
      update[i]->forward_[i] = cur->forward_[i];
    }

    while (list_level_ > 0 && header_->forward_[list_level_] == nullptr) {
      --list_level_;
    }
    element_count_--;
  }
  return;
}

template <typename K, typename V> void SkipList<K, V>::dumpFile() {
  file_writer_.open(dump_file);
  Node<K, V> *node = header_->forward_[0];

  while (node != nullptr) {
    file_writer_ << node->getKey() << delimiter << node->getValue() << "\n";
    node = node->next();
  }
  file_writer_.flush();
  file_writer_.close();
}

template <typename K, typename V> void SkipList<K, V>::loadFile() {
  file_reader_.open(dump_file);
  std::string line;
  std::string key;
  std::string value;
  while (std::getline(file_reader_, line)) {
    getKeyValueFromString(line, key, value);
    if (key.empty() || value.empty()) {
      continue;
    }
    insertElement(key, value);
  }
  file_reader_.close();
}

template <typename K, typename V> int SkipList<K, V>::size() {
  return element_count_;
}

template <typename K, typename V> Node<K, V> *SkipList<K, V>::front() const {
  header_->next();
}

template <typename K, typename V>
void SkipList<K, V>::getKeyValueFromString(const std::string &str,
                                           std::string &key,
                                           std::string &value) {

  if (!isValidString(str)) {
    return;
  }
  key = str.substr(0, str.find(delimiter));
  value = str.substr(str.find(delimiter) + 1, str.length());
}

template <typename K, typename V>
bool SkipList<K, V>::isValidString(const std::string &str) {
  if (str.empty()) {
    return false;
  }
  if (str.find(delimiter) == std::string::npos) {
    return false;
  }
  return true;
}

} // namespace db
} // namespace bamboo