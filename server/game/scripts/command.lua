command_funcs = {
    ['명령어'] = {
        ['privilege'] = ROLE_USER,
        ['usage'] = '- 사용 가능한 명령어 목록 표시',
        ['command'] = function (me, args)
            local user_role = me:role()
            local available_commands = {}
            
            for cmd_name, cmd_data in pairs(command_funcs) do
                if cmd_name ~= '명령어' then
                    local privilege = nil
                    local usage = nil
                    
                    if type(cmd_data) == 'table' then
                        privilege = cmd_data['privilege'] or ROLE_ADMIN
                        usage = cmd_data['usage'] or ''
                    else
                        privilege = ROLE_ADMIN
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
            
            me:message("=== 사용 가능한 명령어 목록 ===", MESSAGE_TYPE_BROWN)
            for i, cmd in ipairs(available_commands) do
                local role_name = ''
                if cmd.privilege == ROLE_OWNER then
                    role_name = '[OWNER]'
                elseif cmd.privilege == ROLE_SUPERADMIN then
                    role_name = '[SUPERADMIN]'
                elseif cmd.privilege == ROLE_ADMIN then
                    role_name = '[ADMIN]'
                elseif cmd.privilege == ROLE_MODERATOR then
                    role_name = '[MODERATOR]'
                end
                
                local line = string.format("%d. /%s %s", i, cmd.name, cmd.usage)
                if role_name ~= '' then
                    line = line .. ' ' .. role_name
                end
                me:message(line, MESSAGE_TYPE_BROWN)
            end
            me:message(string.format("총 %d개의 명령어가 있습니다.", #available_commands), MESSAGE_TYPE_BROWN)
            
            return true
        end,
    },

    ['관리자'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 관리자 권한 부여',
        ['command'] = function (me, args)
            me:mkspell('강제이동(좌)')
            me:mkspell('강제이동(하)')
            me:mkspell('강제이동(우)')
            me:mkspell('강제이동(상)')
            return true
        end,
    },

    ['권한변경'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<유저이름> <직책> - 권한 변경',
        ['command'] = function (me, args)
            local name, role = table.unpack(args)
            if not name or not role then
                me:message("사용법: /권한변경 <직책이름> <직책>")
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<맵이름> [x] [y] - 맵 이동',
        ['command'] = function (me, args)
            local map, x, y = table.unpack(args)
            if not map then
                me:message("사용법: /맵이동 <맵이름> [x] [y]")
                return true
            end
            
            if name2map(map) == nil then
                me:message(string.format("존재하지 않는 맵입니다: %s", map))
                return true
            end

            if x ~= nil and y ~= nil then
                x = tonumber(x)
                y = tonumber(y)
                if not x or not y then
                    me:message("좌표는 숫자여야 합니다.")
                    return true
                end
                me:map(map, x, y)
            else
                me:map(map)
            end
            return true
        end,
    },

    ['사운드'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<시간(초)> - 타이머 설정',
        ['command'] = function (me, args)
            local time = table.unpack(args)
            if not time then
                me:message("사용법: /타이머 <시간(초)>")
                return true
            end
            time = tonumber(time)
            if not time or time <= 0 then
                me:message("시간은 0보다 큰 숫자여야 합니다.")
                return true
            end
            timer(time, true)
            return true
        end,
    },

    ['타이틀'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<변신ID> - 변신',
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
            me:disguise(value)
            return true
        end,
    },

    ['변신해제'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 변신 해제',
        ['command'] = function (me, args)
            me:disguise(nil)
            return true
        end,
    },

    ['마법배우기'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<몬스터이름> <거리> - 범위 내 몬스터 생성',
        ['command'] = function (me, args)
            local name, distance = table.unpack(args)
            
            -- Validate arguments
            if not name or not distance then
                me:message("사용법: /몬스터범위생성 <몬스터이름> <거리>")
                return true
            end
            
            distance = tonumber(distance)
            if not distance or distance < 0 then
                me:message("거리는 0 이상의 숫자여야 합니다.")
                return true
            end
            
            -- Get player's current position
            local player_x, player_y = me:position()
            local spawned_count = 0
            local oids = {}
            
            -- Spawn monsters within the specified distance using squared distance
            local distance_squared = distance * distance
            for dx = -distance, distance do
                for dy = -distance, distance do
                    -- Skip the center position (player's position)
                    if dx ~= 0 or dy ~= 0 then
                        -- Calculate squared distance (faster than sqrt)
                        local distance_sq = dx * dx + dy * dy
                        
                        -- Only spawn if within the specified distance
                        if distance_sq <= distance_squared then
                            local target_x = player_x + dx
                            local target_y = player_y + dy
                            
                            -- Validate position bounds
                            if target_x >= 0 and target_y >= 0 and target_x < 1000 and target_y < 1000 then
                                local monster = me:spawn_mob(name, target_x, target_y, false, false)
                                if monster then
                                    spawned_count = spawned_count + 1
                                    table.insert(oids, monster:id())
                                end
                            end
                        end
                    end
                end
            end
            
            -- Call bulk_update if monsters were spawned
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 모든 몬스터 제거',
        ['command'] = function (me, args)
            local map = me:map()
            if map == nil then
                me:message("맵에 있지 않습니다.")
                return true
            end

            local objects = map:objects(OBJECT_TYPE_MOB)
            for _, object in ipairs(objects) do
                object:destroy()
            end

            return true
        end,
    },

    ['직업바꾸기'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<레벨> - 레벨 설정',
        ['command'] = function (me, args)
            local level = table.unpack(args)
            if not level then
                me:message("사용법: /레벨바꾸기 <레벨>")
                return true
            end
            level = tonumber(level)
            if not level or level < 1 then
                me:message("레벨은 1 이상의 숫자여야 합니다.")
                return true
            end
            me:level(level)
            return true
        end,
    },

    ['힘바꾸기'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
            if me:state() == STATE_GHOST then
                me:state(STATE_NORMAL)
            end
            return true
        end,
    },

    ['마력바꾸기'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<아이템이름> [개수] [저장여부] - 아이템 생성',
        ['command'] = function (me, args)
            local name, count, store = table.unpack(args)
            if not name then
                me:message("사용법: /아이템생성 <아이템이름> [개수] [저장여부]")
                return true
            end
            
            if count == nil then
                count = 1
            else
                count = tonumber(count)
                if not count or count < 1 then
                    me:message("개수는 1 이상의 숫자여야 합니다.")
                    return true
                end
            end
            
            if store == nil then
                store = true
            else
                store = tonumber(store) == 1
            end
            me:mkitem(name, count, store)
            return true
        end,
    },

    ['월드맵'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 스크립트 실행',
        ['command'] = function (me, args)
            me:script("script.lua", "func", 1, "hello", "good")
            return true
        end,
    },

    ['머리바꾸기'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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

    ['맵타일'] = {
        ['privilege'] = ROLE_ADMIN,
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
                me:message(string.format('타일 : %d\n오브젝트 : %d\n블록 : %s', id, obj, blocked), MESSAGE_TYPE_POPUP)
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 서버 저장',
        ['command'] = function (me, args)
            save()
            return true
        end,
    },

    ['랜덤이동'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 랜덤 맵 이동',
        ['command'] = function (me, args)
            local maps = maps()
            if not maps or #maps == 0 then
                me:message("이동할 수 있는 맵이 없습니다.")
                return true
            end
            local map = maps[math.random(0, #maps-1)]
            local x = math.random(0, map:width())
            local y = math.random(0, map:height())
            me:map(map, x, y)
            return true
        end,
    },

    ['엔피씨생성'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<NPC이름> [맵이름] [x] [y] - NPC 생성',
        ['command'] = function (me, args)
            local name, map, x, y = table.unpack(args)
            if not name then
                me:message("사용법: /엔피씨생성 <NPC이름> [맵이름] [x] [y]")
                return true
            end
            
            if map == nil then
                map = me:map()
            else
                map = name2map(map)
                if not map then
                    me:message(string.format("존재하지 않는 맵입니다: %s", map))
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- NPC 제거',
        ['command'] = function (me, args)
            local front = me:front(OBJECT_TYPE_NPC)
            if front == nil then
                return true
            end

            front:destroy()
            return true
        end,
    },

    ['내구도'] = {
        ['privilege'] = ROLE_ADMIN,
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
                if model:attr(ITEM_ATTRIBUTE_EQUIPMENT) then
                    item:durability(model:durability() * (percent / 100.0))
                end
            end
            return true
        end,
    },

    ['sleep'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<시간(초)> - 대기',
        ['command'] = function (me, args)
            local time = table.unpack(args)
            if not time then
                me:message("사용법: /sleep <시간(초)>")
                return true
            end
            time = tonumber(time)
            if not time or time <= 0 then
                me:message("시간은 0보다 큰 숫자여야 합니다.")
                return true
            end
            sleep(time)
            me:message('done')
            return true
        end,
    },

    ['광고'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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

    ['메일쓰기'] = {
        ['privilege'] = ROLE_ADMIN,
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

    ['쿨타임초기화'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 스킬 쿨타임 초기화',
        ['command'] = function (me, args)
            for slot, spell in pairs(me:spells()) do
                spell:delay(0)
            end
            return true
        end,
    },

    ['금전'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 성별 변경',
        ['command'] = function (me, args)
            if me:sex() == SEX_MAN then
                me:sex(SEX_WOMAN)
            else
                me:sex(SEX_MAN)
            end
            return true
        end,
    },

    ['성별바꾸기'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<성별> - 성별 변경',
        ['command'] = function (me, args)
            local sex = table.unpack(args)
            if not sex then
                me:message("사용법: /성별바꾸기 <성별>")
                return true
            end

            sex = tonumber(sex)
            if not sex or sex < 0 or sex > 1 then
                me:message("성별은 0 또는 1이어야 합니다.")
                return true
            end
            me:sex(sex)
            return true
        end,
    },

    ['업적'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '<ID> <텍스트> [아이콘] [색상] - 업적 추가',
        ['command'] = function (me, args)
            local id, text, icon, color = table.unpack(args)
            if not id or not text then
                me:message("사용법: /업적 <ID> <텍스트> [아이콘] [색상]")
                return true
            end
            
            id = tonumber(id)
            if not id or id < 0 then
                me:message("업적 ID는 0 이상의 숫자여야 합니다.")
                return true
            end
            
            if icon then
                icon = tonumber(icon)
                if not icon or icon < 0 then
                    me:message("아이콘 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
            end
            
            if color then
                color = tonumber(color)
                if not color or color < 0 then
                    me:message("색상 ID는 0 이상의 숫자여야 합니다.")
                    return true
                end
            end
            
            me:push_achievement(id, text, icon, color)
            return true
        end,
    },

    ['업적초기화'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 업적 초기화',
        ['command'] = function (me, args)
            local achievements = me:achievements()
            for id, achievement in pairs(achievements) do
                me:erase_achievement(id-1)
            end
            return true
        end,
    },

    ['버프해제'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 모든 버프 해제',
        ['command'] = function (me, args)
            for _, buff in pairs(me:buffs()) do
                me:unbuff(buff)
            end
            return true
        end,
    },

    ['아이템삭제'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 맵의 모든 아이템 삭제',
        ['command'] = function (me, args)
            local map = me:map()
            if map == nil then
                me:message("맵에 있지 않습니다.")
                return true
            end

            for _, item in pairs(map:objects(OBJECT_TYPE_ITEM)) do
                item:destroy()
            end
            return true
        end,
    },

    ['아이템초기화'] = {
        ['privilege'] = ROLE_ADMIN,
        ['usage'] = '- 인벤토리 아이템 초기화',
        ['command'] = function (me, args)
            for slot, item in pairs(me:items()) do
                me:rmitem(slot, item:count())
            end
            return true
        end,
    },

    ['리스폰'] = {
        ['privilege'] = ROLE_ADMIN,
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
        ['privilege'] = ROLE_ADMIN,
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

    ['퀘스트제거'] = {
        ['privilege'] = ROLE_ADMIN,
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

    ['서버종료'] = {
        ['privilege'] = ROLE_SUPERADMIN,
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

                broadcast(string.format('[공지] %s 후 서버가 종료됩니다.', message), MESSAGE_TYPE_NOTIFY, BROADCAST_TYPE_GLOBAL)
                sleep(term * 1000)
                delay = delay - term
            end
            shutdown()
            return true
        end,
    }
}
