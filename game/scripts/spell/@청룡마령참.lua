function handle_spell(me, spell, you)
    you:effect(0x89)
    me:action(ACTION_ARROW)
    you:sound(0x08)
    me:chat('청룡마령참')
end