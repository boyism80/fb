{
    set @sd, mysd();
    set @id, myid();
    set @bonus, 0;
    set @minus, 0;

    if(party(@sd) != 0)
    {
        for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
        {
            set @target_sd, party_member(@sd, @loop);
            if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
            {
                if(get_job(@target_sd) == 4) { set @bonus, @bonus+1; }

                if(get_level(@sd)+30 < get_level(@target_sd)) { set @minus, @minus+1; }
                else if(get_level(@sd) < get_level(@target_sd)-30) { set @minus, @minus+1; }

                if(get_joblevel(@sd) == get_joblevel(@sd)+2) { set @minus, @minus+1; }
                else if(get_joblevel(@sd) == get_joblevel(@sd)+3) { set @minus, @minus+1; }
                else if(get_joblevel(@sd) == get_joblevel(@sd)+4) { set @minus, @minus+1; }
                else if(get_joblevel(@sd) == get_joblevel(@sd)-2) { set @minus, @minus+1; }
                else if(get_joblevel(@sd) == get_joblevel(@sd)-3) { set @minus, @minus+1; }
                else if(get_joblevel(@sd) == get_joblevel(@sd)-4) { set @minus, @minus+1; }
            }
        }
        for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
        {
            set @target_sd, party_member(@sd, @loop);
            if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
            {
                if(@bonus > 0 && @minus == 0) { set @count, 4; }
                else { set @count, 0; }

                set_userexpevent @target_sd, @count;
            }
        }
    }
    else
    {
        set_userexpevent @sd, 0;
    }
    return;
}