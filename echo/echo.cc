#include "echo.h"

#include <boost/bind.hpp>
#include <stdio.h>
#include <iostream>

EchoServer::EchoServer(muduo::net::EventLoop* loop,
                       const muduo::net::InetAddress& listenAddr)
  :loop_(loop), server_(loop, listenAddr, "EchoServer") {
  server_.setConnectionCallback(
    boost::bind(&EchoServer::onConnection, this, _1)
  );

  server_.setMessageCallback(
    boost::bind(&EchoServer::onMessage, this, _1, _2, _3)
  );
}


void EchoServer::onConnection(const muduo::net::TcpConnectionPtr& conn) {
  printf("connected\n");
}

void EchoServer::start() {
  server_.start();
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                      muduo::net::Buffer* buf,
                      muduo::Timestamp time) {
  muduo::string msg(buf->retrieveAllAsString());

  printf("%d size data received at ", msg.size());
  std::cout << time.toString() << std::endl;  
  conn->send(msg);
}
