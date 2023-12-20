{
    set @sd, mysd();
    set @id, myid();

    if($hwagisamdongchung != 1)
    {
        set @pos, dialog(0, 0, 1, "안녕하시오. 장안성에는 무슨일로 오셨는가?");
        end;
    }
    if($donuhap == 0)
    {
COS001:
        set @pos, list2(@list, 0, 0, "자네는 왜 날 찾아왔나? 무슨 볼일이라도?", "화기삼동충초돈유합의 달인이라 하여 찾아왔습니다.", "아닙니다...지나가는 길입니다.");
        if(@pos == 0) end;
        else if(@list == 1)
        {
COS002:
            set @pos, dialog(0, 0, 1, "아하...그런것이라면 내가 도와줄 수 있지. 나도 공짜로는 도와줄수 없으니 내가 필요한 물건좀 구해다주게. 약재로 사용할 인삼과 동충하초가 급히 필요하거든. 부탁하네.");
            if(@pos == 0) end;
COS003:
            set @pos, dialog(0, 1, 1, "동충하초는 대방성입구에, 인삼은 국경지대를 뒤져보면 나올걸세.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;

            set $donuhap, 1;

            addlegend @sd, 31, 7, 0, 1, "돈유합의달인의 부탁을 들어주자. [1/2]";
            spellaether @sd, name2spellid("동충하초인삼"), 180;
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 1, "허허. 그러시오?");
            end;
        }
    }
    if($donuhap == 1)
    {
        if(countitem(name2itemid("동충하초")) == 0 || countitem(name2itemid("인삼")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 재료가 부족한것 같소. 동충하초는 대방성입구, 인삼은 국경지대를 뒤져보면 나올걸세.");
            end;
        }
COS004:
        set @pos, dialog(0, 0, 1, "허허 고맙구만. 이 재료들은 내가 잘 사용하도록 하지.");
        if(@pos == 0) end;
COS005:
        set @pos, dialog(0, 1, 1, "그럼 화기삼동충초돈유합의 재료를 알려주겠네. 비둘기고기,인삼,동충하초,구기자 이렇게 네가지의 재료가 필요하다네. 구해오면 내가 직접 요리해줄테니 재료를 찾아오게.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS004;
COS006:
        set @pos, dialog(0, 1, 1, "동충하초와 인삼은 어디서 구할 수 있는지 알고 있지? 비둘기고기와 구기자는 귀하기 때문에 아무곳에서나 팔지는 않고 중국 어딘가의 푸줏간에서 살 수 있을걸세.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS005;

        set $donuhap, 2;
        addlegend @sd, 31, 7, 0, 1, "돈유합의달인의 부탁을 들어주자. [2/2]";
        delitem @sd, name2itemid("동충하초"), 1;
        delitem @sd, name2itemid("인삼"), 1;
        message @sd, 3, "동충하초 주었습니다";
        message @sd, 3, "인삼 주었습니다";
        end;
    }
    if($donuhap == 2)
    {
        if(countitem(name2itemid("동충하초")) == 0 || countitem(name2itemid("인삼")) == 0 || countitem(name2itemid("비둘기고기")) == 0 || countitem(name2itemid("구기자")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 재료를 다 구하지 못했군. 재료는 동충하초,인삼,비둘기고기,구기자를 구해오게.");
            end;
        }
COS007:
        set @pos, dialog(0, 0, 1, "재료를 다 구해왔군. 내가 직접 요리해주겠네.");
        if(@pos == 0) end;
COS008:
        set @pos, dialog(0, 1, 1, "자~ 다됐네. 귀환 음식이니 좋은 곳에 쓰도록 하게.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS007;

        set $donuhap, 3;
        delitem @sd, name2itemid("동충하초"), 1;
        delitem @sd, name2itemid("인삼"), 1;
        delitem @sd, name2itemid("비둘기고기"), 1;
        delitem @sd, name2itemid("구기자"), 1;
        message @sd, 3, "동충하초 주었습니다";
        message @sd, 3, "인삼 주었습니다";
        message @sd, 3, "비둘기고기 주었습니다";
        message @sd, 3, "구기자 주었습니다";
        additem @sd, name2itemid("화기삼동충초돈유합"), 1;
        addlegend @sd, 31, 7, 0, 1, "돈유합의달인의 부탁을 들어주었다.";
        end;
    }
    if($donuhap == 3)
    {
        set @pos, dialog(0, 0, 1, "흐음..");
        end;
    }
}