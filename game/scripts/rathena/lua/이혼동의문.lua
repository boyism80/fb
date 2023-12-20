{
    set @sd, mysd();
    set @id, myid();
    set @marry_session, #marry_session;
    set @target_sd, @marry_session;
    set @list, list1(get_name(@target_sd) + "님께서 " + get_name(@sd) + "님과 이혼하시길 원합니다. 이혼하시겠습니까?", "예, 동의합니다.", "아니요, 거절합니다.");
    if(@list == 1)
    {
        set @pos, dialog(0, 0, 1, "이혼이 성립되었습니다.");
        if(@pos == 0) end;

        if(@target_sd <= 0) end;

        announce 5, "★ " + get_name(@sd) + "님과 " + get_name(@target_sd) + "님이 이혼하셨습니다.";

        set $marry_check, 0;
        set $marry_name$, "";
        set $divorce, $divorce+1;
        set #marry_session, @sd;

        dellegend @sd, 33;
        addlegend @sd, 34, 14, 0, 5, "생에 " + $divorce + "번 이혼을 하다.";

        set @nt, callfunc("Server_Nowtime");
        set $marry_time, @nt+604800;

        runscript @target_sd, "이혼성사";
    }
    else if(@list == 2)
    {
        set @pos, dialog(0, 0, 1, "이혼을 거절하셨습니다.");
        if(@pos == 0) end;

        if(@target_sd <= 0) end;

        set #marry_session, @sd;
        runscript @target_sd, "이혼거절";
    }
}