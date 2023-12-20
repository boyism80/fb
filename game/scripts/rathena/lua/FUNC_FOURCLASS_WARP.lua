{
    set @sd, mysd();
    set @id, myid();
    image 0, 0, 0;
    if($fourclass != getarg(0)) end;
    set @pos, list2(@ret, 0, 0, "이 곳은 " + getarg(4) + "" + getarg(5) + " 머무는 곳, 함께 온 동료가 있으면 같이 들여보내주겠다.\\n동료가 있느냐?", "네. 있습니다.", "아니오, 없습니다.");
    if(@pos == 0) { end; }
    else if(@ret == 1)
    {
        if(party(@sd) != 0)
        {
            for(set @i, 0; @i < partymax(); set @i, @i+1)
            {
                set @target_sd, party_member(@sd, @i);
                if(@target_sd > 0)
                {
                    if(get_mapid(@sd) == get_mapid(@target_sd))
                    {
                        message @target_sd, 0, "해당 맵에서 로그아웃시 자동귀환 됩니다.";
                        message @target_sd, 0, "해당 맵에서 나갈시 퀘스트 진행자와 다시 와야 합니다.";
                        warp @target_sd, getarg(1), getarg(2), getarg(3);
                    }
                }
            }
        }
        else
        {
            set @pos, dialog(0, 0, 1, "동료가 없는 것 같은데?");
            end;
        }
    }
    else if(@ret == 2)
    {
        message @sd, 0, "해당 맵에서 로그아웃시 자동귀환 됩니다.";
        message @sd, 0, "해당 맵에서 나갈시 퀘스트 진행자와 다시 와야 합니다.";
        warp @sd, getarg(1), getarg(2), getarg(3);
    }
    return;
}