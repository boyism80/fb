{
    set @id, getarg(0);
    set @rand, rand(1, getarg(1));
    set @rate, getarg(2);
    set @action, getarg(3);
    set @sound, getarg(4);
    set @effect, getarg(5);
    set @damage, getarg(6);
    set @type, getarg(7);
    set @magic$, getarg(8);
    set @range, getarg(9);
    set @say$, getarg(10);

    if(mobspellaethered(@id, name2spellid("절망")) == 1) { mobspellaether @id, name2spellid("절망"), 1; }

    if(@rand <= @rate) { end; }

    set @x1, get_x(@id)-@range;
    set @x2, get_x(@id)+@range;
    set @y1, get_y(@id)-@range;
    set @y2, get_y(@id)+@range;

    if(@x1 < 0) { set @x1, 0; }
    if(@x2 > mapwidth()-1) { set @x2, mapwidth()-1; }
    if(@y1 < 0) { set @y1, 0; }
    if(@y2 > mapheight()-1) { set @y2, mapheight()-1; }

    for(set @i, @x1; @i <= @x2; set @i, @i+1)
    {
        for(set @j, @y1; @j <= @y2; set @j, @j+1)
        {
            set @target_id, obj_getxy(@i, @j);
            if(@target_id > 0)
            {
                if(istype(@target_id) == 1)
                {
                    set @target_sd, id2sd(@target_id);

                    if(@action >= 0) { action @id, @action, 30, 0; }
                    if(@sound >= 0) { sound @target_id, @sound; }
                    if(@effect >= 0) { effect @target_id, @effect; }

                    damage @id, @target_id, @damage, @type;
                    message @target_sd, 3, get_mobname(@id) + "(이)가 " + @magic$ + "을(를) 가합니다.";

                    if(@say$ != "-1") { say @id, 0, get_mobname(@id) + ": " + @say$; }
                }
            }
        }
    }
    return;
}