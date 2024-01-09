function on_interact(me, npc)
	local selected = npc:item(me, 'message', { name2item('동동주'), name2item('막걸리'), ['야월도'] = 100000})
    me:chat(selected)
end