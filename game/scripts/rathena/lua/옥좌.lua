{
    set @sd, mysd();
    set @id, myid();

    message @sd, 4, "[알림] 옥좌를 점령하였습니다.";
    spellaether @sd, name2spellid("옥좌점령"), 310;
}