# *First OS course project*
## `a naive local chat app with c!`

* [X] Adding   Group
* [X] Sharing  Group List
* [X] Joining  to Group
* [X] Private  Chat (client->master->client)
* [X] Secret   Chat (p2p)
* [X] Left     From Group
* [X] Deleting inactive Groups (Heatbeat)
* [X] Adding   Logs to Server

## run the project using following commands:
```properties
make
./server MASTER_SERVER_PORT
./client MASTER_SERVER_PORT
```
## for example:
```properties
make
./server 9002
./client 9002
```