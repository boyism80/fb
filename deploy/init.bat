REM download ubuntu image
curl https://lxrunoffline.apphb.com/download/Ubuntu/bionic -J -L -o ubuntu.tar.gz

REM re-install ubuntu instance
LxRunOffline.exe uninstall -n ubuntu
LxRunOffline.exe install -n ubuntu -f ubuntu.tar.gz -d ubuntu

REM delete ubuntu image
del /f ubuntu.tar.gz

REM provision
LxRunOffline.exe run -n ubuntu -c ./provision.sh
LxRunOffline.exe run -n ubuntu