{
    set @sd, mysd();
    set @id, myid();

    if($tangtang == 0)
    {
        set @pos, dialog(0, 0, 1, "요즘 대륙 사람들이 자꾸 눈에 띄눈군. 정말 성가신 일이야. 그러고보니 자네도 대륙 사람이지? 그렇다면 태존도에 대해 알겠군?");
        if(@pos == 0) end;

        set @pos, list2(@list, 0, 0, "다른 나라의 무기를 연구중인데, 그 태존도가 좀 필요해. 구해다주겠나?", "네, 구해다 드리겠습니다.", "싫습니다.");
        if(@pos == 0) end;
        else if(@list != 1) end;

        set $tangtang, 1;
        set @pos, dialog(0, 0, 1, "나름대로 보답은 하겠어. 좀 서둘러만 줬음 좋겠군.");
        end;
    }
    if($tangtang == 1)
    {
        if(countitem(name2itemid("태존도")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 태존도를 구하지 못한건가?");
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "음, 그게 태존도인가? 이리 주시게.");
            if(@pos == 0) end;

            set $tangtang, 2;
            delitem @sd, name2itemid("태존도"), 1;
            message @sd, 3, "태존도 주었습니다";
            additem @sd, name2itemid("선장의일기4"), 1;
            addlegend @sd, 32, 7, 0, 1, "진백랑의 부탁을 들어주자. [5/6]";
            set @pos, dialog(0, 0, 1, "고맙다는 말은 하지 않겠네. 이건 거래니까. 자, 가져가게");
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "태존도.. 정말 신기한 무기군.");
        end;
    }
}