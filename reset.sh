git reset --hard && git clean -fxd
git fetch && git pull
git submodule foreach git reset --hard && git clean -fxd
git submodule update --init --recursive
cd infra/pulumi
npm install
cd ../..
