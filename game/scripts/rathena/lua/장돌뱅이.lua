{
    set @sd, mysd();
    set @id, myid();
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "장돌뱅이_별주부전");
    if(@ret == 1)
    {
        if($rabbit_liver != 1)
        {
            set @pos, dialog(0, 0, 1, "지금은 이야기를 할 때가 아니군..");
            end;
        }
        else
        {
            set @pos, list2(@menu, 0, 0, "어쩐일이신가?", "토끼의간을 구해야되는데..", "뇌진도를 드리겠습니다.");
            if(@pos == 0) end;
            else if(@menu == 1)
            {
COS001:
                set @pos, dialog(0, 0, 1, "음.. 거북장군에게서 토끼의간을 구해오라는 부탁을 받았단 말아지. 그런데 그 토끼는 정말 희귀한 토끼라네. 보통토끼와는 다르지");
                if(@pos == 0) end;
COS002:
                set @pos, dialog(0, 1, 1, "나도 장사 하느라 전국 방방곡곡 안가보는데가 없지만, 그 토끼는 딱 한 번 밖에 못봤었지.. 어디서 보았냐 하면 말일세...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS001;
COS003:
                set @pos, dialog(0, 1, 1, "흐흐흐.. 이보게, 하지만 말일세.. 조금 야속하게 들릴지 모르겠지만, 나도 장사꾼이 아닌가?");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS002;
COS004:
                set @pos, list2(@choice, 0, 0, "흐흐흐...", "원하는게 뭡니까");
                if(@pos == 0) end;
                else if(@choice == 1)
                {
                    set @pos, dialog(0, 0, 0, "요즘 한창 장사가 되는 물건이 뇌진도라네.. 그걸 하나 구해오면 내 분명히 자네에게 그 토끼가 출현하는 장소를 가르쳐 줌세.");
                    end;
                }
                else end;
            }
            else if(@menu == 2)
            {
                if(countitem(name2itemid("뇌진도")) == 0)
                {
                    set @pos, dialog(0, 0, 0, "뇌진도가 없지 않은가?");
                    end;
                }
                else
                {
                    set $rabbit_liver_2, 1;
                    delitem @sd, name2itemid("뇌진도"), 1;
                    message @sd, 3, "뇌진도 주었습니다";
                    addlegend @sd, 20, 7, 0, 1, "토깽이를 생포하자.";
COS005:
                    set @pos, dialog(0, 0, 1, "그 토끼가 출현하는 곳은 다름 아닌 한두고개라네.. 나도 한두고개에서 깊숙히 안들어가봐서 깊숙한 곳에서도 출현할지 어떨지는 모르겠네만");
                    if(@pos == 0) end;
COS006:
                    set @pos, dialog(0, 1, 1, "분명한 것은 한두고개에서 깊지 않은 곳에서 봤다는 사실일세.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS005;
COS007:
                    set @pos, dialog(0, 1, 1, "그럼 행운을 비네.. 아참, 이건 자네를 위해서 하는 얘기네만, 그 토끼는 아주 간악하다는 사실을 꼭 염두에 두게");
                    if(@pos == -1) goto COS006;
                    end;
                }
            }
            else end;
        }
    }
    else end;
}