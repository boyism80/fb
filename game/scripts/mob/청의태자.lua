function on_attack(me, you)
    math.randomseed(seed())
    if math.random() > 0.01 then
        return false
    end
    
    if you == nil then
        return true
    end

    local ptr_me = me:ptr()
    local ptr_you = you:ptr()
    local thread = me:thread()

    me:chat('여의주의 힘을 받은 용이여..')
    sleep(2000);
    
    if not thread:assert_ptr(ptr_me) then
        return false
    end
    me:chat('그대 이름은 청룡일지다...')
    sleep(2000);
    
    if not thread:assert_ptr(ptr_me) then
        return false
    end
    me:chat('나 청의태자의 이름으로 말하노니...')
    sleep(2000);

    if not thread:assert_ptr(ptr_me) then
        return false
    end
    me:chat('네 포효를 적에게 발산하라!!')
    sleep(2000);

    if not thread:assert_ptr(ptr_me) then
        return false
    end
    me:chat('청룡마령참!!', 0x03)

    if not thread:assert_ptr(ptr_you) then
        return false
    end

    if me:near(you) then
        me:cast(you, '청룡마령참')
    end
    sleep(2000);
    return true
end