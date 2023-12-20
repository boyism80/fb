{
    set @sd, mysd();
    set @id, myid();

    message @sd, 5, "현재 경험치 배율은 " + #server_exprate + "배 입니다.";

    set @pos, input1(@exprate, "경험치 배율을 몇배로 수정하시겠습니까?");
    if(@pos == 0) { end; }

    if(@exprate <= 2) { set #server_exprate, 3; }
    else { set #server_exprate, @exprate; }

    broadcast 5, "경험치가 " + #server_exprate + "배로 수정되었습니다.";

    return;
}