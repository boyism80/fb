{
    set @sd, mysd();
    set @id, myid();

    if($totem_clothes == 0)
    {
        set @pos, dialog(0, 0, 1, "황혼과 여명은 각각 해가 지고 뜨는 때...\\n\\n정령의 활동이 가장 활성화되는 때...");
        if(@pos == 0) end;

        set @pos, list2(@list, 0, 0, "나는 이 활성화 된 정령의 힘을 모두 결집한, 황혼의 옷과 여명의 옷을 만드는 자...", "제게도 옷을 만들어주세요.", "수고하세요.");
        if(@pos == 1 && @list == 1)
        {
COS001:
            set @pos, dialog(0, 0, 1, "그러나 옷을 만드는 데에는 재료가 필요하다...\\n\\n정령의 힘을 담은 재료가 필요하다...");
            if(@pos == 0) end;
COS002:
            set @pos, dialog(0, 1, 1, "외부인인 그대가 과연 정령의 힘을 이해하고 그 재료가 어떤 것인지 깨달을 수 있을까...\\n\\n깨닫는 것은 그대가 할 일... 재료를 구해오는 것도 그대가 할 일... 나는 재료로 옷을 만들 뿐...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS001;

            if(countitem(name2itemid("번개의토템")) == 0 && countitem(name2itemid("바람의토템")) == 0 && countitem(name2itemid("대지의토템")) == 0 && countitem(name2itemid("화염의토템")) == 0)
            {
                set @pos, dialog(0, 0, 1, "외부인이여... 그대는 아직 깨닫지 못했다... 그대는 아직 정령의 힘을 담은 재료가 무엇인지 모르고 있다...");
                end;
            }
            else
            {
COS003:
                set @pos, dialog(0, 0, 1, "그렇다... 그대는 정령의 힘을 깨달았다... 토템이야말로 정령의 힘을 담은 재료...");
                if(@pos == 0) end;
COS004:
                set @pos, dialog(0, 1, 1, "화염, 번개, 대지, 바람의토템... 모두 4가지의 토템이 3개씩 모여야 한다...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS003;
COS005:
                set @pos, dialog(0, 1, 1, "그대는 비록 외부인이지만 정령의 힘을 이해했다... 노력에 대한 보답으로... 다음 재료가 무엇인지 가르쳐준다...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS004;
COS006:
                set @pos, dialog(0, 1, 1, "황혼과 여명 사이에는... 정령들의 활동의 증거로 '이슬'이 맺힌다...\\n\\n그 이슬을 50방울 모아 마음으로부터 기원하면... 어떤 특별한 이슬... '여신의이슬'을 만들 수 있다...\\n\\n그 이슬 또한 옷의 재료다...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS005;

                set @pos, dialog(0, 1, 1, "폭염도의 북쪽으로 가라... 이슬을 얻을 수 있을 것이다...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS006;

                set $totem_clothes, 1;
                spellaether @sd, name2spellid("여신의이슬"), 180;
                addlegend @sd, 42, 7, 0, 16, "투명한 이슬을 " + $totem_water + "개 구했다.";
                end;
            }
        }
        else end;
        end;
    }
    if($totem_clothes == 1)
    {
        if(countitem(name2itemid("번개의토템")) < 3 || countitem(name2itemid("바람의토템")) < 3 || countitem(name2itemid("대지의토템")) < 3 || countitem(name2itemid("화염의토템")) < 3 || countitem(name2itemid("여신의이슬")) == 0)
        {
            set @pos, dialog(0, 0, 1, "아직 그대는 재료를 모두 구하지 못했다. 화염, 번개, 대지, 바람의토템... 모두 4가지의 토템이 3개씩 모여야 하며 여신의이슬이 필요하다. 이슬은 폭염도 북쪽에서 구할 수 있을 것이다...");
            end;
        }
        else
        {
COS007:
            set @pos, dialog(0, 0, 1, "여신의이슬을 구해왔는가...\\n\\n그럼 마지막으로 자연의인장을 구하라...");
            if(@pos == 0) end;
COS008:
            set @pos, dialog(0, 1, 1, "자연의인장은 모든 정령들의 힘을 결집할 수 있는 힘을 가지는 일종의 증표다...\\n\\n자연의인장 또한 옷의 재료다...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS008;

            set @pos, dialog(0, 1, 1, "자연의인장은 이 섬을 다르시는 존재가 두 조각으로 나눠 보관하고 있다...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS008;

            for(set @loop, 0; @loop < 3; set @loop, @loop+1)
            {
                delitem @sd, name2itemid("번개의토템"), 1;
                delitem @sd, name2itemid("바람의토템"), 1;
                delitem @sd, name2itemid("대지의토템"), 1;
                delitem @sd, name2itemid("화염의토템"), 1;
            }
            delitem @sd, name2itemid("여신의이슬"), 1;
            message @sd, 3, "번개의토템 주었습니다";
            message @sd, 3, "바람의토템 주었습니다";
            message @sd, 3, "대지의토템 주었습니다";
            message @sd, 3, "화염의토템 주었습니다";
            message @sd, 3, "여신의이슬 주었습니다";

            set $totem_clothes, 2;
            addlegend @sd, 42, 7, 0, 16, "자연의인장을 구하자.";
            end;
        }
    }
    if($totem_clothes == 2)
    {
        if(countitem(name2itemid("자연의인장")) == 0)
        {
            set @pos, dialog(0, 0, 0, "그대는 아직 자연의인장을 구하지 못했다..");
            end;
        }
        else
        {
            set @pos, list2(@list, 0, 0, "재료를 다 모았다면, 만들고자 하는 옷의 이름을 말하라...", "황혼의갑주를 주세요.(남자전사용)", "여명의연갑을 주세요.(여자전사용)", "황혼의활복을 주세요.(남자도적용)", "여병의도복을 주세요.(여자도적용)", "황혼의도포를 주세요.(남자주술사용)", "여명의치마를 주세요.(여자주술사용)", "황혼의장삼을 주세요.(남자도사용)", "여명의통옷을 주세요.(여자도사용)");
            if(@pos == 0) end;
            else if(@list > 0)
            {
                setarray @item$[0], "황혼의갑주", "여명의연갑", "황혼의활복", "여명의도복", "황혼의도포", "여명의치마", "황혼의장삼", "여명의통옷";
                delitem @sd, name2itemid("자연의인장"), 1;
                message @sd, 3, "자연의인장 주었습니다";
                set @add$, @item$[@list-1];
                deletearray @item$[0], getarraysize(@item$);
                additem @sd, name2itemid(@add$), 1;
                set $totem_clothes, 3;
                addlegend @sd, 42, 7, 0, 16, "정령의옷을 만들었다!";
                set @pos, dialog(0, 0, 1, @add$ + "이(가) 완성되었다.. " + @add$ + "에 깃든 정령들이 그대를 수호할 것이다...");
            }
            deletearray @item$[0], getarraysize(@item$);
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, ".............");
        end;
    }
}