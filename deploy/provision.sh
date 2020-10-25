apt-get update -y
apt-get upgrade -y
apt-get install make -y
apt-get install libjsoncpp-dev -y
apt-get install lua5.3-dev -y
apt-get install libmysqlcppconn-dev -y
apt-get install libmysqlclient-dev -y

wget http://ftp.superuser.co.kr/pub/etc/zlib-1.1.4.tar.gz
tar xvfz zlib-1.1.4.tar.gz
cd zlib-1.1.4
./configure && make && make install
cd ..
rm -rf zlib-1.1.4*

apt-get install wget -y
wget -O boost_1_74_0.tar.gz https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.zip
apt-get install unzip -y
unzip boost_1_74_0.tar.gz
cd boost_1_74_0
./bootstrap.sh --prefix=/usr/
./b2
./b2 install

apt-get install libssl-dev -y
apt-get install libncurses5-dev libncursesw5-dev -y

apt-get install python3-pip -y
pip3 install fabric3
pip3 install jinja2