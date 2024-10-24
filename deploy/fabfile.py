# coding: utf-8

from fabric.api import *
from fabric.contrib import files
from pathlib import Path
import json
import jinja2
import os
import shutil
import zipfile
import time
import copy
import pathlib

env.timeout = 60
CONFIGURATION = None

@task
def environment(e):
    global CONFIGURATION

    with open(f'deploy/{e}.json', encoding='utf8') as f:
        CONFIGURATION = json.load(f)

    env.user = CONFIGURATION['authorization']['user']
    env.key_filename = CONFIGURATION['authorization']['key']
    env.port = CONFIGURATION['authorization']['port']

    env.roledefs = {service:list(set([x['ip'] for x in configs.values()])) for service, configs in CONFIGURATION['deploy'].items()}
    env.hosts = list(set([host for role in env.roledefs.values() for host in role]))

@task
@runs_once
def optimize():
    # for internal
    with open('game/json/map.json', 'r', encoding='utf8') as f:
        maps = json.load(f)
        hosts = {x['id']:x['host'] for x in maps.values()}
    
    with open('host.json', 'w', encoding='utf8') as f:
        f.write(json.dumps(hosts, ensure_ascii=False, sort_keys=True))

    zfile = zipfile.ZipFile('resources.zip', 'w')
    zfile.write('resources/maps/maps.zip', 'maps.zip')
    for (path, dir, files) in os.walk('game/scripts'):
        for file in files:
            if not file.endswith('.lua'):
                continue

            src = os.path.join(path, file)
            dst = os.path.join(*pathlib.Path(path).parts[1:], file)
            zfile.write(src, dst)

    for (path, dir, files) in os.walk('game/json'):
        for file in files:
            if not file.endswith('.json'):
                continue

            src = os.path.join(path, file)
            dst = os.path.join(*pathlib.Path(path).parts[1:], file)
            zfile.write(src, dst)

    zfile.close()

@task
@parallel
def setup():
    configs = current()
    if 'internal' in configs['deploy']:
        sudo('rm -rf internal', quiet=True)
        sudo('mkdir -p internal/table', quiet=True)
        put(f'host.json', 'internal/table/host.json', use_sudo=True)

    if 'game' in configs['deploy']:
        sudo('rm -rf game', quiet=True)
        sudo('mkdir -p game', quiet=True)

        put('resources.zip', 'game/resources.zip', use_sudo=True)
        with cd('game'):
            sudo('unzip -qq resources.zip', quiet=True)
            sudo('rm -f resources.zip', quiet=True)
            sudo('mkdir -p maps', quiet=True)
            sudo('mv maps.zip maps', quiet=True)
            with cd('maps'):
                sudo('unzip -qq maps.zip', quiet=True)
                sudo('rm -f maps.zip', quiet=True)

@task
def clean():
    local('rm -f host.json')
    local('rm -f resources.json')

@task
def build_cpp(service):
    local(f'docker pull cshyeon/fb:build')
    local(f'docker run -v $PWD:/app -i -w /app/{service}/build cshyeon/fb:build /bin/bash -c "cmake .. && make -j4 && cp ./app /app/dist/fb/{service}/app"')

@task
def build_dotnet(service):
    local(f'docker build --tag cshyeon/fb/dotnet:latest --build-arg SERVICE={service} -f ./deploy/Dockerfile.net .')

@task
@parallel
def deploy_cpp(service):
    global CONFIGURATION

    configs = current()
    if service not in configs['deploy']:
        return

    context = copy.deepcopy(CONFIGURATION)
    sudo(f'mkdir -p {service}', quiet=True)
    with cd(service):
        for name, config in configs['deploy'][service].items():
            context.update({'name': name})

            template = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=f"deploy/template")).get_template(f'config.{service}.txt')
            json_str = template.render(context)
            json.loads(json_str)
            sudo(f"echo '{json_str}' > config.{name}.json", quiet=True)

        put(f'dist/fb/{service}/app', '.', use_sudo=True, mode='0755')

        for name, config in configs['deploy'][service].items():
            sudo(f'docker pull cshyeon/fb:latest')
            sudo(f"docker run -d -it --name fb_{name} -v $PWD:/app --restart unless-stopped -w /app -e LC_ALL=C.UTF-8 -e KINGDOM_OF_WIND_ENVIRONMENT={name} -p {config['port']}:{config['port']} cshyeon/fb:latest ./app")

@task
def generate_config_file(service,output='.'):
    global CONFIGURATION
    configs = current()
    if service not in configs['deploy']:
        return

    os.makedirs(output, exist_ok=True)
    context = copy.deepcopy(CONFIGURATION)
    for name, config in configs['deploy'][service].items():
        context.update({'name': name})

        template = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=f"deploy/template")).get_template(f'config.{service}.txt')
        with open(f'{output}/config.{name}.json', 'w', encoding='utf8') as f:
            f.write(template.render(context))

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

@task
@parallel
def restart():
    configs = current()
    container_names = [f'fb_{x}' for config in configs['deploy'].values() for x in config]

    with settings(warn_only=True):
        sudo(f"docker restart {' '.join(container_names)}", quiet=True)