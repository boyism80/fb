{
    set @sd, mysd();
    set @id, myid();

    if($alcoholic_drink == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "안녕하시오. " + get_name(@sd) + "씨, 처음 뵙겠소. 난 여기 남양에서 뱀술을 만드는 주경원이라고 하오.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "나는 20년동안 뱀술을 만들면서 살아왔지. 우리 집안 대대로 전해지는 특별한 방법으로 만드는데, 많은 재료가 필요하지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set @pos, dialog(0, 1, 1, "그런데 요즘은 남양에도 괴물들이 많아서 땅꾼들이 뱀을 잡을 수가 없단 말이네. 뱀술을 만드는 게 내 유일한 낙인데...");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;

        set @pos, list2(@list, 0, 0, "뱀이 필요한데.. 어떻게 구한담?", "제가 도와드리죠. 뱀을 구해 드리면 되는 건가요?", "뱀술말고 다른 술을 만드시면 되지 않나요?");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $alcoholic_drink, 1;
            addlegend @sd, 40, 7, 0, 20, "청심사주의 재료를 구하자.[1/2]";
            set @pos, dialog(0, 0, 1, "아아! 고맙네 고마워. 재료를 구해주면 내가 만든 술을 한병 주겠네.\\n\\n내가 만들려는 것은 그냥 뱀술이 아니고, '청심사주'라는 것이지. 보통 뱀술도 몸에 좋다고 하지만, 청심사주에 비할바는 아니야. 뱀고기 100개와 좋은뱀고기 5개를 농축하여 만든다네.");
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그건 용납할 수 없네! 나는 오로지 뱀술만을 만들 것이네!");
            end;
        }
    }
    if($alcoholic_drink == 1)
    {
        if(countitem(name2itemid("뱀고기")) < 100 && countitem(name2itemid("좋은뱀고기")) < 5)
        {
            set @pos, dialog(0, 0, 0, "청심사주를 만들기 위해선 뱀고기 100개와 좋은뱀고기 5개가 필요하네.");
            end;
        }
        else
        {
COS003:
            set @pos, dialog(0, 0, 1, "오오, 뱀고기 100개와 좋은뱀고기 5개를 정말 구해왔군. 고맙네 고마워.. 이제 뱀술을 한 번 만들어 보겠네.");
            if(@pos == 0) end;

            set @pos, dialog(0, 1, 1, "........\\n\\n아아 이럴수가... 오랫동안 뱀술을 만들지 못해 얼마간 재료 살피는 것을 소홀히 했더니 재료가 몽땅 못쓰게 되었군. 청심사주엔 뱀만 필요한 것이 아니거든.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS003;

            set @pos, list2(@list, 0, 0, "이 일을 어쩐다....", "내친김에 나머지 재료도 제가 구해다 드리지요.", "에이, 이젠 저도 몰라요.");
            if(@pos == 1 && @list == 1)
            {
                set $alcoholic_drink, 2;
                addlegend @sd, 40, 7, 0, 20, "청심사주의 재료를 구하자.[2/2]";

                spellaether @sd, name2spellid("청심초"), 180;

                delitem @sd, name2itemid("뱀고기"), 100;
                delitem @sd, name2itemid("좋은뱀고기"), 5;
                message @sd, 3, "뱀고기 주었습니다";
                message @sd, 3, "좋은뱀고기 주었습니다";
COS004:
                set @pos, dialog(0, 0, 1, "아아 고맙네, 그럼 일단 뱀고기는 내가 가져가겠네..\\n\\n나머지 재료는 기린의피와 청심초 5뿌리라네. 기린의피는 붉은색의 기린을 잡아서 피를 받아오면 되네.");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "청심초는 남경지역을 한참 돌아다니다 보면 운이 좋은 사람에게 발견된다네.\\n\\n기린의피와 청심초 5뿌리만 있으면 이제 정말 만들 수 있을 것이네.");
                if(@pos == -1) goto COS004;
                end;
            }
            else end;
            end;
        }
    }
    if($alcoholic_drink == 2)
    {
        if(countitem(name2itemid("기린의피")) == 0 && countitem(name2itemid("청심초")) < 5)
        {
            set @pos, dialog(0, 0, 0, "재료가 모자라군.. 기린의피와 청심초 5뿌리만 있다면 이젠 정말 만들수 있을 것이네.");
            end;
        }
        else
        {
COS005:
            set @pos, dialog(0, 0, 1, "오오, 고맙네 고마워.. 내 뱀술을 만들어 줄 테니 잠시만 기다리게!");
            if(@pos == 0) end;

            set @pos, dialog(0, 1, 1, "... ... ...\\n\\n다 됐네! 한 병은 평소에 신세가 많았던 천선도사에게 좀 가져다 드리고, 한병은 자네가 마시게. 허허허.. 정말 고마우이..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS005;

            delitem @sd, name2itemid("기린의피"), 1;
            delitem @sd, name2itemid("청심초"), 5;
            message @sd, 3, "기린의피 주었습니다";
            message @sd, 3, "청심초 주었습니다";

            set $alcoholic_drink, 3;
            additem @sd, name2itemid("청심사주"), 2;
            addlegend @sd, 40, 7, 0, 20, "청심사주를 천선도사에게 전해주자.";
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "어떤가? 청심사주의 맛이 아주 기가 막히지 않은가?");
        end;
    }
}