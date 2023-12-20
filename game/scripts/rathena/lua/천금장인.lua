{
    set @sd, mysd();
    set @id, myid();
a1:
    set @pos, list2(@list, 0, 0, "안녕하세요. 어떻게 오셨나요?", "하급 용무기 각성");
    if(@pos == 0) end;
    else if(@list == 1)
    {
a2:
        set @pos ,dialog(1, 1, 1, "흐음.. 자네는 전설의 무기인 용무기를\\n각성시키고자 하는가?\\n\\n그렇다면 잘 찾아왔네.\\n\\n허나, 7등급 이상의 상급 용무기를 각성시키려 한다면 전설의 대장장이로 일컬어지는 백의장인을 찾아야 한다네.\\n\\n나에게 하급 용무기 각성을 가르쳐주신 위대한 분이시지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto a1;
a3:
        set @pos, dialog(1, 1, 1, "용무기는 각성을 통해 한 단계씩 성장하게 된다는 사실을 자네는 이미 알고 있는 듯 하군.\\n\\n하지만 이는 무척 시도하기 쉽지 않은 일이라네.\\n\\n만약 실패할 경우 그 귀한 용무기가 부숴저버릴수도 있으니..");
        if(@pos == 0) end;
        else if(@pos == -1) goto a2;
a4:
        set @pos, dialog(1, 1, 1, "7등급 이상의 완숙한 용무기들은 각성이 실패해도 부숴지는 일은 없네만, 손상을 입어 더이상 각성이 불가능한 상태가 되어버리지.\\n손상된 용무기는 신수성에서의 수리비도 5배나 더 비싸진다네.\\n\\n게다가 각성을 위해서는 특별한 재료를 필요로 하지.\\n용무기는 지극히 높은 온도에서 제련을 통해 발현된 용의 기운을 타고 각성하게 되는데, 충분한 온도를 내기 위해선 '은나무가지'가 필요하다네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto a3;
a5:
        set @pos, list2(@list, 1, 1, "사람이 하는 일에 완전은 있을 수 없다네. 용의 기운은 미묘하여, 하늘, 땅, 그리고 사람의 기운중 하나가 조금이라도 어긋나면 용무기는 깨져버릴 것이네. 자네는 실패의 위험을 무릎쓰고서 용무기를 각성시키려는가?", "예, 진인사재천명! 하늘에 뜻에 맡기겠습니다.", "아니요, 내 귀한 용무기를 시험에 들게할 순 없죠.");
        if(@pos == 0) end;
        else if(@pos == -1) goto a4;
        else if(@list == 1)
        {
a6:
            set @pos, list2(@list1, 0, 1, "좋아 각오가 대단하군.\\n그럼 어떤 용무기를 각성시키고자 하는가?", "용마", "용천", "용랑", "용겸");
            if(@pos == 0) end;
            else if(@pos == -1) goto a5;
            else if(@list1 == 1)
            {
                set @pos, list2(@list2, 0, 1, "각성시킬 용무기를 고르게", "용마제일검", "용마제이검", "용마제삼검", "용마제사검", "용마제오검");
                if(@pos == 0) end;
                else if(@pos == -1) goto a6;
                else if(@list2 > 0)
                {
                    setarray @success_item$[1], "용마제이검", "용마제삼검", "용마제사검", "용마제오검", "용마제육검";
                    setarray @check_item$[1], "용마제일검", "용마제이검", "용마제삼검", "용마제사검", "용마제오검";
                }
            }
            else if(@list1 == 2)
            {
                set @pos, list2(@list2, 0, 1, "각성시킬 용무기를 고르게", "용천제일검", "용천제이검", "용천제삼검", "용천제사검", "용천제오검");
                if(@pos == 0) end;
                else if(@pos == -1) goto a6;
                else if(@list2 > 0)
                {
                    setarray @success_item$[1], "용천제이검", "용천제삼검", "용천제사검", "용천제오검", "용천제육검";
                    setarray @check_item$[1], "용천제일검", "용천제이검", "용천제삼검", "용천제사검", "용천제오검";
                }
            }
            else if(@list1 == 3)
            {
                set @pos, list2(@list2, 0, 1, "각성시킬 용무기를 고르게", "용랑제일봉", "용랑제이봉", "용랑제삼봉", "용랑제사봉", "용랑제오봉");
                if(@pos == 0) end;
                else if(@pos == -1) goto a6;
                else if(@list2 > 0)
                {
                    setarray @success_item$[1], "용랑제이봉", "용랑제삼봉", "용랑제사봉", "용랑제오봉", "용랑제육봉";
                    setarray @check_item$[1], "용랑제일봉", "용랑제이봉", "용랑제삼봉", "용랑제사봉", "용랑제오봉";
                }
            }
            else if(@list1 == 4)
            {
                set @pos, list2(@list2, 0, 1, "각성시킬 용무기를 고르게", "용겸제일봉", "용겸제이봉", "용겸제삼봉", "용겸제사봉", "용겸제오봉");
                if(@pos == 0) end;
                else if(@pos == -1) goto a6;
                else if(@list2 > 0)
                {
                    setarray @success_item$[1], "용겸제이봉", "용겸제삼봉", "용겸제사봉", "용겸제오봉", "용겸제육봉";
                    setarray @check_item$[1], "용겸제일봉", "용겸제이봉", "용겸제삼봉", "용겸제사봉", "용겸제오봉";
                }
            }
        }
        else if(@list == 2)
        {
            set @pos, dialog(1, 0, 0, "그래, 자네 판단이 맞을수도 있을걸세.\\n\\n나도 나 자신을 믿을수 없으니말일세.\\n\\n그럼 가보게나.");
            end;
        }
    }
    setarray @success_rand[1], 85, 70, 50, 40, 25;
    if(countitem(name2itemid("은나무가지")) == 0 || countitem(name2itemid(@check_item$[@list2])) == 0)
    {
        set @pos, dialog(0, 0, 0, "자네는 아직 용무기를 각성시킬 준비가 완벽하지 못하군.");
    }
    else
    {
        if(rand(1, 100) <= @success_rand[@list2])
        {
            broadcast 5, get_name(@sd) + "님이 " + @check_item$[@list2] + " 강화에 성공하셨습니다.";
            delitem @sd, name2itemid("은나무가지"), 1;
            delitem @sd, name2itemid(@check_item$[@list2]), 1;
            message @sd, 3, "은나무가지 주었습니다";
            message @sd, 3, @check_item$[@list_2] + " 주었습니다";
            additem @sd, name2itemid(@success_item$[@list2]), 1;
            set @pos, dialog(0, 0, 1, "축하하네. 하늘이 자네를 " + @success_item$[@list2] + "의 주인으로 인정하는군.");
        }
        else
        {
            broadcast 5, get_name(@sd) + "님이 " + @check_item$[@list2] + " 강화에 실패하셨습니다.";
            delitem @sd, name2itemid("은나무가지"), 1;
            delitem @sd, name2itemid(@check_item$[@list2]), 1;
            message @sd, 3, "은나무가지 주었습니다";
            message @sd, 3, @check_item$[@list_2] + " 주었습니다";
            set @pos, dialog(0, 0, 0, "미안하네. 온도 조절이 실패하여 용무기가 부숴저버렸군.. 너무 낙심하지 말게나.");
        }
        
    }
    deletearray @success_item$[1], getarraysize(@success_item$);
    deletearray @check_item$[1], getarraysize(@check_item$);
    deletearray @success_rand[1], getarraysize(@success_rand);
}