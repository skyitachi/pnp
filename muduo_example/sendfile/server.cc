#include <boost/bind.hpp>
#include <stdio.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

const char *g_file = NULL;

string readFile(const char* filename) {
  string content;
  FILE *fp = ::fopen(filename, "rb");
  if (fp) {
    const int kBufSize = 1024 * 1024;
    char iobuf[kBufSize];
    ::setbuffer(fp, iobuf, sizeof(iobuf));
    char buf[kBufSize];
    size_t nread = 0;
    while ((nread = ::fread(buf, 1, sizeof(buf), fp)) > 0) {
      content.append(buf, nread);
    }
    ::fclose(fp);
  }
  return content;
}

// 读文件, 然后send给客户端
void onConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "Connection comes: " << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is " << (conn->connected() ? "up" : "down");
  if (conn->connected()) {
    LOG_INFO << "sending file " << g_file << " to " << conn->peerAddress().toIpPort();
    string fileContent = readFile(g_file);
    conn->send(fileContent);
    conn->shutdown();
    LOG_INFO << "FileServer - done";
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: filetransfer <filename>\n");
    exit(1);
  }
  g_file = argv[1];
  EventLoop loop;
  InetAddress listenAddr(2021);
  TcpServer server(&loop, listenAddr, "FileServer");
  server.setConnectionCallback(onConnection);
  server.start();
  loop.loop();
  return 0;
}
