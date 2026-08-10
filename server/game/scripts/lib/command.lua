local sky_maze = require('lib.sky_maze')

local M = {}

M.functions = {
    ['명령어'] = {
        ['privilege'] = ROLE.USER,
        ['usage'] = '- 사용 가능한 명령어 목록 표시',
        ['command'] = function (me, args)
            local user_role = me:role()
            local available_commands = {}
            
            for cmd_name, cmd_data in pairs(M.functions) do
                if cmd_name ~= '명령어' then
                    local privilege = nil
                    local usage = nil
                    
                    if type(cmd_data) == 'table' then
                        privilege = cmd_data['privilege'] or ROLE.ADMIN
                        usage = cmd_data['usage'] or ''
                    else
                        privilege = ROLE.ADMIN
                        usage = '- (설명 없음)'
                    end
                    
                    if user_role >= privilege then
                        table.insert(available_commands, {
                            name = cmd_name,
                            usage = usage,
                            privilege = privilege
                        })
                    end
                end
            end
            
            table.sort(available_commands, function(a, b)
                return a.name < b.name
            end)
            
            me:message("=== 사용 가능한 명령어 목록 ===", MESSAGE_TYPE.BROWN)
            for i, cmd in ipairs(available_commands) do
                local role_name = ''
                if cmd.privilege == ROLE.OWNER then
                    role_name = '[OWNER]'
                elseif cmd.privilege == ROLE.SUPERADMIN then
                    role_name = '[SUPERADMIN]'
                elseif cmd.privilege == ROLE.ADMIN then
                    role_name = '[ADMIN]'
                elseif cmd.privilege == ROLE.MODERATOR then
                    role_name = '[MODERATOR]'
                end
                
                local line = string.format("%d. /%s %s", i, cmd.name, cmd.usage)
                if role_name ~= '' then
                    line = line .. ' ' .. role_name
                end
                me:message(line, MESSAGE_TYPE.BROWN)
            end
            me:message(string.format("총 %d개의 명령어가 있습니다.", #available_commands), MESSAGE_TYPE.BROWN)
            
            return true
        end,
    },
    
    ['관리자'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '- 관리자 권한 부여',
        ['command'] = function (me, args)
            me:mkspell('강제이동(좌)')
            me:mkspell('강제이동(하)')
            me:mkspell('강제이동(우)')
            me:mkspell('잠복근무')
            me:mkspell('강제이동(상)')
            return true
        end,
    },
    
    ['권한변경'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<유저이름> <직책> - 권한 변경',
        ['command'] = function (me, args)
            local name, role = table.unpack(args)
            if not name or not role then
                me:message("사용법: /권한변경 <유저이름> <직책>")
                return true
            end
            
            role = tonumber(role)
            if not role or role < 0 then
                me:message("직책은 0 이상의 숫자여야 합니다.")
                return true
            end
            
            local you = name2ch(name)
            if you == nil then
                me:message(string.format("%s : 접속중이 아닙니다.", name))
                return true
            end
            
            if you:role() >= me:role() then
                me:message(string.format("권한이 없습니다.", name))
                return true
            end
            
            if role >= me:role() then
                me:message(string.format("권한이 없습니다.", name))
                return true
            end
            
            you:role(role)
            me:message(string.format("%s : 직책을 %d로 변경했습니다.", name, role))
            return true
        end,
    },
    
    ['경험치'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<경험치> - 경험치 설정',
        ['command'] = function (me, args)
            local exp = table.unpack(args)
            if not exp then
                me:message("사용법: /경험치 <경험치>")
                return true
            end
            exp = tonumber(exp)
            if not exp or exp < 0 then
                me:message("경험치는 0 이상의 숫자여야 합니다.")
                return true
            end
            me:exp(exp)
            return true
        end,
    },
    
    ['맵이동'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<맵이름> [x] [y] [slot] - 맵 이동',
        ['command'] = function (me, args)
            local name, x, y, slot = table.unpack(args)
            if not name then
                me:message("사용법: /맵이동 <맵이름> [x] [y] [slot]")
                return true
            end

            local model = name2map(name)
            if model == nil then
                me:message(string.format("존재하지 않는 맵입니다: %s", name))
                return true
            end

            if slot ~= nil then
                slot = tonumber(slot)
                if not slot or slot < 1 then
                    me:message("슬롯은 1 이상의 숫자여야 합니다.")
                    return true
                end
            end

            if x ~= nil and y ~= nil then
                x = tonumber(x)
                y = tonumber(y)
                if not x or not y then
                    me:message("좌표는 숫자여야 합니다.")
                    return true
                end
            end

            local map = model
            if slot ~= nil then
                map = model:instance(slot)
                if map == nil then
                    me:message(string.format("인스턴스 맵을 생성할 수 없습니다: %s (slot %d)", name, slot))
                    return true
                end
            end

            -- Without slot always land on S; never CAPACITY/GROUP auto-route to an unknown C.
            local opts = { skip_instance_rule = true }
            if x ~= nil and y ~= nil then
                me:map(map, x, y, opts)
            else
                me:map(map, opts)
            end
            return true
        end,
    },
    
    ['사운드'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<사운드ID> - 사운드 재생',
        ['command'] = function (me, args)
            local sound = table.unpack(args)
            if not sound then
                me:message("사용법: /사운드 <사운드ID>")
                return true
            end
            sound = tonumber(sound)
            if not sound or sound < 0 then
                me:message("사운드 ID는 0 이상의 숫자여야 합니다.")
                return true
            end
            me:sound(sound)
            return true
        end,
    },
    
    ['액션'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<액션ID> - 액션 실행',
        ['command'] = function (me, args)
            local action = table.unpack(args)
            if not action then
                me:message("사용법: /액션 <액션ID>")
                return true
            end
            action = tonumber(action)
            if not action or action < 0 then
                me:message("액션 ID는 0 이상의 숫자여야 합니다.")
                return true
            end
            me:action(action)
            return true
        end,
    },
    
    ['날씨'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<날씨값> - 날씨 설정',
        ['command'] = function (me, args)
            local value = table.unpack(args)
            if not value then
                me:message("사용법: /날씨 <날씨값>")
                return true
            end
            value = tonumber(value)
            if not value or value < 0 then
                me:message("날씨값은 0 이상의 숫자여야 합니다.")
                return true
            end
            weather(value)
            return true
        end,
    },
    
    ['밝기'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<밝기값> - 밝기 설정',
        ['command'] = function (me, args)
            local value = table.unpack(args)
            if not value then
                me:message("사용법: /밝기 <밝기값>")
                return true
            end
            value = tonumber(value)
            if not value or value < 0 then
                me:message("밝기값은 0 이상의 숫자여야 합니다.")
                return true
            end
            bright(value)
            return true
        end,
    },
    
    ['타이머'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<시간(초)> [증가|감소] - 타이머 설정 (기본: 감소)',
        ['command'] = function (me, args)
            local time_arg = args[1]
            local mode_arg = args[2]
            if not time_arg then
                me:message("사용법: /타이머 <시간(초)> [증가|감소]")
                return true
            end
            local time = tonumber(time_arg)
            if not time or time < 0 then
                me:message("시간은 0보다 큰 숫자여야 합니다.")
                return true
            end

            local decrease = true
            if mode_arg then
                if mode_arg == '증가' then
                    decrease = false
                elseif mode_arg == '감소' then
                    decrease = true
                else
                    me:message("사용법: /타이머 <시간(초)> [증가|감소]")
                    return true
                end
            end

            me:timer(time, decrease)
            return true
        end,
    },

    ['스크립트타이머'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '시작|중지 [맵ID] [밀리초] - map:set_timer 스모크 (기본: 현재맵, 1000ms)',
        ['command'] = function (me, args)
            local action = args[1]
            if action ~= '시작' and action ~= '중지' then
                me:message("사용법: /스크립트타이머 시작|중지 [맵ID] [밀리초]")
                return true
            end

            local function resolve_game_map(map_id)
                for _, map in pairs(maps()) do
                    if map:model():id() == map_id then
                        return map
                    end
                end
                return nil
            end

            local map_id = tonumber(args[2])
            local map = nil
            if map_id ~= nil then
                map = resolve_game_map(map_id)
            else
                map = me:map()
                if map ~= nil then
                    map_id = map:model():id()
                end
            end
            if map == nil then
                me:message("맵을 찾을 수 없습니다.")
                return true
            end

            local NAME = 'script_timer_smoke'
            local PATH = 'scripts/lib/script_timer_smoke.lua'

            if action == '중지' then
                local ok = map:cancel_timer(NAME)
                if ok then
                    me:message(string.format("스크립트 타이머 중지: map=%d name=%s", map_id, NAME))
                else
                    me:message("중지할 스크립트 타이머가 없습니다.")
                end
                return true
            end

            local interval = tonumber(args[3]) or 1000
            if interval <= 0 then
                me:message("밀리초는 1 이상이어야 합니다.")
                return true
            end

            local id = map:set_timer(interval, PATH, 'on_tick', { name = NAME })
            if id == nil then
                me:message("스크립트 타이머 설치 실패")
                return true
            end

            me:message(string.format("스크립트 타이머 시작: map=%d interval=%dms id=%s", map_id, interval, tostring(id)))
            return true
        end,
    },

    ['현재시간'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '[YYYY-MM-DD HH:MM:SS [음력]] - 현재 서버 시간 조회/설정',
        ['command'] = function (me, args)
            if #args == 0 then
                local dt = datetime()
                if dt.lunar_year ~= nil then
                    local leap = dt.lunar_leap and ' (윤)' or ''
                    me:message(string.format(
                        "현재 서버 시간: %04d-%02d-%02d %02d:%02d:%02d / 음력 %04d-%02d-%02d%s",
                        dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second,
                        dt.lunar_year, dt.lunar_month, dt.lunar_day, leap), MESSAGE_TYPE.BROWN)
                else
                    me:message(string.format("현재 서버 시간: %04d-%02d-%02d %02d:%02d:%02d",
                        dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second), MESSAGE_TYPE.BROWN)
                end
                return true
            end

            local lunar = args[#args] == '음력'
            local date_args = args
            if lunar then
                date_args = { table.unpack(args, 1, #args - 1) }
            end

            local value = table.concat(date_args, ' ')
            if not string.match(value, '^%d%d%d%d%-%d%d%-%d%d %d%d:%d%d:%d%d$') then
                me:message("사용법: /현재시간 YYYY-MM-DD HH:MM:SS [음력]")
                return true
            end

            if lunar then
                local y, m, d, hh, mm, ss = string.match(value, '^(%d%d%d%d)%-(%d%d)%-(%d%d) (%d%d):(%d%d):(%d%d)$')
                local solar, err = from_lunar({
                    year = tonumber(y),
                    month = tonumber(m),
                    day = tonumber(d),
                    leap = false,
                }, tonumber(hh), tonumber(mm), tonumber(ss))
                if solar == nil then
                    me:message(string.format("음력 변환 실패: %s", err or "unknown error"), MESSAGE_TYPE.BROWN)
                    return true
                end
                value = string.format("%04d-%02d-%02d %02d:%02d:%02d",
                    solar.year, solar.month, solar.day, solar.hour, solar.minute, solar.second)
            end

            local success, error_message = now(value)
            if success then
                if lunar then
                    me:message(string.format("현재 시간을 음력 기준 양력 %s 로 설정 요청했습니다.", value), MESSAGE_TYPE.BROWN)
                else
                    me:message(string.format("현재 시간을 %s 로 설정 요청했습니다.", value), MESSAGE_TYPE.BROWN)
                end
            else
                me:message(string.format("현재시간 설정 실패: %s", error_message or "unknown error"), MESSAGE_TYPE.BROWN)
            end
            return true
        end,
    },

    ['현재시간초기화'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '- 현재 서버 시간 보정 초기화',
        ['command'] = function (me, args)
            local success, error_message = now("reset")
            if success then
                me:message("현재 시간 보정 초기화 요청을 전송했습니다.", MESSAGE_TYPE.BROWN)
            else
                me:message(string.format("현재시간 보정 초기화 실패: %s", error_message or "unknown error"), MESSAGE_TYPE.BROWN)
            end
            return true
        end,
    },

    ['시간가속'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<timespan> - 시간 앞으로 이동 (예: 10.12:30:00 / 12:30:00)',
        ['command'] = function (me, args)
            local value = table.unpack(args)
            if not value then
                me:message("사용법: /시간가속 <timespan>")
                return true
            end

            local success, error_message = time_forward(value)
            if success then
                me:message(string.format("시간가속 적용 요청: %s", value), MESSAGE_TYPE.BROWN)
            else
                me:message(string.format("시간가속 실패: %s", error_message or "unknown error"), MESSAGE_TYPE.BROWN)
            end
            return true
        end,
    },

    ['시간역전'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<timespan> - 시간 뒤로 이동 (예: 10.12:30:00 / 12:30:00)',
        ['command'] = function (me, args)
            local value = table.unpack(args)
            if not value then
                me:message("사용법: /시간역전 <timespan>")
                return true
            end

            local success, error_message = time_backward(value)
            if success then
                me:message(string.format("시간역전 적용 요청: %s", value), MESSAGE_TYPE.BROWN)
            else
                me:message(string.format("시간역전 실패: %s", error_message or "unknown error"), MESSAGE_TYPE.BROWN)
            end
            return true
        end,
    },
    
    ['타이틀'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<타이틀> - 타이틀 설정',
        ['command'] = function (me, args)
            local value = table.unpack(args)
            if not value then
                me:message("사용법: /타이틀 <타이틀>")
                return true
            end
            me:title(value)
            return true
        end,
    },
    
    ['이펙트'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<이펙트ID> - 이펙트 실행',
        ['command'] = function (me, args)
            local value = table.unpack(args)
            if not value then
                me:message("사용법: /이펙트 <이펙트ID>")
                return true
            end
            value = tonumber(value)
            if not value or value < 0 then
                me:message("이펙트 ID는 0 이상의 숫자여야 합니다.")
                return true
            end
            me:effect(value)
            return true
        end,
    },
    
    ['변신'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '<변신ID> - 변신 (mob.json raw look, +0x7FFF 보정)',
        ['command'] = function (me, args)
            local value = table.unpack(args)
            if not value then
                me:message("사용법: /변신 <변신ID>")
                return true
            end
            value = tonumber(value)
            if not value or value < 0 then
                me:message("변신 ID는 0 이상의 숫자여야 합니다.")
                return true
            end
            -- mob/npc table look = json raw + 0x7FFF (server.init build hook)
            local MOB_LOOK_OFFSET = 0x7FFF
            local wire = value
            if value < MOB_LOOK_OFFSET then
                wire = value + MOB_LOOK_OFFSET
            end
            me:mimic({ disguise = wire })
            me:message(string.format("변신 raw=%d wire=%d", value, wire))
            return true
        end,
    },
    
    ['랜덤의태'] = {
        ['privilege'] = ROLE.ADMIN,
        ['usage'] = '- 랜덤 의태',
        ['command'] = function (me, args)
            me:mimic({ 
                hair = math.random(1, 102),
                hair_color = math.random(1, 31),
                gender = math.random(1, 2),
                state = math.random(0, 2),
                weapon = math.random(1, 231),
                armor = math.random(1, 231),
                shield = math.random(1, 231),
                disguise = nil })
                return true
            end,
        },
        
        ['변신해제'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 변신 해제',
            ['command'] = function (me, args)
                me:mimic(nil)
                return true
            end,
        },
        
        ['마법배우기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<마법이름> - 마법 학습',
            ['command'] = function (me, args)
                local name = table.unpack(args)
                if not name then
                    me:message("사용법: /마법배우기 <마법이름>")
                    return true
                end
                me:mkspell(name)
                return true
            end,
        },
        
        ['마법지우기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[슬롯] - 마법 삭제',
            ['command'] = function (me, args)
                if #args == 0 then
                    me:rmspell()
                else
                    local slot = table.unpack(args)
                    slot = tonumber(slot)
                    if not slot or slot < 0 then
                        me:message("슬롯 번호는 0 이상의 숫자여야 합니다.")
                        return true
                    end
                    me:rmspell(slot)
                end
                return true
            end,
        },
        
        ['몬스터생성'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<몬스터이름> [x] [y] - 몬스터 생성',
            ['command'] = function (me, args)
                local name, x, y = table.unpack(args)
                if not name then
                    me:message("사용법: /몬스터생성 <몬스터이름> [x] [y]")
                    return true
                end
                
                if x == nil and y == nil then
                    x, y = me:position()
                else
                    x = tonumber(x)
                    y = tonumber(y)
                    if not x or not y then
                        me:message("좌표는 숫자여야 합니다.")
                        return true
                    end
                end
                me:spawn_mob(name, x, y, false)
                return true
            end,
        },
        
        ['몬스터범위생성'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<몬스터이름> <거리> - 범위 내 몬스터 생성',
            ['command'] = function (me, args)
                local name, distance = table.unpack(args)

                if not name or not distance then
                    me:message("사용법: /몬스터범위생성 <몬스터이름> <거리>")
                    return true
                end
                
                distance = tonumber(distance)
                if not distance or distance < 0 then
                    me:message("거리는 0 이상의 숫자여야 합니다.")
                    return true
                end

                local player_x, player_y = me:position()
                local spawned_count = 0
                local oids = {}

                local distance_squared = distance * distance
                for dx = -distance, distance do
                    for dy = -distance, distance do
                        if dx ~= 0 or dy ~= 0 then
                            local distance_sq = dx * dx + dy * dy

                            if distance_sq <= distance_squared then
                                local target_x = player_x + dx
                                local target_y = player_y + dy

                                if target_x >= 0 and target_y >= 0 and target_x < 1000 and target_y < 1000 then
                                    local mob = me:spawn_mob(name, target_x, target_y, false, false)
                                    if mob then
                                        spawned_count = spawned_count + 1
                                        table.insert(oids, mob:oid())
                                    end
                                end
                            end
                        end
                    end
                end

                if spawned_count > 0 then
                    local map = me:map()
                    map:bulk_update(oids)
                    me:message(string.format("'%s' 몬스터를 %d마리 생성했습니다. (거리: %d)", name, spawned_count, distance))
                else
                    me:message(string.format("'%s' 몬스터 생성에 실패했습니다.", name))
                end
                
                return true
            end,
        },
        
        ['몬스터제거'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 맵 내 모든 몬스터 제거',
            ['command'] = function (me, args)
                local map = me:map()
                if map == nil then
                    me:message("맵에 있지 않습니다.")
                    return true
                end
                
                local objects = map:objects(OBJECT_TYPE.MOB)
                for _, object in ipairs(objects) do
                    if object:body() == nil then
                        object:destroy()
                    end
                end
                
                return true
            end,
        },

        ['몬스터범위제거'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<거리> - 시전자 주변 범위 내 몬스터 제거',
            ['command'] = function (me, args)
                local distance = tonumber(table.unpack(args))
                if distance == nil or distance < 0 then
                    me:message("사용법: /몬스터범위제거 <거리>")
                    return true
                end

                local map = me:map()
                if map == nil then
                    me:message("맵에 있지 않습니다.")
                    return true
                end

                local player_x, player_y = me:position()
                local distance_squared = distance * distance
                local removed_count = 0

                local objects = map:objects(OBJECT_TYPE.MOB)
                for _, object in ipairs(objects) do
                    if object:body() ~= nil then
                        goto continue
                    end

                    local mob_x, mob_y = object:position()
                    local dx = mob_x - player_x
                    local dy = mob_y - player_y
                    if dx * dx + dy * dy <= distance_squared then
                        object:destroy()
                        removed_count = removed_count + 1
                    end
                    ::continue::
                end

                if removed_count > 0 then
                    me:message(string.format(
                        "주변 %d칸 내 몬스터 %d마리를 제거했습니다.",
                        distance, removed_count))
                end

                return true
            end,
        },

        ['아이템범위제거'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<거리> - 시전자 주변 범위 내 드롭 아이템 제거',
            ['command'] = function (me, args)
                local distance = tonumber(table.unpack(args))
                if distance == nil or distance < 0 then
                    me:message("사용법: /아이템범위제거 <거리>")
                    return true
                end

                local map = me:map()
                if map == nil then
                    me:message("맵에 있지 않습니다.")
                    return true
                end

                local player_x, player_y = me:position()
                local distance_squared = distance * distance
                local removed_count = 0

                local objects = map:objects(OBJECT_TYPE.ITEM)
                for _, object in ipairs(objects) do
                    local item_x, item_y = object:position()
                    local dx = item_x - player_x
                    local dy = item_y - player_y
                    if dx * dx + dy * dy <= distance_squared then
                        object:destroy()
                        removed_count = removed_count + 1
                    end
                end

                if removed_count > 0 then
                    me:message(string.format(
                        "주변 %d칸 내 아이템 %d개를 제거했습니다.",
                        distance, removed_count))
                end

                return true
            end,
        },
        
        ['직업바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<직업이름> - 직업 변경',
            ['command'] = function (me, args)
                local name = table.unpack(args)
                if not name then
                    me:message("사용법: /직업바꾸기 <직업이름>")
                    return true
                end
                local class, promotion = name2class(name)
                if class ~= nil then
                    me:class(class)
                    me:promotion(promotion)
                else
                    me:message(string.format("존재하지 않는 직업입니다: %s", name))
                    return true
                end
                return true
            end,
        },
        
        ['레벨바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<레벨> - 레벨 설정 (0~255)',
            ['command'] = function (me, args)
                local level = table.unpack(args)
                if not level then
                    me:message("사용법: /레벨바꾸기 <레벨> (0~255)")
                    return true
                end
                level = tonumber(level)
                if not level or level < 0 or level > 255 then
                    me:message("레벨은 0~255 사이의 숫자여야 합니다.")
                    return true
                end
                me:level(level)
                return true
            end,
        },
        
        ['힘바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<힘값> - 힘 설정',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /힘바꾸기 <힘값>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("힘값은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:base_str(value)
                return true
            end,
        },
        
        ['민첩바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<민첩값> - 민첩 설정',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /민첩바꾸기 <민첩값>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("민첩값은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:base_dex(value)
                return true
            end,
        },
        
        ['지력바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<지력값> - 지력 설정',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /지력바꾸기 <지력값>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("지력값은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:base_int(value)
                return true
            end,
        },
        
        ['스탯바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<힘> <민첩> <지력> - 스탯 설정',
            ['command'] = function (me, args)
                local str, dex, int = table.unpack(args)
                if not str or not dex or not int then
                    me:message("사용법: /스탯바꾸기 <힘> <민첩> <지력>")
                    return true
                end
                str = tonumber(str)
                dex = tonumber(dex)
                int = tonumber(int)
                if not str or not dex or not int or str < 0 or dex < 0 or int < 0 then
                    me:message("힘, 민첩, 지력은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:base_str(str)
                me:base_dex(dex)
                me:base_int(int)
                return true
            end,
        },
        
        ['체력바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<체력값> - 체력 설정',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /체력바꾸기 <체력값>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("체력값은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:base_hp(value)
                me:hp(me:maxhp())
                if me:state() == STATE.GHOST then
                    me:state(STATE.NORMAL)
                end
                return true
            end,
        },
        
        ['마력바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<마력값> - 마력 설정',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /마력바꾸기 <마력값>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("마력값은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:base_mp(value)
                me:mp(me:maxmp())
                return true
            end,
        },
        
        ['현재체력'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<체력값> - 현재 체력 설정',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /현재체력 <체력값>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("체력값은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:hp(value)
                return true
            end,
        },
        
        ['현재마력'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<마력값> - 현재 마력 설정',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /현재마력 <마력값>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("마력값은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:mp(value)
                return true
            end,
        },
        
        ['아이템생성'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<아이템이름> [개수] [유효시간(초)] - 아이템 생성',
            ['command'] = function (me, args)
                local name = args[1]
                if not name then
                    me:message("사용법: /아이템생성 <아이템이름> [개수] [유효시간(초)]")
                    return true
                end

                local count = 1
                if args[2] ~= nil then
                    count = tonumber(args[2])
                    if not count or count < 1 then
                        me:message("개수는 1 이상의 숫자여야 합니다.")
                        return true
                    end
                end

                local expire_sec = nil
                if args[3] ~= nil then
                    expire_sec = tonumber(args[3])
                    if expire_sec == nil or expire_sec < 1 then
                        me:message("유효시간은 1 이상의 숫자(초)여야 합니다.")
                        return true
                    end
                end

                me:mkitem(name, count, true, expire_sec)
                return true
            end,
        },

        ['룩미리보기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<look> [개수] [색] - 빈 슬롯에 가짜 인벤 아이콘 미리보기 (item.json raw look)',
            ['command'] = function (me, args)
                local look = tonumber(args[1])
                if look == nil or look < 0 then
                    me:message("사용법: /룩미리보기 <look> [개수] [색]")
                    return true
                end

                local count = 1
                if args[2] ~= nil then
                    count = tonumber(args[2])
                    if not count or count < 1 then
                        me:message("개수는 1 이상의 숫자여야 합니다.")
                        return true
                    end
                end

                local color = 0
                if args[3] ~= nil then
                    color = tonumber(args[3])
                    if color == nil or color < 0 then
                        me:message("색은 0 이상의 숫자여야 합니다.")
                        return true
                    end
                end

                local ITEM_LOOK_OFFSET = 0xBFFF
                local sent = me:preview_item_looks(look, count, color)
                if sent == nil or sent <= 0 then
                    me:message("빈 인벤 슬롯이 없습니다.")
                    return true
                end

                local raw_end = look + sent - 1
                local function to_wire(raw)
                    if raw >= ITEM_LOOK_OFFSET then
                        return raw
                    end
                    return raw + ITEM_LOOK_OFFSET
                end
                me:message(string.format(
                    "룩미리보기 %d개 (시각만, 실제 아이템 아님) raw=%d..%d wire=%d..%d",
                    sent, look, raw_end, to_wire(look), to_wire(raw_end)))
                return true
            end,
        },
        
        ['월드맵'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<월드맵이름> - 월드맵 설정',
            ['command'] = function (me, args)
                local name = table.unpack(args)
                if not name then
                    me:message("사용법: /월드맵 <월드맵이름>")
                    return true
                end
                me:world(name)
                return true
            end,
        },
        
        ['스크립트'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[파일] [함수] [인자...] - Lua 스크립트 함수 실행 (기본: scripts/script.lua func)',
            ['command'] = function (me, args)
                local script_file = args[1] or 'scripts/script.lua'
                local func_name = args[2] or 'func'
                if not script_file:match('%.lua$') then
                    script_file = script_file .. '.lua'
                end
                me:script(script_file, func_name, table.unpack(args, 3))
                return true
            end,
        },
        
        ['천상미궁셔플'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[seed] - 천상미궁/PK천상미궁 워프 셔플 (seed 생략 시 now() 사용)',
            ['command'] = function (me, args)
                local seed = args[1]
                if seed ~= nil then
                    seed = tonumber(seed)
                end
                sky_maze.shuffle(seed)
                sky_maze.pk_shuffle(seed)
                local msg = "천상미궁 / PK천상미궁 워프 셔플 적용되었습니다."
                if seed ~= nil then
                    msg = msg .. " (seed=" .. tostring(seed) .. ")"
                end
                me:message(msg, MESSAGE_TYPE.NOTIFY)
                return true
            end,
        },
        
        ['천상미궁루트'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 현재 셔플 기준 천상미궁시작~천상미궁비밀방 루트를 메시지로 출력',
            ['command'] = function (me, args)
                sky_maze.print_route(me)
                return true
            end,
        },
        
        ['머리바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<머리ID> - 머리 변경',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /머리바꾸기 <머리ID>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("머리 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:look(value)
                return true
            end,
        },
        
        ['머리염색'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<색상ID> - 머리 염색',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /머리염색 <색상ID>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("색상 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:color(value)
                return true
            end,
        },
        
        ['갑옷염색'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<색상ID> - 갑옷 염색',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /갑옷염색 <색상ID>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("색상 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:armor_color(value)
                return true
            end,
        },

        ['무기염색'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<색상ID> - 무기 염색',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /무기염색 <색상ID>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("색상 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:weapon_color(value)
                return true
            end,
        },

        ['방패염색'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<색상ID> - 방패 염색',
            ['command'] = function (me, args)
                local value = table.unpack(args)
                if not value then
                    me:message("사용법: /방패염색 <색상ID>")
                    return true
                end
                value = tonumber(value)
                if not value or value < 0 then
                    me:message("색상 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:shield_color(value)
                return true
            end,
        },
        
        ['맵타일'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[타일ID] - 맵 타일 정보/설정',
            ['command'] = function (me, args)
                local map = me:map()
                if map == nil then
                    me:message("맵에 있지 않습니다.")
                    return true
                end
                
                local x, y = me:position()
                if #args == 0 then
                    local id, obj, blocked = map:tile(x, y)
                    if id == nil then
                        me:message("타일 정보를 가져올 수 없습니다.")
                        return true
                    end
                    
                    if blocked then
                        blocked = 'true'
                    else
                        blocked = 'false'
                    end
                    me:message(string.format('타일 : %d\n오브젝트 : %d\n블록 : %s', id, obj, blocked), MESSAGE_TYPE.POPUP)
                else
                    local value = table.unpack(args)
                    if not value then
                        me:message("사용법: /맵타일 [타일ID]")
                        return true
                    end
                    value = tonumber(value)
                    if not value or value < 0 then
                        me:message("타일 ID는 0 이상의 숫자여야 합니다.")
                        return true
                    end
                    map:tile(x, y, value)
                end
                return true
            end,
        },
        
        ['서버저장'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 서버 저장',
            ['command'] = function (me, args)
                save()
                return true
            end,
        },
        
        ['랜덤이동'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 랜덤 맵 이동',
            ['command'] = function (me, args)
                local maps = maps()
                if not maps or #maps == 0 then
                    me:message("이동할 수 있는 맵이 없습니다.")
                    return true
                end
                local map = maps[math.random(1, #maps)]
                local x = math.random(0, map:width())
                local y = math.random(0, map:height())
                me:map(map, x, y)
                return true
            end,
        },
        
        ['엔피씨생성'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<NPC이름> [맵이름] [x] [y] - NPC 생성',
            ['command'] = function (me, args)
                local name, map_name, x, y = table.unpack(args)
                if not name then
                    me:message("사용법: /엔피씨생성 <NPC이름> [맵이름] [x] [y]")
                    return true
                end
                
                local map = nil
                if map_name == nil then
                    map = me:map()
                else
                    map = name2map(map_name)
                    if not map then
                        me:message(string.format("존재하지 않는 맵입니다: %s", map_name))
                        return true
                    end
                end
                
                if x == nil or y == nil then
                    x, y = me:position()
                else
                    x = tonumber(x)
                    y = tonumber(y)
                    if not x or not y then
                        me:message("좌표는 숫자여야 합니다.")
                        return true
                    end
                end
                mknpc(name, map, x, y)
                return true
            end,
        },
        
        ['엔피씨제거'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- NPC 제거',
            ['command'] = function (me, args)
                local front = me:front(OBJECT_TYPE.NPC)
                if front == nil then
                    return true
                end
                
                front:destroy()
                return true
            end,
        },
        
        ['내구도'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<퍼센트> - 장비 내구도 설정',
            ['command'] = function (me, args)
                local percent = table.unpack(args)
                if not percent then
                    me:message("사용법: /내구도 <퍼센트>")
                    return true
                end
                percent = tonumber(percent)
                if not percent or percent < 0 or percent > 100 then
                    me:message("퍼센트는 0~100 사이의 숫자여야 합니다.")
                    return true
                end
                percent = math.max(0, math.min(100, percent))
                for parts, equipment in pairs(me:equipments()) do
                    local model = equipment:model()
                    equipment:durability(model:durability() * (percent / 100.0))
                end
                
                for _, item in pairs(me:items()) do
                    local model = item:model()
                    if model:attr(ITEM_ATTRIBUTE.EQUIPMENT) then
                        item:durability(model:durability() * (percent / 100.0))
                    end
                end
                return true
            end,
        },
        
        ['sleep'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<시간(ms)> - 대기',
            ['command'] = function (me, args)
                local time = table.unpack(args)
                if not time then
                    me:message("사용법: /sleep <시간(ms)>")
                    return true
                end
                time = tonumber(time)
                if not time or time < 0 then
                    me:message("시간은 0 이상의 숫자여야 합니다.")
                    return true
                end
                sleep(time)
                me:message('done')
                return true
            end,
        },
        
        ['광고'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<너비> <높이> <URL> [시간] - 광고 표시',
            ['command'] = function (me, args)
                local width, height, url, time = table.unpack(args)
                if not width or not height or not url then
                    me:message("사용법: /광고 <너비> <높이> <URL> [시간]")
                    return true
                end
                
                width = tonumber(width)
                height = tonumber(height)
                if not width or not height or width <= 0 or height <= 0 then
                    me:message("너비와 높이는 0보다 큰 숫자여야 합니다.")
                    return true
                end
                
                if time then
                    time = tonumber(time)
                    if not time or time <= 0 then
                        me:message("시간은 0보다 큰 숫자여야 합니다.")
                        return true
                    end
                end
                
                me:ad(width, height, url, time)
                return true
            end,
        },
        
        ['웹'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<타입> <URL> - 웹 페이지 열기',
            ['command'] = function (me, args)
                local type, url, message = table.unpack(args)
                if not type or not url then
                    me:message("사용법: /웹 <타입> <URL>")
                    return true
                end
                
                type = tonumber(type)
                if not type or type < 0 then
                    me:message("타입은 0 이상의 숫자여야 합니다.")
                    return true
                end
                
                me:web(type, url)
                return true
            end,
        },
        
        ['unknown_12'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[oid] [slot] [flag] - S2C 0x12 (slot=인벤문자 1=a, flag<0xA0→[무장] / >=0xA0→레벨업토스트). 생략 시 자신 oid·slot0·flag1',
            ['command'] = function (me, args)
                local oid, slot, flag = table.unpack(args)
                if oid or slot or flag then
                    me:unknown_12(tonumber(oid) or 0, tonumber(slot) or 0, tonumber(flag) or 1)
                else
                    me:unknown_12()
                end
                return true
            end,
        },

        ['move_confirm_noscroll'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[direction] [x] [y] [vx] [vy] [walk_slot] - 이동 ACK(노스크롤, 0x26). 생략 시 현재 위치',
            ['command'] = function (me, args)
                local dir, x, y, vx, vy, slot = table.unpack(args)
                if dir or x or y or vx or vy or slot then
                    me:move_confirm_noscroll(tonumber(dir) or 0, tonumber(x) or 0, tonumber(y) or 0,
                        tonumber(vx) or 0, tonumber(vy) or 0, tonumber(slot) or 0)
                else
                    me:move_confirm_noscroll()
                end
                return true
            end,
        },

        ['UI화면'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[0=인벤토리 2=스킬 4=상태 6=게시판 7=세부상태 8=업적] - UI 화면 전환 (cmd 0x3E)',
            ['command'] = function (me, args)
                local screen = tonumber(table.unpack(args)) or 0
                me:ui(screen)
                return true
            end,
        },
        
        ['아이템던지기확인'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[inventory_slot] - item throw confirm (cmd 0x4E), client shows "정말 던지겠습니까?"',
            ['command'] = function (me, args)
                local slot = tonumber(table.unpack(args)) or 0
                me:item_throw_confirm(slot)
                return true
            end,
        },
        
        ['친구목록동기화'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[0|1] - send friends_sync (0x6A): 0=disable sync, non-zero=enable sync',
            ['command'] = function (me, args)
                local enabled = tonumber(table.unpack(args)) or 0
                me:friends_sync(enabled)
                return true
            end,
        },
        
        ['c2s_relay'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[payload] - 클라이언트가 C2S로 재전송 (0x4B). 생략 시 miss(0x0C)+자기 oid',
            ['command'] = function (me, args)
                if args and #args > 0 then
                    me:c2s_relay(table.concat(args, ' '))
                else
                    me:c2s_relay()
                end
                return true
            end,
        },

        ['user_info'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[type] [s1]..[s8] - USERINFO UI (0x4D). 생략 시 type=0 빈 폼. type=2면 유효 콤보 기본값',
            ['command'] = function (me, args)
                local type_val = tonumber(args and args[1]) or 0
                if args and #args > 1 then
                    local s = {}
                    for i = 1, 8 do
                        s[i] = (args[i + 1] and tostring(args[i + 1]) ~= '') and tostring(args[i + 1]) or nil
                    end
                    me:user_info(type_val, s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8])
                else
                    me:user_info(type_val)
                end
                return true
            end,
        },

        ['popup_message'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[text] [p0] [p1] [p2] [p3] - 긴 타이머 팝업 (0x35). 생략 시 기본 문구/크기',
            ['command'] = function (me, args)
                local text = args and args[1]
                local p0, p1, p2, p3 = tonumber(args and args[2]), tonumber(args and args[3]), tonumber(args and args[4]), tonumber(args and args[5])
                if text then
                    me:popup_message(text, p0, p1, p2, p3)
                else
                    me:popup_message()
                end
                return true
            end,
        },

        ['popup_input'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[text] [p0] [p1] [p2] [p3] - 편집 팝업 입력 (S2C 0x1B). 닫으면 텍스트 반환 (yield)',
            ['command'] = function (me, args)
                local text = args and args[1]
                local p0, p1, p2, p3 = tonumber(args and args[2]), tonumber(args and args[3]), tonumber(args and args[4]), tonumber(args and args[5])
                local result
                if text then
                    result = me:popup_input(text, p0, p1, p2, p3)
                else
                    result = me:popup_input()
                end
                me:chat(string.format('popup_input result: %s', tostring(result)))
                return true
            end,
        },
        
        ['세시화면'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[screen] [direction] [x] [y] - holyday_screen packet (cmd 0x5A, test)',
            ['command'] = function (me, args)
                local screen = tonumber(args[1]) or 0
                local direction = tonumber(args[2]) or 0
                local x = tonumber(args[3]) or 0
                local y = tonumber(args[4]) or 0
                me:holyday_screen(screen, direction, {x, y})
                return true
            end,
        },
        
        ['메일쓰기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<받는사람> <제목> <내용> - 메일 전송',
            ['command'] = function (me, args)
                local to, title, contents = table.unpack(args)
                if not to or not title or not contents then
                    me:message("사용법: /메일쓰기 <받는사람> <제목> <내용>")
                    return true
                end
                me:send_mail(to, title, contents)
                return true
            end,
        },
        
        ['시스템메일'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<제목> <내용> [만료일] - 시스템 메일 전송',
            ['command'] = function (me, args)
                local title, contents, expire_date = table.unpack(args)
                if not title or not contents then
                    me:message("사용법: /시스템메일 <제목> <내용> [만료일]")
                    return true
                end
                
                local success = me:send_system_mail(title, contents, expire_date)
                if success then
                    me:message("시스템 메일이 성공적으로 전송되었습니다.", MESSAGE_TYPE.BROWN)
                else
                    me:message("시스템 메일 전송에 실패했습니다.", MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },
        
        ['밴'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<유저이름> <사유> [기간(일)] - 유저 밴',
            ['command'] = function (me, args)
                local name, reason, days = table.unpack(args)
                if not name or not reason then
                    me:message("사용법: /밴 <유저이름> <사유> [기간(일)]")
                    return true
                end
                
                if days then
                    days = tonumber(days)
                    if not days or days < 0 then
                        me:message("기간은 0 이상의 숫자여야 합니다. (0 또는 생략 시 영구정지)")
                        return true
                    end
                end
                
                local success, error_msg = ban(me, name, reason, days)
                if success then
                    if days and days > 0 then
                        me:message(string.format("%s 유저를 %d일간 밴했습니다. (사유: %s)", name, days, reason), MESSAGE_TYPE.BROWN)
                    else
                        me:message(string.format("%s 유저를 영구정지했습니다. (사유: %s)", name, reason), MESSAGE_TYPE.BROWN)
                    end
                else
                    me:message(string.format("밴 실패: %s", error_msg or "알 수 없는 오류"), MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },
        
        ['밴해제'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<유저이름> - 유저 밴 해제',
            ['command'] = function (me, args)
                local name = table.unpack(args)
                if not name then
                    me:message("사용법: /밴해제 <유저이름>")
                    return true
                end
                
                local success, error_msg = unban(me, name)
                if success then
                    me:message(string.format("%s 유저의 밴을 해제했습니다.", name), MESSAGE_TYPE.BROWN)
                else
                    me:message(string.format("밴 해제 실패: %s", error_msg or "알 수 없는 오류"), MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },
        
        ['쿨타임초기화'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 스킬 쿨타임 초기화',
            ['command'] = function (me, args)
                for slot, spell in pairs(me:spells()) do
                    spell:delay(0)
                end
                return true
            end,
        },
        
        ['금전'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<금액> - 금전 설정',
            ['command'] = function (me, args)
                local money = table.unpack(args)
                if not money then
                    me:message("사용법: /금전 <금액>")
                    return true
                end
                money = tonumber(money)
                if not money or money < 0 then
                    me:message("금액은 0 이상의 숫자여야 합니다.")
                    return true
                end
                me:money(money)
                return true
            end,
        },
        
        ['성전환'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 성별 변경',
            ['command'] = function (me, args)
                if me:gender() == GENDER.MALE then
                    me:gender(GENDER.FEMALE)
                else
                    me:gender(GENDER.MALE)
                end
                return true
            end,
        },
        
        ['성별바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<성별> - 성별 변경',
            ['command'] = function (me, args)
                local gender = table.unpack(args)
                if not gender then
                    me:message("사용법: /성별바꾸기 <성별>")
                    return true
                end
                
                gender = tonumber(gender)
                if not gender or gender < 0 or gender > 1 then
                    me:message("성별은 0 또는 1이어야 합니다.")
                    return true
                end
                me:gender(gender)
                return true
            end,
        },
        
        ['업적'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<ID> <텍스트> <아이콘> <색상> - 업적 추가',
            ['command'] = function (me, args)
                local id, text, icon, color = table.unpack(args)
                if not id or not text or not icon or not color then
                    me:message("사용법: /업적 <ID> <텍스트> <아이콘> <색상>")
                    return true
                end

                id = tonumber(id)
                if id == nil or id < 0 then
                    me:message("업적 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                
                icon = tonumber(icon)
                if icon == nil or icon < 0 then
                    me:message("아이콘 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                
                color = tonumber(color)
                if color == nil or color < 0 then
                    me:message("색상 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                
                me:push_achievement(id, text, icon, color)
                return true
            end,
        },
        
        ['업적초기화'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 업적 초기화',
            ['command'] = function (me, args)
                local achievements = me:achievements()
                for _, achievement in pairs(achievements) do
                    me:erase_achievement(achievement:id())
                end
                return true
            end,
        },
        
        ['버프해제'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 모든 버프 해제',
            ['command'] = function (me, args)
                for _, buff in pairs(me:buffs()) do
                    me:unbuff(buff)
                end
                return true
            end,
        },
        
        ['아이템삭제'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 맵의 모든 아이템 삭제',
            ['command'] = function (me, args)
                local map = me:map()
                if map == nil then
                    me:message("맵에 있지 않습니다.")
                    return true
                end
                
                for _, item in pairs(map:objects(OBJECT_TYPE.ITEM)) do
                    item:destroy()
                end
                return true
            end,
        },
        
        ['아이템초기화'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '- 인벤토리 아이템 초기화',
            ['command'] = function (me, args)
                for slot, item in pairs(me:items()) do
                    me:rmitem(slot, item:count())
                end
                return true
            end,
        },
        
        ['리스폰'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[전역여부] - 몬스터 강제 리스폰',
            ['command'] = function (me, args)
                local global = table.unpack(args)
                if global == nil then
                    global = 0
                else
                    global = tonumber(global)
                    if not global or global < 0 or global > 1 then
                        me:message("전역 생성 여부는 0 또는 1이어야 합니다.")
                        return true
                    end
                end
                me:rezen_force(global == 1)
                return true
            end,
        },
        
        ['퀘스트완료'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<ID> - 퀘스트 완료',
            ['command'] = function (me, args)
                local id = table.unpack(args)
                if not id then
                    me:message("사용법: /퀘스트완료 <ID>")
                    return true
                end
                id = tonumber(id)
                if not id or id < 0 then
                    me:message("퀘스트 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                
                local quest = me:quest(id)
                if not quest then
                    me:message("존재하지 않는 퀘스트입니다.")
                    return true
                end
                if not quest:complete() then
                    me:message("퀘스트 완료에 실패했습니다.")
                    return true
                end
                return true
            end,
        },
        
        ['퀘스트설정'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<ID> <스텝> <진행도> <파라미터> - 퀘스트 설정',
            ['command'] = function (me, args)
                local id, step, progress, param = table.unpack(args)
                if not id then
                    me:message("사용법: /퀘스트설정 <ID> <스텝> <진행도> <파라미터>")
                    return true
                end
                
                id = tonumber(id)
                if not id or id < 0 then
                    me:message("퀘스트 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                step = tonumber(step)
                if step == nil or step < 0 then
                    me:message("스텝은 0 이상의 숫자여야 합니다.")
                    return true
                end
                progress = tonumber(progress)
                if progress == nil or progress < 0 then
                    me:message("진행도는 0 이상의 숫자여야 합니다.")
                    return true
                end
                param = tostring(param or '')
                local quest = me:quest(id)
                if not quest then
                    quest = me:start_quest(id)
                    if not quest then
                        me:message("퀘스트를 시작할 수 없습니다.")
                        return true
                    end
                end
                quest:step(step)
                quest:progress(progress)
                quest:param(param)
                return true
            end,
        },
        
        ['퀘스트제거'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<ID> - 퀘스트 제거',
            ['command'] = function (me, args)
                local id = table.unpack(args)
                if not id then
                    me:message("사용법: /퀘스트제거 <ID>")
                    return true
                end
                
                id = tonumber(id)
                if not id or id < 0 then
                    me:message("퀘스트 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                
                me:remove_quest(id)
                return true
            end,
        },
        
        ['퀘스트진행'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<ID> <진행도> - 퀘스트 진행도 설정',
            ['command'] = function (me, args)
                local id = args[1]
                local value = args[2]
                if not id or not value then
                    me:message("사용법: /퀘스트진행 <ID> <진행도>")
                    return true
                end
                id = tonumber(id)
                value = tonumber(value)
                if not id or id < 0 then
                    me:message("퀘스트 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
                if not value or value < 0 then
                    me:message("진행도는 0 이상의 숫자여야 합니다.")
                    return true
                end
                local quest = me:quest(id)
                if not quest then
                    me:message("해당 퀘스트를 보유하고 있지 않습니다. 퀘스트를 먼저 수락하세요.")
                    return true
                end
                quest:progress(value)
                me:message(string.format("퀘스트 %d 진행도를 %d로 설정했습니다.", id, value))
                return true
            end,
        },
        
        ['신수바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<신수> - 신수 변경 (청룡, 주작, 백호, 현무)',
            ['command'] = function (me, args)
                local creature_name = table.unpack(args)
                if not creature_name then
                    me:message("사용법: /신수바꾸기 <신수> (청룡, 주작, 백호, 현무 중 하나)")
                    return true
                end
                
                local creature_value = nil
                if creature_name == '청룡' then
                    creature_value = CREATURE.DRAGON
                elseif creature_name == '주작' then
                    creature_value = CREATURE.PHOENIX
                elseif creature_name == '백호' then
                    creature_value = CREATURE.TIGER
                elseif creature_name == '현무' then
                    creature_value = CREATURE.TURTLE
                else
                    me:message("신수는 청룡, 주작, 백호, 현무 중 하나여야 합니다.")
                    return true
                end
                
                local success = me:creature(creature_value)
                if success then
                    me:message(string.format("신수가 %s 변경되었습니다. 재접속 후 변경사항을 확인할 수 있습니다.", name_with(creature_name, '으로', '로')), MESSAGE_TYPE.BROWN)
                else
                    me:message("신수 변경에 실패했습니다.", MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },
        
        ['국적바꾸기'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '<국적> - 국적 변경 (고구려, 부여)',
            ['command'] = function (me, args)
                local nation_name = table.unpack(args)
                if not nation_name then
                    me:message("사용법: /국적바꾸기 <국적> (고구려, 부여 중 하나)")
                    return true
                end
                
                local nation_value = nil
                if nation_name == '고구려' then
                    nation_value = NATION.GOGURYEO
                elseif nation_name == '부여' then
                    nation_value = NATION.BUYEO
                else
                    me:message("국적은 고구려, 부여 중 하나여야 합니다.")
                    return true
                end
                
                local success = me:nation(nation_value)
                if success then
                    me:message(string.format("국적이 %s 변경되었습니다. 재접속 후 변경사항을 확인할 수 있습니다.", name_with(nation_name, '으로', '로')), MESSAGE_TYPE.BROWN)
                else
                    me:message("국적 변경에 실패했습니다.", MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },
        
        ['서버종료'] = {
            ['privilege'] = ROLE.SUPERADMIN,
            ['usage'] = '[지연시간] - 서버 종료',
            ['command'] = function (me, args)
                local delay = table.unpack(args)
                if delay == nil then
                    delay = 10
                else
                    delay = tonumber(delay)
                    if not delay or delay <= 0 then
                        me:message("지연시간은 0보다 큰 숫자여야 합니다.")
                        return true
                    end
                end
                
                while delay > 0 do
                    local term
                    if delay > 600 then
                        term = math.min(600, delay-600)
                    elseif delay > 60 then
                        term = math.min(60, delay-60)
                    elseif delay > 10 then
                        term = math.min(10, delay-10)
                    else
                        term = 1
                    end
                    
                    local times = delay
                    local hours = times // 3600
                    times = times - (hours * 3600)
                    
                    local mins = times // 60
                    times = times - (mins * 60)
                    
                    local secs = times % 60
                    local message = ''
                    if hours > 0 then
                        message = string.format('%d시간', hours)
                    end
                    
                    if mins > 0 then
                        message = string.format('%s %d분', message, mins)
                    end
                    
                    if secs > 0 then
                        message = string.format('%s %d초', message, secs)
                    end
                    
                    broadcast(string.format('[공지] %s 후 서버가 종료됩니다.', message), MESSAGE_TYPE.NOTIFY, BROADCAST_TYPE.GLOBAL)
                    sleep(term * 1000)
                    delay = delay - term
                end
                shutdown()
                return true
            end,
        },
        
        ['경험치배율'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[배율] - 경험치 배율 조회/설정',
            ['command'] = function (me, args)
                if #args == 0 then
                    local multiplier = exp_multiplier()
                    me:message(string.format("현재 경험치 배율: %.2fx", multiplier), MESSAGE_TYPE.BROWN)
                else
                    local value = tonumber(table.unpack(args))
                    if not value or value < 0 then
                        me:message("배율은 0 이상의 숫자여야 합니다.")
                        return true
                    end
                    exp_multiplier(value)
                    me:message(string.format("경험치 배율을 %.2fx로 설정했습니다. (모든 게임 서버에 적용됩니다)", value), MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },
        
        ['드롭률배율'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[배율] - 드롭률 배율 조회/설정',
            ['command'] = function (me, args)
                if #args == 0 then
                    local multiplier = drop_rate_multiplier()
                    me:message(string.format("현재 드롭률 배율: %.2fx", multiplier), MESSAGE_TYPE.BROWN)
                else
                    local value = tonumber(table.unpack(args))
                    if not value or value < 0 then
                        me:message("배율은 0 이상의 숫자여야 합니다.")
                        return true
                    end
                    drop_rate_multiplier(value)
                    me:message(string.format("드롭률 배율을 %.2fx로 설정했습니다. (모든 게임 서버에 적용됩니다)", value), MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },

        ['HTTP지연'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[밀리초] - HTTP 응답 지연 조회/설정 (테스트용)',
            ['command'] = function (me, args)
                if #args == 0 then
                    local delay = http_response_delay()
                    me:message(string.format("현재 HTTP 응답 지연: %dms", delay), MESSAGE_TYPE.BROWN)
                else
                    local value = tonumber(table.unpack(args))
                    if not value or value < 0 then
                        me:message("지연 시간은 0 이상의 숫자여야 합니다.")
                        return true
                    end
                    http_response_delay(value)
                    me:message(string.format("HTTP 응답 지연을 %dms로 설정했습니다. (이 서버에만 적용됩니다)", value), MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },

        ['이속고정'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[on|off] - LOCK_WALK_SPEED(0x04) 조회/설정 (클라 스티키, off 후 재접 필요할 수 있음)',
            ['command'] = function (me, args)
                if #args == 0 then
                    local enabled = me:option(OPTION.LOCK_WALK_SPEED)
                    me:message(string.format("이속고정(LOCK_WALK_SPEED): %s", enabled and "ON" or "OFF"), MESSAGE_TYPE.BROWN)
                    return true
                end

                local arg = string.lower(tostring(args[1]))
                local enabled = nil
                if arg == 'on' or arg == '1' or arg == 'true' then
                    enabled = true
                elseif arg == 'off' or arg == '0' or arg == 'false' then
                    enabled = false
                else
                    me:message("사용법: /이속고정 [on|off]")
                    return true
                end

                me:option(OPTION.LOCK_WALK_SPEED, enabled)
                me:message(string.format("이속고정(LOCK_WALK_SPEED): %s", enabled and "ON" or "OFF"), MESSAGE_TYPE.BROWN)
                if not enabled then
                    me:message("클라 플래그는 스티키입니다. OFF가 반영되지 않으면 재접속하세요.", MESSAGE_TYPE.BROWN)
                end
                return true
            end,
        },

        ['이속'] = {
            ['privilege'] = ROLE.ADMIN,
            ['usage'] = '[0-5] - 기본 이속(base_speed) 조회/설정 (유효값=base+buff, clamp 0~5)',
            ['command'] = function (me, args)
                if #args == 0 then
                    me:message(string.format("이속 base=%d buff=%d effective=%d",
                        me:base_speed(), me:buff_speed(), me:speed()), MESSAGE_TYPE.BROWN)
                    return true
                end

                local value = tonumber(args[1])
                if value == nil or value < 0 or value > 5 or value ~= math.floor(value) then
                    me:message("사용법: /이속 [0-5]")
                    return true
                end

                me:base_speed(value)
                me:message(string.format("기본 이속(base_speed)을 %d로 설정했습니다. (effective=%d)",
                    me:base_speed(), me:speed()), MESSAGE_TYPE.BROWN)
                return true
            end,
        }
    }

return M