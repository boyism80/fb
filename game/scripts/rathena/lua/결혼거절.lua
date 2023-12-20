{
    set @sd, mysd();
    set @id, myid();
    set @target_sd, #marry_session_2;
    set @pos, dialog(0, 0, 1, get_name(@target_sd) + "님이 " + get_name(@sd) + "님과의 결혼을 거절하셨습니다.");
}