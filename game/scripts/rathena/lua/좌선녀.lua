{
    set @sd, mysd();
    set @id, myid();

    if(get_level(@sd) < 36)
    {
        set @pos, dialog(0, 0, 0, "용왕님의 건강이 많이 나빠지셔서 어떡하면 좋을까..");
        end;
    }

    if($water_ring == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "안녕하세요? " + get_name(@sd) + "님, 전 용왕님의 식사를 담당하고 있는 좌선녀라해요");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "근래 용왕님께서 여러 어려운 일을 처리하시느라 몸이 상당히 나빠지셨어요");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "그래서 제가 용왕님의 건강에 도움이 될 천상보강탕을 해 드릴려고 하는데 재료가 부족하네요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, list2(@list, 0, 0, "제가 부탁하는 재료를 구해주실 수 있나요?", "아.물론입니다.", "죄송하지만 바빠서...");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $water_ring, 1;
            addlegend @sd, 18, 7, 0, 1, "좌선녀의 부탁을 들어주자.";
COS005:
            set @pos, dialog(0, 0, 1, get_name(@sd) + "님 정말 감사합니다. 부탁드릴 재료는 [게등껍질][게집게][문어다리][해마꼬리]랍니다.");
            if(@pos == 0) end;
COS006:
            set @pos, dialog(0, 1, 0, "그럼 부탁드리겠습니다.");
            if(@pos == -1) goto COS005;
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "흠..그러신가요..");
            end;
        }
        else end;
    }
    else
    {
        if(countitem(name2itemid("게등껍질")) == 0 || countitem(name2itemid("게집게")) == 0 || countitem(name2itemid("문어다리")) == 0 || countitem(name2itemid("해마꼬리")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 재료를 다 모으지 못하신 것 같군요.. 재료는 [게등껍질][게집게][문어다리][해마꼬리]랍니다.");
            end;
        }

        set $water_ring, 0;
        set $water_ring_sucess, 1;

        setarray @item$[0], "게등껍질", "게집게", "문어다리", "해마꼬리";
        for(set @loop, 0; @loop < 4; set @loop, @loop+1)
        {
            delitem @sd, name2itemid(@item$[@loop]), 1;
        }
        for(set @loop, 0; @loop < 4; set @loop, @loop+1)
        {
            message @sd, 3, @item$[@loop] + " 주었습니다";
        }
        deletearray @item$[0], getarraysize(@item$);
        additem @sd, name2itemid("인어반지"), 1;
        addlegend @sd, 18, 7, 0, 1, "좌선녀의 부탁을 들어주었다.";
COS007:
        set @pos, dialog(0, 0, 1, "어머, 재료를 벌써 다 모아 오셨군요. 정말 감사드려요.");
        if(@pos == 0) end;
COS008:
        set @pos, dialog(0, 1, 1, "이 인어반지는 제 성의에요. 예전 인어장군님께 선물을 받은 것인데 공좌님의 성의에 꼭 보답하고 싶어서요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS007;
COS009:
        set @pos, dialog(0, 1, 0, "공좌님 정말 감사드려요.");
        if(@pos == -1) goto COS008;
    }
}