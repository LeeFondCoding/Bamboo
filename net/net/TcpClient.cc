#include "net/TcpClient.h"

#include "base/Logging.h"
#include "net/Connector.h"
#include "net/EventLoop.h"
#include "net/SocketOps.h"

#include "assert.h"

namespace bamboo {
namespace detail {

void removeConnnection(EventLoop *loop, const TcpConnectionPtr &conn) {
  loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr &connector) {}

} // namespace detail

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr,
                     const std::string &nameArg)
    : loop_(loop), name_(nameArg), connector_(new Connector(loop, serverAddr)) {
}

TcpClient::~TcpClient() {
  LOG_INFO << "TcpClient::~TcpClient [" << name_ << "] - connector "
           << connector_.get();
  bool unique = false;

  std::unique_lock<std::mutex> lck{mutex_};
  unique = connection_.unique();
  auto conn = connection_;
  lck.unlock();

  if (conn != nullptr) {
    assert(loop_ == conn->getLoop());
    CloseCallback cb =
        std::bind(detail::removeConnnection, loop_, std::placeholders::_1);
    loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
    if (unique) {
      conn->forceClose();
    }
  } else {
    connector_->stop();
    loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
  }
}

void TcpClient::connect() {
  LOG_INFO << "TcpClient::connect [" << name_ << "] - connecting to "
           << connector_->serverAddress().toIpPort();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  connect_ = false;
  std::lock_guard<std::mutex> lck{mutex_};
  if (connection_ != nullptr) {
    connection_->shutdown();
  }
}

void TcpClient::stop() {
  connect_ = false;
  connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
  loop_->assertInLoopThread();
  InetAddress peer_addr(sockets::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof(buf), ":%s#%d", peer_addr.toIpPort().c_str(),
           next_conn_id_);
  ++next_conn_id_;
  std::string conn_name = name_ + buf;

  InetAddress local_addr(sockets::getLocalAddr(sockfd));
  TcpConnectionPtr conn(
      new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
  {
    std::lock_guard<std::mutex> lck{mutex_};
    connection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    std::lock_guard<std::mutex> lck{mutex_};
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_) {
    LOG_INFO << "TcpClient::connect [" << name_ << "] - Reconnecting to "
             << connector_->serverAddress().toIpPort();
    connector_->restart();
  }
}

} // namespace bamboo