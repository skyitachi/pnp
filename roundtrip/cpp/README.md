### test result
#### environments
- localhost(same host)

```shell
# server
./roundtrip

#client
./roundtrip -c

#client output
client send request: 1493043803533572
client and server offset is -40 usec
```
### explain
`offset = (T1 + T3) / 2 - T2 / 2`, 由于从server端接受到数据到发出去数据需要从内核态切换到用户态（recv)再切换回内核态(send)需要时间，这个时间大致就是offset
