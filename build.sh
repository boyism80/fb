docker build --tag cshyeon/fb:build -f Dockerfile.fb.build .
docker push cshyeon/fb:build

docker build --tag cshyeon/fb:gateway -f gateway/Dockerfile .
docker push cshyeon/fb:gateway

docker build --tag cshyeon/fb:login -f login/Dockerfile .
docker push cshyeon/fb:login

docker build --tag cshyeon/fb:game -f game/Dockerfile .
docker push cshyeon/fb:game

docker build --tag cshyeon/fb:internal -f http/Dockerfile --build-arg SERVICE=internal .
docker push cshyeon/fb:internal

docker build --tag cshyeon/fb:db -f http/Dockerfile --build-arg SERVICE=db .
docker push cshyeon/fb:db