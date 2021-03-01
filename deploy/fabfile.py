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
import multiprocessing

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
    optimizer.resources()

@task
def cleanup():
    shutil.rmtree('table.deploy')
    

@task
@parallel
@roles('internal')
def internal():
    global CONFIGURATION

    for name, config in current().items():
        run(f'mkdir -p {name}', quiet=True)

        with cd(name):
            context = { 'port': config['port'], 'database': json.dumps(CONFIGURATION['database']), 'game': json.dumps(CONFIGURATION['deploy']['game']) }
            files.upload_template(filename='config.internal.txt',
                                  destination=f'config.json',
                                  template_dir=f'{LOCAL_ROOT}/template',
                                  context=context, use_sudo=True, backup=False, use_jinja=True)

            put(f'/dist/fb/internal/internal', 'internal', use_sudo=True, mode='0755')
            put(f'internal/Dockerfile', '.', use_sudo=True)
            run('mkdir -p table', quiet=True)
            with cd('table'):
                if os.path.isfile(f'table.deploy/host.json'):
                    put('table.deploy/host.json', 'host.json')
                else:
                    os.remove(f'table.deploy/host.json')

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
                        'database': json.dumps(CONFIGURATION['database'])
                      }
            run(f"echo '{template.render(context)}' > config.json", quiet=True)

            put(f'/dist/fb/login/login', 'login', use_sudo=True, mode='0755')
            put(f'login/Dockerfile', '.', use_sudo=True)

            docker_build(name, config['port'])


def build_game(name, config):
    run(f'mkdir -p {name}', quiet=True)

    with cd(name):
        context = { 
                    'name': name, 
                    'host': config,
                    'internal': CONFIGURATION['deploy']['internal']['internal'],
                    'login': CONFIGURATION['deploy']['login']['login'],
                    'database': json.dumps(CONFIGURATION['database']),
                  }
        files.upload_template(filename='config.game.txt',
                              destination=f'config.json',
                              template_dir=f'{LOCAL_ROOT}/template',
                              context=context, use_sudo=True, backup=False, use_jinja=True)

        put(f'/dist/fb/game/game', 'game', use_sudo=True, mode='0755')
        put(f'game/Dockerfile', '.', use_sudo=True)

        run('mkdir -p table', quiet=True)
        with cd('table'):
            sudo('rm -rf *', quiet=True)
            put(f'resources/table/*.json', '.')
            if os.path.isfile(f'table.deploy/mob.spawn.{name}.json'):
                put(f'table.deploy/mob.spawn.{name}.json', 'mob.spawn.json')
            else:
                sudo('rm -f mob.spawn.json', quiet=True)

            if os.path.isfile(f'table.deploy/npc.spawn.{name}.json'):
                put(f'table.deploy/npc.spawn.{name}.json', 'npc.spawn.json')
            else:
                sudo('rm -f npc.spawn.json', quiet=True)

            if os.path.isfile(f'table.deploy/warp.{name}.json'):
                put(f'table.deploy/warp.{name}.json', 'warp.json')
            else:
                sudo('rm -f warp.json', quiet=True)

            if os.path.isfile(f'table.deploy/world.json'):
                put(f'table.deploy/world.json', 'world.json')
            else:
                sudo('rm -f world.json', quiet=True)

        run('mkdir -p scripts', quiet=True)
        with cd('scripts'):
            sudo('rm -rf *', quiet=True)
            put(f'game/scripts/*', '.')

        run('mkdir -p maps', quiet=True)
        with cd('maps'):
            sudo('rm -rf *', quiet=True)
            if os.path.isfile(f'table.deploy/maps.{name}.zip'):
                put(f'table.deploy/maps.{name}.zip', 'maps.zip')
                sudo('unzip -qq maps.zip', quiet=True)
                sudo('rm -f maps.zip', quiet=True)

        docker_build(name, config['port'])


@task
@parallel
@roles('game')
def game():
    processes = []
    for name, config in current().items():
        p = multiprocessing.Process(target=build_game, kwargs={'name': name, 'config': config})
        p.start()
        processes.append(p)

    for p in processes:
        p.join()

def current():
    global CONFIGURATION

    role = env.effective_roles[0]
    host = env.host
    result = { x : CONFIGURATION['deploy'][role][x] for x in CONFIGURATION['deploy'][role] if CONFIGURATION['deploy'][role][x]['ip'] == host }
    return result

@task
@parallel
def docker_rm():
    with settings(warn_only=True):
        # sudo('docker rmi $(docker images --filter "dangling=true" -q --no-trunc)')
        sudo('docker system prune -a -f')
        sudo('docker ps --filter name=fb_* -aq | xargs docker stop | xargs docker rm', quiet=True)
        # sudo('docker images --filter=reference=fb:* -aq | xargs docker rmi', quiet=True)

def docker_build(name, port):
    container_name = f'fb_{name}'
    sudo(f"DOCKER_BUILDKIT=1 docker build --quiet --tag fb:{name} .", quiet=True)
    sudo(f"docker run --name {container_name} -d -it -p {port}:{port} fb:{name}", quiet=True)