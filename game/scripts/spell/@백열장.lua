function handle_spell(me, spell, you)
	for i, object in pairs(me:map():objects()) do
		if object ~= me and object ~= you then
			object:effect(110)
		end
	end

	me:str(me:str() + 2)
	me:int(10)
	me:dex(10)

	me:exp(100)

	me:dropitem(4, true)
end