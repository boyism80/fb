{
    set @sd, mysd();
    set @id, myid();
    image 0, 0, 0;
    set @pos, menu("동태극던전으로 가시겠습니까?", "네, 가겠습니다.", "아니오, 안갈래요.");
    if(@pos == 2) { end; }
    if(countitem(name2itemid("동문열쇠")) == 0) { set @pos, dialog(0, 0, 1, "동문열쇠가 없으면 들어갈 수 없습니다."); end; }
    delitem @sd, name2itemid("동문열쇠"), 1;
    set @pos, dialog(0, 0, 1, "이동하겠습니다.");
    if(party(@sd) != 0)
    {
        for(set @i, 0; @i < partymax(); set @i, @i+1)
        {
            set @target_sd, party_member(@sd, @i);
            if(@target_sd > 0 && @target_sd != @sd)
            {
                if(get_mapid(@sd) == get_mapid(@target_sd))
                {
                    warp @target_sd, 1772, 13, 13;
                }
            }
        }
        warp @sd, 1772, 13, 13;
    }
    else { warp @sd, 1772, 13, 13; }
}