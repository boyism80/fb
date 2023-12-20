{
    set @sd, mysd();
    set @id, myid();
COS001:
    set @pos, dialog(0, 0, 1, "고기나 생선말고는 건포로 만들기 어렵지...\\n\\n지네 같은 것은 말리면 부서지고 없어져서, 하기가 어려운데...");
    if(@pos == 0) end;

    set @pos, dialog(0, 1, 1, "자네가 가진 지네를 전부 주면 한번 시도해 봄세...");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS001;

    set @rand, rand(1, 100);
    set @count, countitem(name2itemid("죽은지네"));
    if(@count < 7)
    {
        set @pos, dialog(0, 0, 0, "지네가 너무 적어서 만들수 없을 것 같네..");
        end;
    }
    else
    {
        if(@count >= 7 && @count < 20) { set @rate, 30; }
        else if(@count >= 20 && @count < 50) { set @rate, 50; }
        else if(@count >= 50 && @count < 80) { set @rate, 70; }
        else if(@count >= 80 && @count <= 100) { set @rate, 90; }

        if(@rate < 50) { set @rate, 50; }
        else if(@rate > 100) { set @rate, 100; }

        delitem @sd, name2itemid("죽은지네"), @count;
        message @sd, 3, "죽은지네 주었습니다";

        if(@rand < @rate)
        {
            additem @sd, name2itemid("말린지네"), 1;
            set @pos, dialog(0, 0, 1, "이거 쉽지 않구만... 간신히 만들었네... 자주 들리게나...");
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 0, "이런... 지네가 모두 부숴저버렸군..");
            end;
        }
    }
}