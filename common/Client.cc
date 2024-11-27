#include "net/BambooClient.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"

using namespace bamboo;

int main(int argc, char *argv[]) {
  EventLoop loop;
  // Assuming server is running locally on port 9981
  InetAddress serverAddr(9981, "127.0.0.1");
  BambooClient client(&loop, serverAddr);
  client.connect();
  loop.loop();
}