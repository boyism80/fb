{
    set @sd, mysd();
    set @id, myid();

    if($hwagisamdongchung == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "아.. 이 근처에 있다고 했는데... 어디있는 것일까...");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "아, 반갑네 내가 지금 뭘 좀 찾고 있느라 말이지. 실은 아내가 아이를 가졌는데 화기삼동초돈유합을 너무 먹고 싶다고 하는구만.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "그래서 여기저기서 전해지는 요리법으로 내가 한번 만들어보려고 하는데 재료조차 구하기가 어렵군.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, list2(@list, 0, 1, "날 좀 도와주지 않겠는가?", "물론입니다. 부인의 몸조리에나 신경쓰십시오.", "죄송합니다만 그런일까지 할 시간이 없군요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;
        else if(@list == 1)
        {
            set $hwagisamdongchung, 1;
            addlegend @sd, 31, 7, 0, 1, "조왕의동생의 부탁을 들어주자.";
            set @pos, dialog(0, 0, 1, "그럼 자네만 믿고 있겠네. 장안성의 푸줏간에 그 음식에 대해 잘 알고 있는 사람이 있다는 얘길 들은적이 있는데.. 혹시 참고가 될지 모르겠군.");
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그렇다면 할 수 없군요..");
            end;
        }
    }
    if($hwagisamdongchung == 1)
    {
        if(countitem(name2itemid("화기삼동충초돈유합")) == 0)
        {
COS005:
            set @pos, dialog(0, 0, 1, "아직 화기삼동충초돈유합을 구하지 못하셨나보군요.");
            if(@pos == 0) end;
COS006:
            set @pos, dialog(0, 1, 1, "장안성의 푸줏간에 그 음식에 대해 잘 알고 있는 사람이 있다네. 참고가 될지 모르겠군.");
            if(@pos == -1) goto COS005;
            end;
        }
COS007:
        set @pos, dialog(0, 0, 1, "오! 드디어 구해왔군! 수고햇네. 어려운 부탁을 들어줘서 정말 고맙네. 이건 내 작은 정성이니 받아주게.");
        if(@pos == 0) end;
COS008:
        set @pos, dialog(0, 1, 1, "이걸 먹으면 자네의 체력이 더 강해질 걸세! 그럼 고맙네.. 잘가게나.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS007;

        set $hwagisamdongchung, 2;
        delitem @sd, name2itemid("화기삼동충초돈유합"), 1;
        message @sd, 3, "화기삼동충초돈유합 주었습니다";
        additem @sd, name2itemid("자양강장요리"), 1;
        addlegend @sd, 31, 6, 0, 1, "화기삼동충초돈유합을 전해주었다.";
        end;
    }
    if($hwagisamdongchung == 2)
    {
        set @pos, dialog(0, 0, 1, "저번엔 정말 감사했소!");
        end;
    }
}