function handle_spell(me, spell, you)
	for i, object in pairs(me:map():objects()) do
		if object ~= me and object ~= you then
			object:effect(110)
		end
	end
end