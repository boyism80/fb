#!/bin/bash

# Set default build type - if no argument provided, use Debug
BUILD_TYPE=${1:-Debug}

echo "Building with type: $BUILD_TYPE"

pushd ..

echo "Retrieving external IP..."
EXTERNAL_IP=$(curl -s https://ifconfig.me)

sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/data:latest -f server/fb/data/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/data failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/build:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f server/fb/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/build failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/gateway:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f server/gateway/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/gateway failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/login:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f server/login/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/login failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/game:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f server/game/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/game failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/bot:latest --build-arg BUILD_TYPE=$BUILD_TYPE -f server/bot/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/bot failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/internal:latest --build-arg BUILD_TYPE=$BUILD_TYPE --build-arg SERVICE=internal -f server/http/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/internal failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/write-back:latest --build-arg BUILD_TYPE=$BUILD_TYPE --build-arg SERVICE=write-back -f server/http/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/write-back failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/admin-tool:latest --build-arg BUILD_TYPE=$BUILD_TYPE --build-arg SERVICE=admin-tool -f server/http/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/admin-tool failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/log:latest --build-arg BUILD_TYPE=$BUILD_TYPE --build-arg SERVICE=log -f server/http/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/log failed"
    exit $?
fi
sudo docker buildx build --progress=plain --push --tag ghcr.io/boyism80/fb/marketplace:latest --build-arg BUILD_TYPE=$BUILD_TYPE --build-arg SERVICE=marketplace -f server/http/Dockerfile .
if [ $? -ne 0 ]; then
    echo "build fb/marketplace failed"
    exit $?
fi
pushd infra/pulumi
pulumi config set --secret host "$EXTERNAL_IP"
pulumi down -y && pulumi up -y
popd
popd