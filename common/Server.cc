#include "net/BambooServer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"

using namespace bamboo;

int main(int argc, char *argv[]) {
  EventLoop loop;
  InetAddress listenAddr(9981);
  BambooServer server(&loop, listenAddr);
  server.start();
  loop.loop();
}