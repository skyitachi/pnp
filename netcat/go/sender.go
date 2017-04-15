package main

import (
    "log"
    "net"
    "os"
    "time"
    "io"
)

func check(err error) {
    if err != nil {
        log.Fatal(err)
    }
}

// close directly seems no influence to data integrity
func connectionHandler(conn *net.TCPConn) {
    conn.SetReadBuffer(8192)
    log.Println("Starting Connection goroutine")
    time.Sleep(time.Second * 10)
    fp, err := os.Open("./test.dat")
    defer fp.Close()
    check(err)
    buf := make([]byte, 8192)
    for{
        nread, err := fp.Read(buf)
        if err == io.EOF {
            log.Println("Data Sended")
            break
        }
        check(err)
        _, err = conn.Write(buf[:nread])
        check(err)
    }
    conn.Close()
    // Note: CloseWrite should do
    //conn.CloseWrite()
    //for {
    //    nread, err := conn.Read(buf)
    //    if err == io.EOF {
    //        break
    //    }
    //    check(err)
    //    log.Printf("Received %d byte data\n", nread)
    //}
    log.Println("stop the connection")
}

func main() {
    ip := net.ParseIP("127.0.0.1")
    server, err := net.ListenTCP("tcp", &net.TCPAddr{IP:ip, Port: 10000})
    check(err)
    for {
        conn, err := server.AcceptTCP()
        check(err)
        go connectionHandler(conn)
    }
}
