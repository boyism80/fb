{
    image 0, 87, 3;
    set @sd, mysd();
    set @id, myid();
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "PK12지입장", "신부여성입장");
    if(@ret == 1)
    {
POS000:
        set @pos, dialog(0, 0, 1, "그래, 여길 들어가고 싶단 말이지?");
        if(@pos == 0) end;
POS001:
        set @pos, dialog(0, 1, 1, "이 안은 아주 위험하다네...");
        if(@pos == 0) end;
        else if(@pos == -1) goto POS000;
POS002:
        set @pos, dialog(0, 1, 1, "폭탄으로 죽을 수도 있고, 사용자들끼리도 서로 죽일 수 있다네...");
        if(@pos == 0) end;
        else if(@pos == -1) goto POS001;
POS003:
        set @pos, dialog(0, 1, 1, "이 안에서 죽어서 아이템이나 경험치를 잃어버린 것은 자네가 책임져야 하네...");
        if(@pos == 0) end;
        else if(@pos == -1) goto POS002;
        set @pos, list2(@ret, 0, 1, "아주 위험한 곳인데, 그래도 들어가길 원하는가?", "네. 들어가길 원합니다.", "아니오. 다시 생각하니 겁나는군요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto POS003;
        if(@ret == 1)
        {
POS0000:
            set @pos, dialog(0, 0, 1, "각오가 대단한가보군... 마지막으로 묻겟네...");
            if(@pos == 0) end;
POS0001:
            set @pos, dialog(0, 1, 1, "이 안에서 서로 죽이거나 죽어서 생긴 모든 문제는 자네 책임일세...");
            if(@pos == 0) end;
            else if(@pos == -1) goto POS0000;
POS0002:
            set @pos, dialog(0, 1, 1, "죽어서 아이템이나 경험치를 잃은 것도 모두 자네 책임이고 복구는 안된다네...");
            if(@pos == 0) end;
            else if(@pos == -1) goto POS0001;
            set @pos, list2(@ret, 0, 1, "여기에 동의하는가?", "네. 모두 동의합니다.", "아니오. 다시 생각하니 겁나는군요.");
            if(@pos == 0) end;
            if(@ret == 1)
            {
                set @pos, dialog(0, 0, 1, "그럼, 행운을 비네... 조심하게..");
                if(@pos == 0) end;

                warp @sd, 1948, 25+rand(1, 10), 12+rand(1, 5);
            }
            else if(@ret == 2) goto POS0;
        }
        else if(@ret == 2)
        {
            set @pos, dialog(0, 0, 0, "잘 생각 했네. 잘가게...");
            end;
        }
        else end;
    }
    else if(@ret == 2)
    {
COS000:
        set @pos, dialog(0, 0, 1, "그래, 무슨일이지?");
        if(@pos == 0) end;
COS001:
        set @pos, dialog(0, 1, 1, "신부여성으로 가고 싶다고?");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS000;
COS002:
        set @pos, dialog(0, 1, 1, "신부여성은 참 좋은곳이지..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "신부여성에서는 많은 곳을 여행할 수 있다네..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
        set @pos, list2(@ret, 0, 1, "정말 신부여성으로 가겠는가?", "네. 보내주세요.", "아니오. 가기싫어요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;
        if(@ret == 1)
        {
COS0000:
            set @pos, dialog(0, 0, 1, "정말로 신부여성으로 가겠는가?");
            if(@pos == 0) end;
COS0001:
            set @pos, dialog(0, 1, 1, "이 안에서 길을 잃어도 모든 책임은 자네것이네..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS0000;
COS0002:
            set @pos, dialog(0, 1, 1, "정말로 가길 원한다면 보내주도록 하겠네");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS0001;
            set @pos, list2(@ret, 0, 1, "정말로 가겠는가?", "네. 꼭 가고싶어요.", "아니오. 다시 생각해볼게요.");
            if(@pos == 0) end;
            if(@ret == 1)
            {
                set @pos, dialog(0, 0, 1, "그럼, 행운을 비네...");
                if(@pos == 0) end;

                set @rand, rand(1, 3);
                if(@rand == 1) { set @y, 5; }
                else if(@rand == 2) { set @y, 8; }
                else { set @y, 11; }

                warp @sd, rand(15205, 15206), rand(3, 4), @y;
            }
            else if(@ret == 2) goto COS0;
        }
        else if(@ret == 2)
        {
            set @pos, dialog(0, 0, 0, "잘 생각 했네. 잘가게...");
            end;
        }
        else end;
    }
    else end;
}