{
    set @id, getarg(0);
    set @sd, getarg(1);
    set @pk, ismapoption(@sd, 6);
    set @script_target, script_target(@sd);
    set @x, get_x(@script_target);
    set @y, get_y(@script_target);
    set @sound, getarg(2);
    set @damage, getarg(3);
    set @effect, getarg(4);
    
    callfunc "MANA_DELAY", getarg(6), 0, @sd;
    set_mp @sd, get_mp(@sd)-getarg(6);

    if($damageup) { set @damage, @damage*2; }

    if(@script_target > 0 && @script_target != @id)
    {
        if(istype(@script_target) == 1 && @pk)
        {
            set @target_sd, id2sd(@script_target);
            set @r, rand(1, 10);
            if(@r <= 5 && spellaethered(@target_sd, name2spellid("반탄공")))
            {
                callfunc "BEGINFUNC", @effect, @sound, 0, 0, 6, @id;
                damage @script_target, @id, @damage, 0;
                message @sd, 3, "마법 보호!!";
                goto NEXT;
            }
            effect @script_target, @effect;
            damage @id, @script_target, @damage, 0;
            sound @script_target, @sound;
            message @target_sd, 3, get_name(@sd) + "님이 " + getarg(5) + "을(를) 가합니다.";
        }
        else if(istype(@script_target) == 2)
        {
            effect @script_target, @effect;
            damage @id, @script_target, @damage, 0;
        }
    }
    else { set @notarget, @notarget+1; }
NEXT:
    setarray @target_x[0], @x+1, @x  , @x-1, @x;
    setarray @target_y[0], @y  , @y+1, @y  , @y-1;

    for(set @loop, 0; @loop < 4; set @loop, @loop+1)
    {
        set @target_id, obj_getxy(@target_x[@loop], @target_y[@loop]);
        if(@target_id > 0)
        {
            if(istype(@target_id) == 1 && @pk)
            {
                set @target_sd, id2sd(@target_id);
                set @r, rand(1, 10);
                if(@r <= 5 && spellaethered(@target1_sd, name2spellid("반탄공")))
                {
                    callfunc "BEGINFUNC", @effect, @sound, 0, 0, 6, @id;
                    damage @target_id, @id, @damage, 0;
                    message @sd, 3, "마법 보호!!";
                    goto NEXT2;
                }
                effect @target_id, @effect;
                damage @id, @target_id, @damage, 0;
                sound @target_id, @sound;
                message @target_sd, 3, get_name(@sd) + "님이 " + getarg(5) + "을(를) 가합니다.";
            }
            else if(istype(@target_id) == 2)
            {
                effect @target_id, @effect;
                damage @id, @target_id, @damage, 0;
            }
        }
        else { set @notarget, @notarget+1; }
    }
    if(@notarget != 4)
    {
        action @id, 6, 30, @sound;
NEXT2:
        message @sd, 3, getarg(5) + "을(를) 외웠습니다.";
    }
    deletearray @target_x[0], getarraysize(@target_x);
    deletearray @target_y[0], getarraysize(@target_y);
    return;
}