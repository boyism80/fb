{
    set @sd, mysd();
    set @id, myid();
    set @marry_session_2, #marry_session_2;
    set @target_sd, @marry_session_2;
    set @list, list1("신랑 " + get_name(@target_sd) + "님께서 신부 " + get_name(@sd) + "님과 결혼하시길 원합니다. 결혼하시겠습니까?", "예, 결혼합니다.", "아니요, 결혼하지 않습니다.");
    if(@list == 1)
    {
        set @pos, dialog(0, 0, 1, "결혼이 성립되었습니다.");
        if(@pos == 0) end;

        if(@target_sd <= 0) end;

        announce 5, "♥ " + get_name(@target_sd) + "님과 " + get_name(@sd) + "님께서 결혼하셨습니다. 모두 축하해주세요.";

        set $marry_check, 1;
        set $marry_name$, get_name(@target_sd);
        set #marry_session_2, @sd;

        additem @sd, name2itemid("잔치떡"), 50;
        if(countitem(name2itemid("사랑호출기")) == 0) { additem @sd, name2itemid("사랑호출기"), 1; }

        addlegend @sd, 33, 14, 0, 2, "유부녀(배우자 : " + $marry_name$ + ")";

        runscript @target_sd, "결혼성사";
    }
    else if(@list == 2)
    {
        set @pos, dialog(0, 0, 1, "결혼을 거절하셨습니다.");
        if(@pos == 0) end;

        if(@target_sd <= 0) end;

        set #marry_session_2, @sd;
        runscript @target_sd, "결혼거절";
    }
}