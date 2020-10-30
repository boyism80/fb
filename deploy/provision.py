import os
import json
from fabric.api import *
from fabric.contrib import files

env.hosts = ['192.168.0.180', '192.168.0.181', '192.168.0.182']
env.user = 'ubuntu'
env.password = 'tmdgus12!@'
env.port = 22
env.timeout = 60

@parallel
def update():
    if files.exists('/var/provision_completed'):
        return

    sudo('apt-get update -y')
    sudo('apt-get upgrade -y')
    sudo('apt-get install make -y')
    sudo('apt-get install wget -y')
    sudo('apt-get install libjsoncpp-dev -y')
    sudo('apt-get install lua5.3-dev -y')
    sudo('apt-get install libmysqlcppconn-dev -y')
    sudo('apt-get install libmysqlclient-dev -y')
    sudo('apt-get install mysql-server -y')
    sudo('apt-get install redis -y')
    sudo('apt-get install libssl-dev -y')
    sudo('apt-get install libncurses5-dev libncursesw5-dev -y')
    sudo('apt-get install python3-pip -y')
    sudo('apt-get install default-jre -y')
    sudo('apt-get install docker docker.io')
    sudo('apt-get install unzip -y')

    if not files.exists('/var/provision_completed'):
        sudo('pip3 install --upgrade --ignore-installed pip setuptools')
        sudo('pip3 install fabric3')
        sudo('pip3 install jinja2')

        sudo('wget http://ftp.superuser.co.kr/pub/etc/zlib-1.1.4.tar.gz')
        sudo('tar xvfz zlib-1.1.4.tar.gz')
        with cd('zlib-1.1.4'):
            sudo('./configure && make && make install')
        sudo('rm -rf zlib-1.1.4*')

        sudo('wget -O boost_1_74_0.tar.gz https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.zip')
        sudo('unzip boost_1_74_0.tar.gz')
        with cd('boost_1_74_0'):
            sudo('./bootstrap.sh --prefix=/usr/')
            sudo('./b2')
            sudo('./b2 install')
        sudo('rm -rf boost_1_74_0*')

        with cd('/var'):
            files.append('provision_completed', '', use_sudo=True)