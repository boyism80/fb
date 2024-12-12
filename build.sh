docker build --tag ghcr.io/boyism80/fb/build:latest -f Dockerfile .
docker push ghcr.io/boyism80/fb/build:latest

docker build --tag ghcr.io/boyism80/fb/gateway:latest -f gateway/Dockerfile .
docker push ghcr.io/boyism80/fb/gateway:latest

docker build --tag ghcr.io/boyism80/fb/login:latest -f login/Dockerfile .
docker push ghcr.io/boyism80/fb/login:latest

docker build --tag ghcr.io/boyism80/fb/game:latest -f game/Dockerfile .
docker push ghcr.io/boyism80/fb/game:latest

docker build --tag ghcr.io/boyism80/fb/internal:latest -f http/Dockerfile --build-arg SERVICE=internal .
docker push ghcr.io/boyism80/fb/internal:latest