def lua_value(value):
    if isinstance(value, bool):
        return 'true' if value else 'false'
    elif isinstance(value, (int, float)):
        return str(value)
    elif isinstance(value, str):
        return '"' + value.replace('\\', '\\\\').replace('"', '\\"') + '"'
    elif isinstance(value, list):
        return '{ ' + ', '.join(lua_value(item) for item in value) + ' }'
    else:
        return '{ ' + ', '.join(f'{lua_key(key)} = {lua_value(item)}' for key, item in value.items()) + ' }'


def lua_key(key):
    if isinstance(key, str) and key.isascii() and key.isidentifier():
        return key
    return f'[{lua_value(key)}]'
