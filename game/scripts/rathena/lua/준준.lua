{
    set @sd, mysd();
    set @id, myid();

    if($junjun == 0)
    {
        set @pos, list2(@list, 0, 0, "일은 열심히 하고 있는지 모르겠군. 덥겠지만, 분발해주게나!", "토템을 하나 찾아왔습니다", "보상을 요구한다.", "그냥 들려본건데요.");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set @menu, list1("그래..어떤 토템을 가지고 왔는가?", "번개의토템", "바람의토템", "대지의토템", "화염의토템");
            if(@menu > 0)
            {
                setarray @item$[1], "번개의토템", "바람의토템", "대지의토템", "화염의토템";
                if(countitem(name2itemid(@item$[@menu])) == 0)
                {
                    set @pos, dialog(0, 0, 0, @item$[@menu] + "이 없는데?");
                }
                else
                {
                    delitem @sd, name2itemid(@item$[@menu]), 1;
                    message @sd, 3, @item$[@menu] + " 주었습니다";
                    set $totem_count, $totem_count+1;
                    set @pos, dialog(0, 0, 1, "수고했네. 계속 힘내주게.");
                }
                deletearray @item$[1], getarraysize(@item$);
                end;
            }
            else end;
        }
        else if(@list == 2)
        {
            if($totem_count < 5)
            {
                set @pos, dialog(0, 0, 0, "아직 한 일이 별로 없는거 같은데? 토템을 적어도 5번은 가져와야 보상을 줄 수 있네.");
                end;
            }
            else
            {
                additem @sd, name2itemid("선장의일기5"), 1;
                addlegend @sd, 32, 7, 0, 1, "진백랑의 부탁을 들어주자. [6/6]";
                set $junjun, 1;
                set @pos, dialog(0, 0, 1, "수고했네. 계속 힘내주게.");
                end;
            }
        }
        else end;
    }
    else
    {
        set @pos, dialog(0, 0, 1, "일은 정말 힘들군!");
        end;
    }
}