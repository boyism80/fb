function handle_attack(me, you)
    if you == nil then
        return false
    end

    math.randomseed(os.time())
    if math.random() < 0.2 then
        me:chat('여의주의 힘을 받은 용이여..')
        sleep(2000);
        
        me:chat('그대 이름은 청룡일지다...')
        sleep(2000);
        
        me:chat('나 청의태자의 이름으로 말하노니...')
        sleep(2000);
        
        me:chat('네 포효를 적에게 발산하라!!')
        sleep(2000);
        
        you:effect(0x89)
        you:sound(0x08)
        you:message(string.format('%s가 청룡마령참을 가합니다.', me:name()))

        me:action(0x03)
        me:chat('청룡마령참!!', 0x03)
        
        sleep(2000);

        return true
    end

    return false
end