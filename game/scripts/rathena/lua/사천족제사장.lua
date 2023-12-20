{
    set @sd, mysd();
    set @id, myid();
    set @nt, callfunc("Server_Nowtime");

    if(get_level(@sd) < 99)
    {
        set @pos, dialog(0, 0, 0, "99레벨 이상만 이용이 가능합니다.");
        end;
    }

    if($holytree == 0)
    {
        if($holytreetime < @nt)
        {
COS001:
            set @pos, dialog(1, 0, 1, "최근 몇 년간 흉년이 계속되고 있어 마을을 위한 제를 지내기 위해 신성한 나무 가지를 구하고 있습니다.\\n\\n북방대초원에는 백현목,자현목,노송의가지,천년묵은가지와 같이 신성한 기운이 깃든 나무 가지들을 구할 수 있는데,\\n매일 매일 제를 지내야 하다 보니 많은 양의 나무 가지가 필요합니다.");
            if(@pos == 0) end;
COS002:
            set @pos, list2(@list, 1, 1, "저에게 신성한 기운이 깃든 나무를 가져다 주실 수 있을까요?", "네. 구해보도록 할께요.", "아니요. 제가 좀 바빠서.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS001;
            else if(@list == 1)
            {
                set $holytree, 1;
                set @pos, dialog(1, 0, 1, "신성한 나무들은 북방대초원 21~30층을 돌아다니다 보면 구할 수 있을겁니다.\\n\\n그럼 부탁드리겠습니다.");
                end;
            }
            else end;
        }
        else
        {
            set @pos, dialog(0, 0, 0, "" + ($holytreetime-@nt) + "초 후에 다시 오세요.");
            end;
        }
    }
    if($holytree == 1)
    {
COS003:
        set @pos, list2(@list_2, 1, 0, "이렇게 열심히 제를 올리는데 언제쯤 흉년이 멈추려나..", "백현목을 가지고 왔습니다.", "자현목을 가지고 왔습니다.", "노송의가지를 가지고 왔습니다.", "천년묵은가지를 가지고 왔습니다.");
        if(@pos == 0) end;
        else if(@list_2 > 0)
        {
            setarray @item$[1], "백현목", "자현목", "노송의가지", "천년묵은가지";
            setarray @message$[1], "을", "을", "를", "를";
            setarray @message2$[1], "이", "이", "가", "가";
            setarray @exp[1], 926000000, 1353000000, 1550000000, 2100000000;
            set @pos, list2(@list_3, 1, 0, "풍년을 위한 제를 올릴 수 있도록 저에게 " + @item$[@list_2] + "" + @message$[@list_2] + " 주시겠습니까?", "네. 드릴께요.", "아니요. 제가 쓸데가 있어서요.");
            if(@pos == 0) { goto failed; end; }
            else if(@list_3 == 1)
            {
                if(countitem(name2itemid(@item$[@list_2])) == 0)
                {
                    set @pos, dialog(0, 0, 0, @item$[@list_2] + "" + @message2$[@list_2] + " 없는데요?");
                }
                else
                {
                    if(party(@sd) == 0)
                    {
                        delitem @sd, name2itemid(@item$[@list_2]), 1;
                        message @sd, 3, @item$[@list_2] + " 주었습니다";

                        set @giveexp, @exp[@list_2]/#server_exprate;
                        giveexp @sd, @giveexp;

                        set @nt, callfunc("Server_Nowtime");
                        set $holytreetime, @nt+86400;
                        set $holytreecount, $holytreecount+1;
                        set $holytree, 0;

                        deletearray @item$[1], getarraysize(@item$);
                        deletearray @message$[1], getarraysize(@message$);
                        deletearray @message2$[1], getarraysize(@message2$);
                        deletearray @exp[1], getarraysize(@exp);

                        addlegend @sd, 5, 7, 0, 15, "신성한나무를 " + $holytreecount + "회 가져다 주었다.";
                        set @pos, dialog(1, 0, 0, "감사합니다.\\n\\n하루 빨리 풍년이 찾아 올 수 있도록 몸과 마음을 다해 제를 올려야겠습니다.\\n\\n계속해서 잘 부탁드립니다.");
                    }
                    else
                    {
                        deletearray @item$[1], getarraysize(@item$);
                        deletearray @message$[1], getarraysize(@message$);
                        deletearray @message2$[1], getarraysize(@message2$);
                        deletearray @exp[1], getarraysize(@exp);
                        set @pos, dialog(0, 0, 1, "그룹을 하신 상태로는 보상을 받을 수 없습니다.");
                    }
                }
                deletearray @item$[1], getarraysize(@item$);
                deletearray @message$[1], getarraysize(@message$);
                deletearray @message2$[1], getarraysize(@message2$);
                deletearray @exp[1], getarraysize(@exp);
            }
            else { goto failed; end; }
        }
        else end;
    }
failed:
    deletearray @item$[1], getarraysize(@item$);
    deletearray @message$[1], getarraysize(@message$);
    deletearray @message2$[1], getarraysize(@message2$);
    deletearray @exp[1], getarraysize(@exp);
}