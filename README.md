## fb
fb is 2d mmorpg game server implemented by ```C++20``` and ```ASP.NET CORE 8.0``` and ```lua.5.3```.

## Instasllation
### Setup configuration
Create ```config.json``` file into ```each-server-dir/config```. This file required to run server. If you set environment variable ```KINGDOM_OF_WIND_ENVIRONMENT```, server load config file named ```config.{KINGDOM_OF_WIND_ENVIRONMENT}.json``` file. Sample config files are : <br>
[config file for gateway](https://github.com/boyism80/fb/blob/develop/gateway/config/config.dev.json)<br>
[config file for login](https://github.com/boyism80/fb/blob/develop/login/config/config.dev.json)<br>
[config file for game](https://github.com/boyism80/fb/blob/develop/game/config/config.dev.json)<br>

And you have to unzip ```resources/maps.zip``` file into directory that contains binary game server. Then convert excel files in ```resources/table``` to json files using [data-converter](https://github.com/boyism80/data-converter). If you update protocol that used for communicate with internal server, modify fbs file in ```protocol``` path and run [flatbuffer-ex](https://github.com/boyism80/flatbuffer-ex). 

### On Windows
Install git, cmake, visual studio 2022 and run 'tools/update-modules.bat'. then run this:
```
mkdir build
cd build
cmake ..
```
Build 'fb.sln' and execute all servers.


### On Linux
Install docker.io and run this:
```
docker build --tag fb/build:latest -f Dockerfile .
docker build --tag fb/gateway:latest -f gateway/Dockerfile .
docker build --tag fb/login:latest -f login/Dockerfile .
docker build --tag fb/game:latest -f game/Dockerfile .
docker build --tag fb/bot:latest -f bot/Dockerfile .
docker build --tag fb/internal:latest -f http/Dockerfile --build-arg SERVICE=internal .
docker build --tag fb/write-back:latest -f http/Dockerfile --build-arg SERVICE=write-back .
```

You can run all servers simply using kubernetes and pulumi. First, change ```host``` field in ```infra/pulumi/develop.json``` file.
```json
{
    "host": {
        "private": "{enter your private ip}",
        "public": "{enter your external ip}"
    },
    "mysql": {
        "section-1": {
            "-1": {
                "port": {
                    "cluster": 3306,
                    "node": 30100
                }
            },
            "0": {
                "port": {
                    "cluster": 3307,
                    "node": 30101
                }
            },
```
Then install [pulumi](https://github.com/pulumi/pulumi) and call ```pulumi up -y``` command in ```infra/pulumi``` directory.

## Build
If you need to build, look ```Dockerfile``` and ```CMakeLists.txt``` and ```build.sh``` files. ```lib``` module is used by other serveres so you build ```lib``` first. Then build ```gateway```, ```login```, ```game```, ```internal``` serveres.


## Architecture
![screenshot](image/architecture.png)
The gateway server is responsible for routing to one of several login servers, which create accounts, change passwords, and route to game servers. The server-to-server communication is via the internal server.

internal server is based on HTTP 2.0. It reads and writes DBs, notifies all servers (using RabbitMQ), inquires about the status of multiple servers, synchronizes, and so on.

'write-back' is a single process that will carry out a redis caching strategy. When the internal server writes data to the DB, it saves it to Redis first immediately and stores the query to be passed to the actual DB in the Redis buffer. The 'write-back' process continues to query this buffer and applies the query to the DB when it can be done.


### Game server & map
![screenshot](image/map_group.png)
Each game server manages some of the map groups. Unmanaged maps do not perform any operations, and when a character moves to an unmanaged map, it transfers through the internal server to the game server that manages that map.

As shown in the picture above, there are several maps that are managed by a single game server. According to the picture, game servers with id of 0 only manage maps with map id from 0 to 99. This is for ease of description, and in reality, each map has an id of the game server that you need to connect to.

In addition, one map consists of several sectors. It is used to efficiently search for surrounding objects from one object. To the right of the picture above, there are sectors from 0 to 19. Assuming that the character is in sector 7, the game server will search for objects in sectors [1, 2, 3, 6, 7, 8, 11, 12, 13].

### Thread
![screenshot](image/thread.png)
After the I/O thread reads the data, it passes the data to one of several logic threads. The game server determines the thread as follows:

```C++
thread_id = character.map.id % (logic_thread.length+1)
```

For example, if the character who received the data from the I/O thread is currently on the map 40, thread 0 would call the handler. If it was on the map 62, it would be called in thread 2. This makes it possible to avoid considering the concurrency between objects on the same map. However, it makes it difficult to interact between characters located on different maps.

The gateway server or login server determines by modulating the fd in the socket. The number of I/O threads and logical threads can also be determined by the configuration file.

## Contact
 - [youtube](https://www.youtube.com/channel/UCPcH5qX7aLTFs3mgh32_FVQ?view_as=subscriber)
 - [blog](https://blog.naver.com/boyism)
