{
    set @sd, mysd();
    set @id, myid();
COS001:
    set @pos, dialog(0, 0, 1, "일본까지 가시려고? 내 표한장 드릴까? 싸게해드릴께..");
    if(@pos == 0) end;
COS002:
    set @pos, list2(@list, 0, 1, "부여-일본쾌속승선권 한장 사시려나?", "네, 한장 주세요.", "혹시 사기꾼 아냐?");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS001;
    else if(@list == 1)
    {
        set @pos, list2(@list_2, 0, 0, "부여-일본쾌속승선권.. 한장에 8000원에 드리지. 어때?", "8000원에 사죠.", "너무 비싼데...");
        if(@pos == 0) end;
        else if(@list_2 == 1)
        {
            set @pos, dialog(0, 0, 1, "그래? 그럼 표 한장 드릴께..");
            if(@pos == 0) end;

            if(get_money(@sd) < 8000)
            {
                set @pos, dialog(0, 0, 1, "이 사람이! 돈이 없잖아 돈이!");
                end;
            }
            else
            {
                additem @sd, name2itemid("부여-일본쾌속승선권"), 1;
                set_money @sd, get_money(@sd)-8000;
                updatestatus @sd, 2;
                set @pos, dialog(0, 0, 1, "그럼 일본 잘갔다와~ 나중에 또 보자고..");
                end;        
            }
        }
        else if(@list_2 == 2)
        {
COS003:
            set @pos, dialog(0, 0, 1, "너무 비싸다고? 얼마까지 알아보고 왔어? 에이.. 하나도 안 남는데..");
            if(@pos == 0) end;
COS004:
            set @pos, dialog(0, 1, 1, "에이.. 좋다. 그럼 내 노란비서 하나 끼워 준다. 그럼 됐지?");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS003;
COS005:
            set @pos, list2(@list_3, 0, 1, "살꺼지?" ,"그러죠 그럼..", "안살래요.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS004;
            else if(@list_3 == 1)
            {
COS006:
                set @pos, dialog(0, 0, 1, "그래그래. 잘 생각한거야. 그럼 볼까나....");
                if(@pos == 0) end;

                if(get_money(@sd) < 8000)
                {
                    set @pos, dialog(0, 0, 1, "이 사람이! 돈이 없잖아 돈이!");
                    end;
                }
                else
                {
                    additem @sd, name2itemid("노란비서"), 1;
                    additem @sd, name2itemid("부여-일본쾌속승선권"), 1;
                    set_money @sd, get_money(@sd)-8000;
                    updatestatus @sd, 2;
                    set @pos, dialog(0, 0, 1, "그럼 일본 잘갔다와~ 나중에 또 보자고..");
                    end;
                }
            }
            else if(@list_3 == 2)
            {
                set @pos, dialog(0, 0, 1, "관심없음 그냥 가구.. 다른데 알아봐. 더 싼데 있나..");
                end;
            }
            else end;
            

        }
        else end;
    }
    else if(@list == 2)
    {
        set @pos, dialog(0, 0, 1, "어허.. 이 사람 보게? 안사면 안사는거지 왜 사람을 사기꾼으로 몰고 그래!");
        if(@pos == 0) end;        
    }
    else end;
}