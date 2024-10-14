#include "net/TcpServer.h"

#include "base/Logging.h"
#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"

namespace bamboo {

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listen_addr,
                     const std::string &name, Option option)
    : loop_(loop), ip_port_(listen_addr.toIpPort()), name_(name),
      acceptor_(new Acceptor(loop, listen_addr, option == Option::kReusePort)),
      thread_pool_(new EventLoopThreadPool(loop, name_)) {}

TcpServer::~TcpServer() {
  for (auto &item : connections_) {
    TcpConnectionPtr conn = item.second;
    item.second.reset();
    conn->getLoop()->runInLoop(bind(&TcpConnection::connectDestroyed, conn));
  }
}

void TcpServer::setThreadNum(int threads_num) {
  thread_pool_->setThreadNum(threads_num);
}

void TcpServer::start() {
  if (started_++ == 0) {
    thread_pool_->start(thread_init_callback_);

    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peer_addr) {
  auto io_loop = thread_pool_->getNextLoop();
  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_conn_id_++);
  auto conn_name = name_ + buf;
  LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
           << conn_name << "] from " << peer_addr.toIpPort();

  sockaddr_in local;
  bzero(&local, sizeof(local));
  socklen_t addr_len = sizeof(local);
  if (::getsockname(sockfd, (sockaddr *)&local, &addr_len) < 0) {
    LOG_SYSFATAL << "sockets::getLocalAddr";
  }

  InetAddress local_addr(local);
  auto conn = std::make_shared<TcpConnection>(io_loop, sockfd, conn_name,
                                              local_addr, peer_addr);
  connections_.emplace(conn_name, conn);

  conn->setConnectionCallback(connection_callback_);
  conn->setMessageCallback(message_callback_);
  conn->setWriteCompleteCallback(write_complete_callback_);
  conn->setCloseCallback(
      bind(&TcpServer::removeConnection, this, std::placeholders::_1));
  io_loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnnectionInLoop, this, conn));
}

void TcpServer::removeConnnectionInLoop(const TcpConnectionPtr &conn) {
    LOG_INFO << "TcpServer::removeConnection [" << name_ << "] - connection " << conn->name();
    connections_.erase(conn->name());
    auto io_loop = conn->getLoop();
    io_loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn)); 
}

} // namespace bamboo