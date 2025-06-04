#!/bin/bash

echo "Retrieving external IP..."
EXTERNAL_IP=$(curl -s https://ifconfig.me)

sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/build:latest -f lib/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/build failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/gateway:latest -f gateway/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/gateway failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/login:latest -f login/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/login failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/game:latest -f game/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/game failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/bot:latest -f bot/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/bot failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/internal:latest -f http/Dockerfile --build-arg SERVICE=internal .
if [ $? -ne 0 ]; then
    echo "build fb/internal failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/write-back:latest -f http/Dockerfile --build-arg SERVICE=write-back .
if [ $? -ne 0 ]; then
    echo "build fb/write-back failed"
    exit $?
fi
pushd infra/pulumi
pulumi config set --secret host "$EXTERNAL_IP"
pulumi down -y && pulumi up -y
popd
