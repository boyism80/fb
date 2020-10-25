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

env.user = 'ubuntu'
env.password = 'tmdgus12!@'
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


@task
def deploy():
    optimizer.resources()

    execute(internal)
    execute(gateway)
    execute(login)
    execute(game)

    shutil.rmtree('table.deploy')

@parallel
@roles('internal')
def internal():
    global CONFIGURATION

    for name , config in current().items():
        run(f'mkdir -p {name}')

        with cd(name):
            context = { 'port': config['port'], 'database': CONFIGURATION['database'], 'game': json.dumps(CONFIGURATION['deploy']['game']) }
            files.upload_template(filename='config.internal.txt',
                                  destination=f'config.json',
                                  template_dir=f'{LOCAL_ROOT}/template',
                                  context=context, use_sudo=True, backup=False, use_jinja=True)

            put(f'internal/bin/internal', 'internal', use_sudo=True, mode='0755')
            run('mkdir -p table')
            with cd('table'):
                if os.path.isfile(f'table.deploy/host.json'):
                    put('table.deploy/host.json', 'host.json')
                else:
                    os.remove(f'table.deploy/host.json')


@parallel
@roles('gateway')
def gateway():
    global CONFIGURATION
    
    for name , config in current().items():
        run(f'mkdir -p {name}')

        with cd(name):
            template = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=f"{LOCAL_ROOT}/template")).get_template("config.gateway.txt")
            context = {
                        'name': name,
                        'host': config,
                        'internal': CONFIGURATION['deploy']['internal']['internal'],
                        'login': json.dumps({ CONFIGURATION['deploy']['login'][x]['name']: {'desc': CONFIGURATION['deploy']['login'][x]['desc'], 'ip': CONFIGURATION['deploy']['login'][x]['ip'], 'port': CONFIGURATION['deploy']['login'][x]['port']} for x in CONFIGURATION['deploy']['login']}, ensure_ascii=False)
                      }
            run(f"echo '{template.render(context)}' > config.json")

            put(f'gateway/bin/gateway', 'gateway', use_sudo=True, mode='0755')


@parallel
@roles('login')
def login():
    for name , config in current().items():
        run(f'mkdir -p {name}')

        with cd(name):
            template = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=f"{LOCAL_ROOT}/template")).get_template("config.login.txt")
            context = {
                        'name': name,
                        'host': config,
                        'internal': CONFIGURATION['deploy']['internal']['internal'],
                        'database': CONFIGURATION['database']
                      }
            run(f"echo '{template.render(context)}' > config.json")

            put(f'login/bin/login', 'login', use_sudo=True, mode='0755')




@parallel
@roles('game')
def game():
    for name , config in current().items():
        run(f'mkdir -p {name}')

        with cd(name):
            context = { 
                        'name': name, 
                        'host': config,
                        'internal': CONFIGURATION['deploy']['internal']['internal'],
                        'login': CONFIGURATION['deploy']['login']['login'],
                        'database': CONFIGURATION['database'],
                      }
            files.upload_template(filename='config.game.txt',
                                  destination=f'config.json',
                                  template_dir=f'{LOCAL_ROOT}/template',
                                  context=context, use_sudo=True, backup=False, use_jinja=True)

            put(f'game/bin/game', 'game', use_sudo=True, mode='0755')

            run('mkdir -p table')
            with cd('table'):
                sudo('rm -rf *')
                put(f'resources/table/*.json', '.')
                if os.path.isfile(f'table.deploy/mob.spawn.{name}.json'):
                    put(f'table.deploy/mob.spawn.{name}.json', 'mob.spawn.json')
                else:
                    sudo('rm -f mob.spawn.json')

                if os.path.isfile(f'table.deploy/npc.spawn.{name}.json'):
                    put(f'table.deploy/npc.spawn.{name}.json', 'npc.spawn.json')
                else:
                    sudo('rm -f npc.spawn.json')

                if os.path.isfile(f'table.deploy/warp.{name}.json'):
                    put(f'table.deploy/warp.{name}.json', 'warp.json')
                else:
                    sudo('rm -f warp.json')

            run('mkdir -p scripts')
            with cd('scripts'):
                sudo('rm -rf *')
                put(f'game/scripts/*', '.')

            run('mkdir -p maps')
            with cd('maps'):
                sudo('rm -rf *')
                if os.path.isfile(f'table.deploy/maps.{name}.zip'):
                    put(f'table.deploy/maps.{name}.zip', 'maps.zip')
                    sudo('unzip maps.zip')
                    sudo('rm -f maps.zip')




def current():
    global CONFIGURATION

    role = env.effective_roles[0]
    host = env.host
    result = { x : CONFIGURATION['deploy'][role][x] for x in CONFIGURATION['deploy'][role] if CONFIGURATION['deploy'][role][x]['ip'] == host }
    return result