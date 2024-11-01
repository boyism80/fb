docker build --tag cshyeon/fb:build -f infra/Docker.fb.build .
docker push cshyeon/fb:build

docker build --tag cshyeon/fb:gateway -f infra/Docker.fb.gateway .
docker push cshyeon/fb:gateway

docker build --tag cshyeon/fb:login -f infra/Docker.fb.login .
docker push cshyeon/fb:login

docker build --tag cshyeon/fb:game -f infra/Docker.fb.game .
docker push cshyeon/fb:game

docker build --tag cshyeon/fb:internal -f infra/Docker.fb.dotnet --build-arg SERVICE=internal .
docker push cshyeon/fb:internal

docker build --tag cshyeon/fb:db -f infra/Docker.fb.dotnet --build-arg SERVICE=db .
docker push cshyeon/fb:db