{
    set @sd, mysd();
    set @id, myid();

POS01:
    set @pos, dialog(0, 0, 1, "이 곳은 북천황이 봉인된 곳..");
    if(@pos == 0) end;
POS02:
    set @pos, dialog(0, 1, 1, "북천황은 여러사람을 아무렇지도 않게 죽인 매우 잔인한 녀석이다.");
    if(@pos == 0) end;
    else if(@pos == -1) goto POS01;
POS03:
    set @pos, dialog(0, 1, 1, "그러해서 하늘의 선녀가 북천황을 이곳에 봉인하고 나는 이 문을 지키고있다.");
    if(@pos == 0) end;
    else if(@pos == -1) goto POS02;
POS04:
    set @pos, dialog(0, 1, 1, "그렇기 때문에 북천황을 부르기위해선 북천황이 다른곳으로 나갈 수 없도록 현철과 오독산으로 그문을 지켜야한다.");
    if(@pos == 0) end;
    else if(@pos == -1) goto POS03;
POS05:
    set @pos, list2(@list, 0, 1, "현철과 오독산이 있는가?", "네", "아니오");
    if(@pos == -1) goto POS04;
    else if(@list != 1) end;

    setarray @checkitem$[0], "현철", "오독산";
    for(set @i, 0; @i < 2; set @i, @i+1)
    {
        if(countitem(name2itemid(@checkitem$[@i])) == 0)
        {
            set @pos, dialog(0,0,0, "자네에겐 " + @checkitem$[@i] + "이(가) 없군.");
            deletearray @checkitem$[0], getarraysize(@checkitem$);
            end;
        }
    }
POS06:
    set @pos, dialog(0, 0, 1, "현철과 오독산을 가지고 있군.. 그럼 북천황을 부르겠네");
    if(@pos == 0) { deletearray @checkitem$[0], getarraysize(@checkitem$); end; }
POS07:
    mobspawn 1912, 10, 9, 2, 1051;
    for(set @i, 0; @i < 2; set @i, @i+1)
    {
        delitem @sd, name2itemid(@checkitem$[@i]), 1;
        message @sd, 3, @checkitem$[@i] + " 주었습니다";
    }
    deletearray @checkitem$[0], getarraysize(@checkitem$);
    set @pos, dialog(0, 0, 1, "지금 북천황이 나왔네! 빠른시간내에 처리해주게!");
    end;
}