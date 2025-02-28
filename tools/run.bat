pushd runner
dotnet publish -c Release -o bin
start bin\runner.exe
popd