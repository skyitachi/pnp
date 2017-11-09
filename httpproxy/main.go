package main

import (
  "net/http"
  "crypto/tls"
  "net"
  "time"
  "io"
  "log"
)

func handleTunneling(w http.ResponseWriter, r *http.Request) {
  dest_conn, err := net.DialTimeout("tcp", r.Host, 10 * time.Second)
  if err != nil {
    http.Error(w, err.Error(), http.StatusServiceUnavailable)
    return
  }
  w.WriteHeader(http.StatusOK)
  hijacker, ok := w.(http.Hijacker)
  if !ok {
    http.Error(w, "Hijacking not supported", http.StatusInternalServerError)
    return
  }
  client_conn, _, err := hijacker.Hijack()
  if err != nil {
    http.Error(w, err.Error(), http.StatusServiceUnavailable)
    return
  }
  go transfer(dest_conn, client_conn)
  go transfer(client_conn, dest_conn)

}

func transfer(dest io.WriteCloser, source io.ReadCloser) {
  defer dest.Close()
  defer source.Close()
  io.Copy(dest, source)
}

func handleHttp(w http.ResponseWriter, r *http.Request) {
  resp, err := http.DefaultTransport.RoundTrip(r)
  if err != nil {
    http.Error(w, err.Error(), http.StatusServiceUnavailable)
    return
  }
  defer resp.Body.Close()
  w.WriteHeader(resp.StatusCode)
  copyHeader(w.Header(), resp.Header)
  io.Copy(w, resp.Body)
}

func copyHeader(dst, src http.Header) {
  for k, vv := range src {
    for _, v := range vv {
      dst.Add(k, v)
    }
  }
}

func main() {
  var pemPath = "./cert/server.pem"
  var keyPath = "./cert/server.key"

  server := &http.Server {
    Addr: ":9999",
    Handler: http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
      if r.Method == http.MethodConnect {
        handleTunneling(w, r)
      } else {
        handleHttp(w, r)
      }
    }),
    TLSNextProto: make(map[string]func(*http.Server, *tls.Conn, http.Handler)),
  }
  //log.Fatal(server.ListenAndServe())
  log.Fatal(server.ListenAndServeTLS(pemPath, keyPath))
}
