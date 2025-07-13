#!/bin/bash

# Set default build type - if no argument provided, use Debug
BUILD_TYPE=${1:-Debug}

echo "Building with type: $BUILD_TYPE"

pushd ..

echo "Retrieving external IP..."
EXTERNAL_IP=$(curl -s https://ifconfig.me)

sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/build:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f lib/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/build failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/gateway:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f gateway/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/gateway failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/login:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f login/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/login failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/game:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f game/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/game failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/bot:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f bot/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/bot failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/internal:latest --build-arg BUILD_TYPE=$BUILD_TYPE --build-arg SERVICE=internal -f http/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/internal failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/write-back:latest --build-arg BUILD_TYPE=$BUILD_TYPE --build-arg SERVICE=write-back -f http/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/write-back failed"
    exit $?
fi
pushd infra/pulumi
pulumi config set --secret host "$EXTERNAL_IP"
pulumi down -y && pulumi up -y
popd
popd