{
    set @sd, mysd();
    image 1, 19, 0;
    if($nakrang4 == 1)
    {
        if(get_mapname(@sd) == "부여왕초보사냥1" || get_mapname(@sd) == "장훈왕초보사냥1" ||
        get_mapname(@sd) == "읍루왕초보사냥1" || get_mapname(@sd) == "만번한왕초보사냥1" ||
        get_mapname(@sd) == "천안궁왕초보사냥1" || get_mapname(@sd) == "상곡왕초보사냥1" ||
        get_mapname(@sd) == "어양왕초보사냥1" || get_mapname(@sd) == "관령왕초보사냥1" ||
        get_mapname(@sd) == "범안왕초보사냥1" || get_mapname(@sd) == "평양왕초보사냥1" ||
        get_mapname(@sd) == "졸본왕초보사냥1" || get_mapname(@sd) == "숙군왕초보사냥1" ||
        get_mapname(@sd) == "선양왕초보사냥1" || get_mapname(@sd) == "집안왕초보사냥1" ||
        get_mapname(@sd) == "산해왕초보사냥1" || get_mapname(@sd) == "신성왕초보사냥1" ||
        get_mapname(@sd) == "용성왕초보사냥1" || get_mapname(@sd) == "후연왕초보사냥1" ||
        get_mapname(@sd) == "개주왕초보사냥1" || get_mapname(@sd) == "동부여왕초보사냥1" ||
        get_mapname(@sd) == "송원왕초보사냥1" || get_mapname(@sd) == "고구려왕초보사냥1")
        {
COS001:
            set @pos, dialog(0, 0, 1, "잘 찾아오셨네요!!!");
            if(@pos == 0) end;

            set @pos, dialog(0, 0, 1, "잘 하셨습니다!! 상으로\\n노란비서를 드릴게요!!");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS001;

            delitem @sd, name2itemid("낙랑의두루마리4"), 1;
            message @sd, 3, "낙랑의두루마리4 주었습니다";
            additem @sd, name2itemid("낙랑의두루마리5"), 1;
            additem @sd, name2itemid("노란비서"), 10;
            if(get_level(@sd) < 5) 
            { 
                set @pos, dialog(0, 0, 1, "<보상>\\n'노란비서',\\n'낙랑의두루마리5'을 얻다!!!\\n경험치 100 상승!!");
                if(@pos == 0) end;

                giveexp @sd, 100;
            }
            else { set @pos, dialog(0, 0, 1, "<보상>\\n'노란비서',\\n'낙랑의두루마리5'을 얻다!!!"); if(@pos == 0) end; }

            set $nakrang4, 0;
        }
        else
        {
COS002:
            set @pos, dialog(0, 0, 1, "일단 주막에서 나오셔서,\\n화면의 왼쪽 위의 조그만\\n사각형 버튼을 누르시면\\n미니맵이 나옵니다.(단축키\\n*), 이것을 참고하셔서\\n'왕초보사냥터'로 찾아오세요.");
            if(@pos == 0) end;

            set @pos, dialog(0, 1, 1, "왕초보사냥터 안에 들어오셔서 두루마리를 사용하셔야 합니다.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
        }
    }
    else
    {
COS003:
        set @pos, dialog(1, 0, 1, "<임무>\\n 사냥터 찾아가기\\n\\n<내용>\\n 이제, 목도를 사용할 곳을 찾아가봅시다!!\\n본격적인 사냥을~~ 일단 주막에서 나오셔서,\\n화면의 왼쪽 위의 조그만 사각형 버튼을\\n누르시면 미니맵이 나옵니다.(단축키 *), 이것을\\n참고하셔서 '왕초보사냥터'로 찾아오세요.\\n\\n<보상>\\n 노란비서, 경험치 100 (레벨5미만)");
        if(@pos == 0) end;

        set @pos, dialog(1, 1, 1, "<힌트>\\n 집 내부에 있으면 미니맵이 안나오는 경우가\\n있습니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;

        set $nakrang4, 1;
    }
}