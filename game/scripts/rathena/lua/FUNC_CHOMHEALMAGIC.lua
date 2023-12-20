{
    set @id, myid();
    set @sd, mysd();
    set @hp, getarg(1);

    callfunc "MANA_DELAY", getarg(0), 0, @sd;

    if(party(@sd) != 0)
    {
        for(set @i, 0; @i < partymax(); set @i, @i+1)
        {
            set @target_sd, party_member(@sd, @i);
            if(@target_sd > 0) { set @member_count, @member_count+1; }
        }
        set @count, 0;
        for(set @i, 0; @i < partymax(); set @i, @i+1)
        {
            set @target_sd, party_member(@sd, @i);
            if(@target_sd > 0)
            {
                if(get_state(@target_sd) != 1 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    set @hp, (@hp*(@member_count-@count))/@member_count;
                    set @target_id, sd2id(@target_sd);
                    if( (get_hp(@target_sd)+@hp) > get_maxhp(@target_sd) ) { set_hp @target_sd, get_maxhp(@target_sd); showgage @target_id; }
                    else { set_hp @target_sd, get_hp(@target_sd)+@hp; showgage @target_id; }
                    callfunc "BEGINFUNC", getarg(2), getarg(3), 0, 0, 6, @target_id;
                    if(@id != @target_id)
                    {
                        message @target_sd, 3, get_name(@sd) + "님이 " + getarg(4) + "을(를) 외워주셨습니다.";
                    }
                    set @count, @count+1;
                }
            }
        }
    }
    message @sd, 3, getarg(4) + "을(를) 외웠습니다.";
    set_mp @sd, get_mp(@sd)-getarg(0);
    return;
}