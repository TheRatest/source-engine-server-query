# Source Engine Server Query
A simple CLI tool using windows sockets to get info about a source engine server.

## Prerequisites
* [G++](https://gcc.gnu.org/install/index.html)

## Compilation
Run `g++ main.cpp Connection.cpp -o SourceEngineServerQuery.exe -lws2_32 -LC:\Windows\System32`

## Usage
### Flags:
```
-a, -addr, -address <IP:port> - Specify the server address to connect to
-ip <IP> - Specify the server IP
-port <port> - Specify the server port (Defaults to 27015)
-p, -players - Also get active players
-r, -rules - Also get server rules
-o, -out, -output <file path> - Output to a file instead of the console
```

### Examples:
`SourceEngineServerQuery.exe -addr 193.221.192.30:27015`  
`SourceEngineServerQuery -ip 193.221.192.30 -port 27015 -output ugc-tf-mge.txt -players`  
`sourceengineserverquery -ip 193.221.192.30 -o ugc-tf-mge-detailed.txt -p -r`  
