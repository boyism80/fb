fab -f deploy/fabfile.py environment:development generate_config_file:gateway,gateway/config
fab -f deploy/fabfile.py environment:development generate_config_file:login,login/config
fab -f deploy/fabfile.py environment:development generate_config_file:game,game/config
PAUSE