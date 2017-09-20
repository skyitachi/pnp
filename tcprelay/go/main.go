package main

import (
  "net"
  "os"
  "log"
)
var buf = make([]byte, 4096)

func handleConnection(conn net.Conn, dstConn net.Conn) {
  for {
    n, err := conn.Read(buf)
    if err != nil {
      log.Fatal(err)
    }
    if n > 0 {
      log.Printf("relay server get %d bytes data", n)
      nw, err := dstConn.Write(buf[:n])
      if err != nil {
        log.Fatal(err)
      }
      log.Printf("relay server send %d bytes data", nw)
    }
  }
  log.Printf("connection stop reads")
}

// 单向relay
func startServerListening(listener net.Listener, dstConn net.Conn) {
  for {
    conn, err := listener.Accept()
    if err != nil {
      log.Fatal(err)
    }
    go handleConnection(conn, dstConn)
  }
}

func main() {
  if len(os.Args) < 3 {
    log.Fatal("usage: go run main.go [serverport] [clientport]")
  }
  sPort, cPort := os.Args[1], os.Args[2]
  ln, err := net.Listen("tcp", ":" + sPort)
  if err != nil {
    log.Fatal(err)
  }
  log.Printf("tcprelay start listening on port: %s\n", sPort)
  dstConn, err := net.Dial("tcp", ":" + cPort)
  if err != nil {
    log.Fatal(err)
  }
  startServerListening(ln, dstConn)
}