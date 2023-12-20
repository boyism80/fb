{
    set @sd, mysd();
    set @id, myid();
    set @pos, list2(@ret, 0, 0, "....", "파괴왕 소환");
    if(@pos == 0) { end; }
    if(@ret == 1)
    {
        setarray @mixitem$[0], "해골왕의뼈", "유성지의보패", "불의수정", "하선녀의실타래";
        for(set @i, 0; @i < 4; set @i, @i+1)
        {
            if(countitem(name2itemid(@mixitem$[@i])) == 0)
            {
                set @pos, dialog(0, 0, 0, @mixitem$[@i] + "이(가) 없습니다.");
                deletearray @mixitem$[0], getarraysize(@mixitem$);
                end;
            }
        }
        for(set @i, 0; @i < 4; set @i, @i+1)
        {
            delitem @sd, name2itemid(@mixitem$[@i]), 1;
        }
        deletearray @mixitem$[0], getarraysize(@mixitem$);
        deletenpc obj_getxy(14, 3), 1842;
        mobspawn 1842, 14, 3, 2, 1130;
    }
}