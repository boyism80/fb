{
    set @sd, mysd();
    set @id, myid();

    set @pos, dialog(0, 0, 1, "파괴왕의방으로 가기 위해선 해골왕의뼈, 유성지의보패, 하선녀의실타래, 불의수정이 필요하다네.");
    if(@pos == 0) end;

    setarray @item$[0], "해골왕의뼈", "유성지의보패", "하선녀의실타래", "불의수정";
    for(set @loop, 0; @loop < 4; set @loop, @loop+1)
    {
        if(countitem(name2itemid(@item$[@loop])) == 0)
        {
            set @pos, dialog(0, 0, 0, "자네는 " + @item$[@loop] + "가 없군..");
            deletearray @item$[0], getarraysize(@item$);
            end;
        }
    }
    set @pos, dialog(0, 0, 1, "재료를 모두 가지고 있군. 그럼 파괴왕의방으로 보내주겠네.");
    if(@pos == 0) { deletearray @item$[0], getarraysize(@item$); end; }

    warp @sd, name2mapid("파괴왕의제단"), rand(13, 16), rand(35, 38);
}