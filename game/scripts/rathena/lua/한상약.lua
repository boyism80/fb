{
    set @sd, mysd();
    set @id, myid();

    if($hansangyak == 0)
    {
COS001:
        set @pos, list2(@list, 0, 0, "안녕하신가? 시간 좀 내주실 수 있겠나?", "네, 무슨 일이세요?", "죄송합니다. 바빠서 이만.");
        if(@pos == 0) end;
        else if(@list == 1)
        {
COS002:
            set @pos, dialog(1, 0, 1, "나는 고구려 사람이라네.\\n\\n소문을 듣자 하니 이 섬에는 고구려에서는 볼 수 없는 신비한 효험을 지닌 약초들이 많이 자란다고 하더군.\\n\\n이곳의 토질을 조사하여 그 약초들을 재배할 수 있는 환경을 고구려에도 조성해 보자고 한다네.");
            if(@pos == 0) end;
COS003:
            set @pos, dialog(1, 1, 1, "하지만 막상 와보니, 곳곳의 맹수, 괴수들 때문에 필요한 표본들을 모으기가 무척 힘들군.\\n제대로 된 조사를 위해선 더욱 많은 양이 필요한데 큰일일세...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
COS004:
            set @pos, list2(@list_2, 1, 0, "그래서 말인데.. 자네 혹시 도와줄 생각 없는가?\\n내가 원하는 토양을 채집해 준다면, 은전으로 사례하겠네. 어떤가?", "네, 해보죠.", "죄송합니다만, 힘들겠습니다.");
            if(@pos == 0) end;
            else if(@list_2 == 1)
            {
                set $hansangyak, 1;
                set $red_rock, 0;
                spellaether @sd, name2spellid("적심토"), 180;
                addlegend @sd, 22, 6, 0, 1, "적심토 " + $red_rock + "꾸러미 보관중.";
                set @pos, dialog(1, 0, 1, "고맙네. 내가 필요로 하는 흙은 이렇게 생겼다네.\\n\\n고구려에서는 찾아볼 수 없는 것으로, 내가 적심토라 이름 붙혔지.\\n냄세도 잘 기억해 두게나.\\n\\n괴수들이 있는 건처에서 적심토를 찾을 수 있을 것이네. 조심하게나.");
                end;
            }
            else end;
        }
        else end;
    }
    else
    {
        if($red_rock <= 0)
        {
            set @pos, dialog(0, 0, 0, "아직 적심토를 찾지 못한 모양이군. 괴수들이 있는 건처에서 구할수 있을걸세.");
            end;
        }
        else
        {
            set @money, ($red_rock)*5000;
            set @pos, list2(@list_3, 0, 0, "자네가 가져온 적심토 " + $red_rock + " 꾸러미에 대한 보상으로 " + @money + " 전을 주겠네. 괜찮은가?", "좋습니다.", "지금은 싫습니다.");
            if(@pos == 0) end;
            else if(@list_3 == 1)
            {
                set @pos, list2(@list_4, 0, 0, "적심토 채집을 계속 하겠나?", "네", "이제 그만할래요.");
                if(@pos == 0) end;
                else if(@list_4 == 1)
                {
                    set_money @sd, get_money(@sd)+($red_rock*5000);
                    updatestatus @sd, 2;
                    set $red_rock, 0;
                    addlegend @sd, 22, 6, 0, 1, "적심토 " + $red_rock + "꾸러미 보관중.";
                    set @pos, dialog(0, 0, 1, "괴수들이 있는 근처에서 적심토를 찾을 수 있을 것이네. 조심하게나");
                    end;
                }
                else if(@list_4 == 2)
                {
                    set_money @sd, get_money(@sd)+($red_rock*5000);
                    updatestatus @sd, 2;
                    set $red_rock, 0;
                    set $hansangyak, 0;
                    dellegend @sd, 22;
                    set @pos, dialog(0, 0, 1, "알겠네.. 그럼 언제든지 채집을 하고 싶거든 다시 오게.");
                    end;
                }
                else end;
            }
            else end;
        }
    }
}