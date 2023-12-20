{
    set @sd, mysd();
    set @id, myid();
    set @type, getarg(0);
    set @effect, getarg(2);
    set @sound, getarg(3);
    set @damage, rand(getarg(4), getarg(5));
    set @name$, getarg(6);
    set @target_id, obj_getfront();
    if(@target_id > 0)
    {
        if(rand(1, getarg(1)) <= getarg(7))
        {
            if(istype(@target_id) == 1)
            {
                set @target_sd, id2sd(@target_id);
                if(ismapoption(@sd, 6))
                {
                    set @r, rand(1, 10);
                    if(@r <= 5 && spellaethered(@target_sd, name2spellid("반탄공")))
                    {
                        callfunc "BEGINFUNC", @effect, @sound, 0, 0, 6, @id;
                        set_ac @target_sd, 1;
                        damage @target_id, @id, @damage, 2;
                        if(get_state(@target_sd) != 1 && get_hp(@target_sd) > 0) { set_ac @target_sd, @save_ac; }
                        message @sd, 3, "마법 보호!!";
                        goto NEXT1;
                    }
                    effect @target_id, @effect;
                    sound @id, @sound;

                    set @save_ac, get_ac(@target_sd);
                    set_ac @target_sd, 1;
                    damage @id, @target_id, @damage, @type;
                    if(get_state(@target_sd) != 1 && get_hp(@target_sd) > 0) { set_ac @target_sd, @save_ac; }
                    message @target_sd, 3, get_name(@sd) + "님께서 " + @name$ + "을(를) 가합니다.";
                }
NEXT1:
            }
            else if(istype(@target_id) == 2)
            {
                effect @target_id, @effect;
                sound @id, @sound;
                set @save_ac, get_mobac(@target_id);
                set_mobac @target_id, 1;
                damage @id, @target_id, @damage, @type;
                set_mobac @target_id, @save_ac;    
            }
        }
    }
    return;
}