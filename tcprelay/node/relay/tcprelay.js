const net = require("net");

// relay server
const server = net.createServer(function (socket) {
  const relaySocket = net.connect({ port: 9002, host: "localhost" });
  
  // client pipe to remote server
  socket.pipe(relaySocket);
  // remote server pipe to client
  relaySocket.pipe(socket);
  let cnt = 0;
  socket.on("data", function (buffer) {
    cnt += buffer.byteLength;
  });
  socket.on("close", function () {
    console.log(`forward ${cnt} bytes data`);
  });
  let relayCnt = 0;
  relaySocket.on("data", function (buffer) {
    relayCnt += buffer.byteLength;
  });
  relaySocket.on("close", function () {
    console.log(`backforward ${relayCnt} bytes data`);
  });
  
});

server.listen(9001, function () {
  console.log("relay server start working");
});