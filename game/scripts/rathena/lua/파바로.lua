{
    set @sd, mysd();
    set @id, myid();

COS001:
    set @pos, dialog(0, 0, 1, "구워먹기 좋은 음식은 내가 맛있게 금방 구워 주지. 대신 내게도 맛 좀 보게 해 주면 좋겠군. 음식 하나를 구워주면 수수료로 하나는 내가 먹겠어.");
    if(@pos == 0) end;
COS002:
    set @pos, list2(@list, 0, 0, "내가 전에 구워봤던 건 이 정도야. 굽고 싶은 음식이 있으면 골라보게. 나중에 또 구워봐서 성공하는 음식이 있으면 목록에 추가하도록 하지.", "감자", "고구마", "늑대고기", "나중에 다시 올께요.");
    if(@pos == 0) end;
    else if(@list >= 1 && @list <= 3)
    {
        setarray @item$[1], "감자", "고구마", "늑대고기";

        set @pos, dialog(0, 0, 1, @item$[@list] + "! 그거 좋지. 감자는 구우면 씹기도 좋고, 고소한 맛이 더 살아난다네.");
        if(@pos == 0) { goto failed; end; }

        set @pos, dialog(0, 0, 1, "자 그럼 어디 구워 볼까!");
        if(@pos == 0) { goto failed; end; }

        if(countitem(name2itemid(@item$[@list])) < 2)
        {
            set @pos, dialog(0, 0, 0, @item$[@list] + "가 부족한 것 같은데?");
            goto failed;
            end;
        }
        else
        {
            delitem @sd, name2itemid(@item$[@list]), 2;
            message @sd, 3, @item$[@list] + " 주었습니다";
            additem @sd, name2itemid("구운" + @item$[@list]), 1;
            goto success;
            end;
        }
    }
    else end;
    end;

success:
    deletearray @item$[1], getarraysize(@item$);
    set @pos, dialog(0, 0, 1, "자, 다 됐네. 김이 모락모락 나는 것이 아주 잘 구워진 것 같군! 약속대로 내가 하나 먹고, 하나는 자네가 먹게나.");
    end;
failed:
    deletearray @item$[1], getarraysize(@item$);
}