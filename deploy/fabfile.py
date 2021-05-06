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

    for role in CONFIGURATION['deploy']:
        env['roledefs'][role] = {'hosts': []}

        for name, host in CONFIGURATION['deploy'][role].items():
            if host['ip'] not in env['roledefs'][role]['hosts']:
                env['roledefs'][role]['hosts'].append(host['ip'])

            if host['ip'] not in env['hosts']:
                env['hosts'].append(host['ip'])

    print(env['hosts'])

@task
def optimize():
    with lcd('resources/maps'):
        local('rm -f *.map')
        local('rm -f *.block')
        local('unzip -qq maps.zip')

    # for internal
    optimizer.hosts('host.json')

    # for game
    optimizer.compress(maps=os.path.join('resources', 'maps'),
                       tables=os.path.join('resources', 'table'),
                       scripts=os.path.join('game', 'scripts'),
                       dst='resources.zip')

@task
def cleanup():
    local('rm -f host.json')
    local('rm -f resources.zip')
    

@task
@parallel
@roles('internal')
def internal():
    global CONFIGURATION

    for name, config in current().items():
        run(f'mkdir -p {name}', quiet=True)

        with cd(name):
            context = { 'port': config['port'], 'database': CONFIGURATION['database'], 'game': json.dumps(CONFIGURATION['deploy']['game']) }
            files.upload_template(filename='config.internal.txt',
                                  destination=f'config.json',
                                  template_dir=f'{LOCAL_ROOT}/template',
                                  context=context, use_sudo=True, backup=False, use_jinja=True)

            put(f'/dist/fb/internal/internal', 'internal', use_sudo=True, mode='0755')
            put(f'internal/Dockerfile', '.', use_sudo=True)
            run('mkdir -p table', quiet=True)
            with cd('table'):
                put(f'host.json', '.', use_sudo=True)

            docker_build(name, config['port'])


@task
@parallel
@roles('gateway')
def gateway():
    global CONFIGURATION
    
    for name, config in current().items():
        run(f'mkdir -p {name}', quiet=True)

        with cd(name):
            template = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=f"{LOCAL_ROOT}/template")).get_template("config.gateway.txt")
            context = {
                        'name': name,
                        'host': config,
                        'internal': CONFIGURATION['deploy']['internal']['internal'],
                        'login': json.dumps({ CONFIGURATION['deploy']['login'][x]['name']: {'desc': CONFIGURATION['deploy']['login'][x]['desc'], 'ip': CONFIGURATION['deploy']['login'][x]['ip'], 'port': CONFIGURATION['deploy']['login'][x]['port']} for x in CONFIGURATION['deploy']['login']}, ensure_ascii=False)
                      }
            run(f"echo '{template.render(context)}' > config.json", quiet=True)

            put(f'/dist/fb/gateway/gateway', 'gateway', use_sudo=True, mode='0755')
            put(f'gateway/Dockerfile', '.', use_sudo=True)

            docker_build(name, config['port'])


@task
@parallel
@roles('login')
def login():
    for name, config in current().items():
        run(f'mkdir -p {name}', quiet=True)

        with cd(name):
            template = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=f"{LOCAL_ROOT}/template")).get_template("config.login.txt")
            context = {
                        'name': name,
                        'host': config,
                        'internal': CONFIGURATION['deploy']['internal']['internal'],
                        'database': CONFIGURATION['database']
                      }
            run(f"echo '{template.render(context)}' > config.json", quiet=True)

            put(f'/dist/fb/login/login', 'login', use_sudo=True, mode='0755')
            put(f'login/Dockerfile', '.', use_sudo=True)

            docker_build(name, config['port'])

@task
@parallel
@roles('game')
def game():
    root = 'game'

    sudo(f'rm -rf {root}')
    run(f'mkdir -p {root}', quiet=True)

    with cd(root):
        for name, config in current().items():
            context = { 
                    'name': name, 
                    'host': config,
                    'internal': CONFIGURATION['deploy']['internal']['internal'],
                    'login': CONFIGURATION['deploy']['login']['login'],
                    'database': CONFIGURATION['database'],
                  }
            files.upload_template(filename='config.game.txt',
                                  destination=f'config.{name}.json',
                                  template_dir=f'{LOCAL_ROOT}/template',
                                  context=context, use_sudo=True, backup=False, use_jinja=True)

        put(f'/dist/fb/game/game', 'game', use_sudo=True, mode='0755')
        put(f'game/Dockerfile', '.', use_sudo=True)

        put('resources.zip', '.', use_sudo=True)
        sudo('unzip -qq resources.zip')
        sudo('rm -f resources.zip')
        
        sudo(f"DOCKER_BUILDKIT=1 docker build --quiet --tag fb:{root} .", quiet=True)
        for name, config in current().items():
            container_name = f'fb_{name}'
            port = config['port']
            sudo(f"docker run -v `pwd`:/app --name {container_name} -d -it -e LC_ALL=C.UTF-8 -p {port}:{port} fb:{root} --env {name}", quiet=True)


def current():
    global CONFIGURATION

    role = env.effective_roles[0]
    host = env.host
    result = { x : CONFIGURATION['deploy'][role][x] for x in CONFIGURATION['deploy'][role] if CONFIGURATION['deploy'][role][x]['ip'] == host }
    return result

@task
@parallel
def docker_stop():
    with settings(warn_only=True):
        # sudo('docker rmi $(docker images --filter "dangling=true" -q --no-trunc)')
        sudo('docker ps --filter name=fb_* -aq | xargs docker stop | xargs docker rm', quiet=True)
        # sudo('docker images --filter=reference=fb:* -aq | xargs docker rmi', quiet=True)

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
def docker_prune():
    limit = 10

    with settings(warn_only=True):
        for i in range(limit):
            names = sudo("docker container ls -f 'status=running' --format '{{.Names}}'", quiet=True).replace('\r\n', '\n').split('\n')
            containers = [f'fb_{x}' for x in container_names(env.host)]

            if set(containers).issubset(names):
                break

            print(f'host waits while all docker container is running.. ({i + 1}/{10})')
            time.sleep(1)

        sudo('docker system prune -a -f', quiet=True)

def docker_build(name, port):
    container_name = f'fb_{name}'
    sudo(f"DOCKER_BUILDKIT=1 docker build --quiet --tag fb:{name} .", quiet=True)
    sudo(f"docker run -v `pwd`:/app --name {container_name} -d -it -e LC_ALL=C.UTF-8 -p {port}:{port} fb:{name}", quiet=True)