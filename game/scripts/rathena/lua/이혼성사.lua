{
    set @sd, mysd();
    set @id, myid();
    set @target_sd, #marry_session;
    set $marry_check, 0;
    set $marry_name$, "";
    set $divorce, $divorce+1;

    dellegend @sd, 33;
    addlegend @sd, 34, 14, 0, 5, "생에 " + $divorce + "번 이혼을 하다.";

    set @nt, callfunc("Server_Nowtime");
    set $marry_time, @nt+604800;

    set @pos, dialog(0, 0, 1, get_name(@target_sd) + "님과 이혼하였습니다. 다음엔 좀 더 신중히 결혼을 하시길 바랍니다.");
}