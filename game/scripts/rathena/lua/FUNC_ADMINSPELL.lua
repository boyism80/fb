{
    set @sd, mysd();
    set @id, myid();

    for(set @loop, 5001; @loop <= 5009; set @loop, @loop+1)
    {
        if(checkspell(@loop) == 52)
        {
            addspell @loop;
        }
    }
    message @sd, 0, "운영자 마법을 성공적으로 배웠습니다";
    return;
}