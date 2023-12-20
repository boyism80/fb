{
    set @sd, mysd();
    set @id, myid();
    set @target_sd, #marry_session;
    set @pos, dialog(0, 0, 1, get_name(@target_sd) + "님이 " + get_name(@sd) + "님과의 이혼을 거절하셨습니다.");
}