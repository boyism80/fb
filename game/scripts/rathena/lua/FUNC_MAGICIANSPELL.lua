{
    set @id, getarg(0);
    set @sd, getarg(1);
    set @sound, getarg(2);
    set @damage, getarg(3);
    set @effect, getarg(4);
    
    if(getarg(6) != 0) { callfunc "MANA_DELAY", getarg(6), 0, @sd; }
    
    if($damageup) { set @damage, @damage*2; }

    set @target_id, script_target(@sd);
    if(@target_id > 0)
    {        
        if(istype(@target_id) == 1)
        {
            set @target_sd, id2sd(@target_id);
            if(ismapoption(@sd, 6) && (@target_id != @id))
            {
                set @r, rand(1, 10);
                if(@r <= 5 && spellaethered(@target_sd, name2spellid("반탄공")))
                {
                    callfunc "BEGINFUNC", @effect, @sound, 0, 0, 6, @id;
                    damage @target_id, @id, @damage, 0;
                    message @sd, 3, "마법 보호!!";
                    goto NEXT;
                }
                if(@target_id != @id) { callfunc "BEGINFUNC", @effect, @sound, 0, 0, 6, @target_id; }
                damage @id, @target_id, @damage, 0;
                if(@target_sd != @sd)
                {
                    message @target_sd, 3, get_name(@sd) + "님이 " + getarg(5) +"을(를) 가합니다.";
                }
            }
            else { message @sd, 3, "걸리지 않습니다."; end; }
        }
        else if(istype(@target_id) == 2)
        {
            if(@target_id != @id) { callfunc "BEGINFUNC", @effect, @sound, 0, 0, 6, @target_id; }
            damage @id, @target_id, @damage, 0;
        }
        else { message @sd, 3, "걸리지 않습니다."; end; }
NEXT:
        set_mp @sd, get_mp(@sd)-getarg(6);
        message @sd, 3, getarg(5) + "을(를) 외웠습니다.";
    }
    return;
}