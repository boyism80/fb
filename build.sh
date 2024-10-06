docker build --tag cshyeon/fb:gateway -f deploy/Docker.fb.gateway .
docker push cshyeon/fb:gateway

docker build --tag cshyeon/fb:login -f deploy/Docker.fb.login .
docker push cshyeon/fb:login

docker build --tag cshyeon/fb:game -f deploy/Docker.fb.game .
docker push cshyeon/fb:game

docker build --tag cshyeon/fb:internal -f deploy/Docker.fb.dotnet --build-arg SERVICE=internal .
docker push cshyeon/fb:internal

docker build --tag cshyeon/fb:db -f deploy/Docker.fb.dotnet --build-arg SERVICE=db .
docker push cshyeon/fb:db