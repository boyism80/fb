function handle_attack(me, you)
	me:chat('1')
	sleep(1000)

	me:chat('2')
	sleep(1000)

	me:chat('3')
	sleep(1000)

    you:effect(0x89)
    me:action(0x03)
    you:sound(0x08)
    me:chat('청룡마령참')
    return true
end