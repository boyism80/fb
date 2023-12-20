{
    set @sd, mysd();
    set @id, myid();
    set @target_sd, #marry_session_2;
    set $marry_check, 1;
    set $marry_name$, get_name(@target_sd);

    additem @sd, name2itemid("잔치떡"), 50;
    if(countitem(name2itemid("사랑호출기")) == 0) { additem @sd, name2itemid("사랑호출기"), 1; }

    addlegend @sd, 33, 14, 0, 2, "유부남(배우자 : " + $marry_name$ + ")";

    set @pos, dialog(0, 0, 1, get_name(@target_sd) + "님과 결혼하였습니다. 축하드립니다.");
}