{
    set @sd, mysd();
    set @id, myid();

    if(get_mapname(@sd) == $mountain_name$ && get_x(@id) == $mountain_x && get_y(@id) == $mountain_y && countitem(name2itemid("산신의보물지도")) > 0)
    {
        delitem @sd, name2itemid("산신의보물지도"), 1;
        message @sd, 3, "산신의보물지도 주었습니다";
        additem @sd, name2itemid("산신의비단"), 1;
    }
}