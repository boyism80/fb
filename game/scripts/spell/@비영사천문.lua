function handle_spell(me, spell, message)
	local map = name2map(message)
	if map ~= nil then
		me:map(map, map:width()/2, map:height()/2)
	end
end