## fb
fb is 2d mmorpg game server implemented by ```C++20``` and ```ASP.NET CORE 8.0```.

## Instasllation
### Setup configuration
Create ```config.json``` file into ```each-server-dir/config```. This file required to run server. If you set environment variable ```KINGDOM_OF_WIND_ENVIRONMENT```, server load config file named ```config.{KINGDOM_OF_WIND_ENVIRONMENT}.json``` file. Sample config files are : <br>
[config file for gateway](https://github.com/boyism80/fb/blob/develop/gateway/config/config.dev.json)<br>
[config file for login](https://github.com/boyism80/fb/blob/develop/login/config/config.dev.json)<br>
[config file for game](https://github.com/boyism80/fb/blob/develop/game/config/config.dev.json)<br>

And you have to unzip ```resources/maps.zip``` file into directory that contains binary game server. Then convert excel files in ```resources/table``` to json files using [data-converter](https://github.com/boyism80/data-converter). If you update protocol that used for communicate with internal server, modify fbs file in ```protocol``` path and run [flatbuffer-ex](https://github.com/boyism80/flatbuffer-ex). 

### Use Docker
```
# create config file into $PWD/{each-server}/config
sudo docker run -v $PWD/gateway/config:/app/config ghcr.io/boyism80/fb/gateway:latest
sudo docker run -v $PWD/login/config:/app/config ghcr.io/boyism80/fb/login:latest
sudo docker run -v $PWD/game/config:/app/config ghcr.io/boyism80/fb/game:latest
sudo docker run -v $PWD/internal/config:/app/config ghcr.io/boyism80/fb/internal:latest
```


### Use Kubernetes
You can run all servers simply using kubernetes and pulumi. First, change ```host``` field in ```infra/pulumi/develop.json``` file.
```json
{
    "host": "{enter your host}",
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
TODO



## Contact
 - [youtube](https://www.youtube.com/channel/UCPcH5qX7aLTFs3mgh32_FVQ?view_as=subscriber)
 - [blog](https://blog.naver.com/boyism)
