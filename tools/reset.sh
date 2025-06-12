pushd ..
git reset --hard && git clean -fxd
git fetch && git pull
git submodule foreach git reset --hard && git clean -fxd
git submodule update --init --recursive
pushd infra/pulumi
npm install
popd
popd
