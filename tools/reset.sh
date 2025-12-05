pushd ..
git reset --hard
git clean -fxd
git fetch
git pull
git submodule foreach 'git reset --hard; git clean -fxd'
git submodule foreach git fetch
git submodule update --init --recursive --remote --force
pushd infra/pulumi
npm install
popd
popd
