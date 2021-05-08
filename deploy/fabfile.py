# coding: utf-8

from fabric.api import *
from fabric.contrib import files
from pathlib import Path
import json
import jinja2
import os
import shutil
import zipfile
import optimizer
import time
import copy

env.user = 'ubuntu'
env.key_filename = 'ssh/fb'
env.port = 22
env.timeout = 60

LOCAL_ROOT = './deploy'
CONFIGURATION = None
TABLES = {}

@task
def environment(e):
    global CONFIGURATION

    with open(f'{LOCAL_ROOT}/{e}.json') as f:
        CONFIGURATION = json.load(f)

    for service in CONFIGURATION['deploy']:
        env['roledefs'][service] = {'hosts': []}

        for name, config in CONFIGURATION['deploy'][service].items():
            if config['ip'] not in env['roledefs'][service]['hosts']:
                env['roledefs'][service]['hosts'].append(config['ip'])

            if config['ip'] not in env['hosts']:
                env['hosts'].append(config['ip'])

@task
@runs_once
def optimize():
    with lcd('resources/maps'):
        local('rm -f *.map')
        local('rm -f *.block')
        local('unzip -qq maps.zip')

    # for internal
    hosts = optimizer.hosts()
    with open('host.json', 'w', encoding='utf8') as f:
        f.write(json.dumps(hosts, ensure_ascii=False, sort_keys=True))

    # for game
    local('rm -rf temp && mkdir -p temp')
    optimizer.convert(os.path.join('resources', 'table'), 'temp')
    optimizer.compress(maps=os.path.join('resources', 'maps'),
                       tables='temp',
                       scripts=os.path.join('game', 'scripts'),
                       dst='resources.zip')
    local('rm -rf temp')

@task
@parallel
def setup():
    configs = current()
    if 'internal' in configs['deploy']:
        sudo('rm -rf internal', quiet=True)
        sudo('mkdir -p internal/table', quiet=True)
        with cd('internal/table'):
            put(f'host.json', '.', use_sudo=True)

    if 'game' in configs['deploy']:
        sudo('rm -rf game', quiet=True)
        sudo('mkdir -p game', quiet=True)
        with cd('game'):
            put('resources.zip', '.', use_sudo=True)
            sudo('unzip -qq resources.zip', quiet=True)
            sudo('rm -f resources.zip', quiet=True)

@task
def clean():
    local('rm -f host.json')
    local('rm -f resources.zip')

@task
def build(service):
    local(f'docker run -v $PWD:/app -i -w /app/{service} cshyeon/fb:build make -j4')


@task
@parallel
def deploy(service):
    global CONFIGURATION

    configs = current()
    if service not in configs['deploy']:
        return

    context = copy.deepcopy(CONFIGURATION)
    sudo(f'mkdir -p {service}', quiet=True)
    with cd(service):
        for name, config in configs['deploy'][service].items():
            context.update({'name': name})

            template = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=f"{LOCAL_ROOT}/template")).get_template(f'config.{service}.txt')
            sudo(f"echo '{template.render(context)}' > config.{name}.json", quiet=True)

        put(f'dist/fb/{service}/{service}', '.', use_sudo=True, mode='0755')

        for name, config in configs['deploy'][service].items():
            sudo(f"docker run -d -it --name fb_{name} -v $PWD:/app -w /app -e LC_ALL=C.UTF-8 -p {config['port']}:{config['port']} cshyeon/fb:latest ./{service} --env {name}", quiet=True)

def current():
    global CONFIGURATION

    result = copy.deepcopy(CONFIGURATION)

    for service, configs in CONFIGURATION['deploy'].items():
        for name, config in configs.items():
            if config['ip'] == env.host:
                continue

            del result['deploy'][service][name]

    for service in [x for x in result['deploy']]:
        if not result['deploy'][service]:
            del result['deploy'][service]

    return result

@task
@parallel
def stop():
    with settings(warn_only=True):
        sudo('docker ps --filter name=fb_* -aq | xargs docker stop | xargs docker rm', quiet=True)

def container_names(host):
    global CONFIGURATION

    result = []
    for services in CONFIGURATION['deploy'].values():
        for name, service in services.items():
            if service['ip'] == host:
                result.append(name)

    return result


@task
@parallel
def prune():
    with settings(warn_only=True):
        sudo('docker container prune -f', quiet=True)
        sudo('docker image prune -f', quiet=True)