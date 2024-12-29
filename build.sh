#!/bin/bash

sudo docker build --tag ghcr.io/boyism80/fb/build:latest -f Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/build failed"
    exit $?
fi
sudo docker build --tag ghcr.io/boyism80/fb/gateway:latest -f gateway/Dockerfil .
if [ $? -ne 0 ]; then
    echo "build fb/gateway failed"
    exit $?
fi
sudo docker build --tag ghcr.io/boyism80/fb/login:latest -f login/Dockerfil .
if [ $? -ne 0 ]; then
    echo "build fb/login failed"
    exit $?
fi
sudo docker build --tag ghcr.io/boyism80/fb/game:latest -f game/Dockerfil .
if [ $? -ne 0 ]; then
    echo "build fb/game failed"
    exit $?
fi
sudo docker build --tag ghcr.io/boyism80/fb/bot:latest -f bot/Dockerfil .
if [ $? -ne 0 ]; then
    echo "build fb/bot failed"
    exit $?
fi
sudo docker build --tag ghcr.io/boyism80/fb/internal:latest -f http/Dockerfile --build-arg SERVICE=interna .
if [ $? -ne 0 ]; then
    echo "build fb/internal failed"
    exit $?
fi
sudo docker build --tag ghcr.io/boyism80/fb/write-back:latest -f http/Dockerfile --build-arg SERVICE=write-bac .
if [ $? -ne 0 ]; then
    echo "build fb/write-back failed"
    exit $?
fi
pushd infra/pulumi
pulumi down -y && pulumi up -y
popd