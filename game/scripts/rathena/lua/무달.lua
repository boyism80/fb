{
    set @sd, mysd();
    set @id, myid();
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "정기모으기");
    if(@ret == 1)
    {
        if($shark_weapon < 4)
        {
            set @pos, dialog(0, 0, 0, "자네는 아직 알 때가 아니군.");
            end;
        }
        else
        {
            if($shark_weapon == 4)
            {
                set @pos, list2(@list, 0, 0, "무슨 일인가?", "용궁의정기를 만들줄 아십니까?");
                if(@pos == 0) end;
                else if(@list != 1) end;

                set @pos, list2(@list_2, 0, 0, "물론 만들줄은 아네만...그리 쉽게 만들수는 없다네..", "!!!");
                if(@pos == 0) end;
                else if(@list_2 != 1) end;

                set @pos, list2(@list_3, 0, 0, "그래도 만들고 싶은가?", "물론이지요.", "아니요. 포기할래요.");
                if(@pos == 0) end;
                else if(@list_3 != 1) end;

                set $shark_weapon, 5;
                addlegend @sd, 24, 7, 0, 1, "무달의 부탁을 들어주자. [1/5]";
COS001:
                set @pos, dialog(0, 0, 1, "한번에 다 알려줘 봐야 소용없으니 한번에 하나씩만 알려주겠네.");
                if(@pos == 0) end;
COS002:
                set @pos, dialog(0, 1, 0, "먼저 복어의심장 30개를 가지고 오게나.");
                if(@pos == -1) goto COS001;
                end;
            }
            if($shark_weapon == 5)
            {
                if(countitem(name2itemid("복어의심장")) < 30)
                {
                    set @pos, dialog(0, 0, 0, "복어의심장 갯수가 부족한 것은 아닌가? 30개가 필요하네.");
                    end;
                }
                else
                {
                    set $shark_weapon, 6;
                    addlegend @sd, 24, 7, 0, 1, "무달의 부탁을 들어주자. [2/5]";
                    delitem @sd, name2itemid("복어의심장"), 30;
                    message @sd, 3, "복어의심장 주었습니다";
                    additem @sd, name2itemid("용궁의정기1"), 1;
                    set @pos, dialog(0, 0, 1, "다 모아왔군 그래. 그럼 이젠 문어의심장을 30개 모아오게나.");
                    end;
                }
            }
            if($shark_weapon == 6)
            {
                if(countitem(name2itemid("문어의심장")) < 30)
                {
                    set @pos, dialog(0, 0, 0, "문어의심장 갯수가 부족한 것은 아닌가? 30개가 필요하네.");
                    end;
                }
                else
                {
                    set $shark_weapon, 7;
                    addlegend @sd, 24, 7, 0, 1, "무달의 부탁을 들어주자. [3/5]";
                    delitem @sd, name2itemid("문어의심장"), 30;
                    message @sd, 3, "문어의심장 주었습니다";
                    additem @sd, name2itemid("용궁의정기2"), 1;
                    set @pos, dialog(0, 0, 1, "다 모아왔군 그래. 그럼 이젠 해마의심장을 30개 모아오게나.");
                    end;
                }
            }
            if($shark_weapon == 7)
            {
                if(countitem(name2itemid("해마의심장")) < 30)
                {
                    set @pos, dialog(0, 0, 0, "해마의심장 갯수가 부족한 것은 아닌가? 30개가 필요하네.");
                    end;
                }
                else
                {
                    set $shark_weapon, 8;
                    addlegend @sd, 24, 7, 0, 1, "무달의 부탁을 들어주자. [4/5]";
                    delitem @sd, name2itemid("해마의심장"), 30;
                    message @sd, 3, "해마의심장 주었습니다";
                    additem @sd, name2itemid("용궁의정기3"), 1;
                    set @pos, dialog(0, 0, 1, "다 모아왔군 그래. 그럼 이젠 인어의심장을 30개 모아오게나.");
                    end;
                }
            }
            if($shark_weapon == 8)
            {
                if(countitem(name2itemid("인어의심장")) < 30)
                {
                    set @pos, dialog(0, 0, 0, "인어의심장 갯수가 부족한 것은 아닌가? 30개가 필요하네.");
                    end;
                }
                else
                {
                    set $shark_weapon, 9;
                    addlegend @sd, 24, 7, 0, 1, "무달의 부탁을 들어주자. [5/5]";
                    delitem @sd, name2itemid("인어의심장"), 30;
                    message @sd, 3, "인어의심장 주었습니다";
                    additem @sd, name2itemid("용궁의정기4"), 1;
                    set @pos, dialog(0, 0, 1, "다 모아왔군 그래. 그럼 이젠 상어의심장을 30개 모아오게나.");
                    end;
                }
            }
            if($shark_weapon == 9)
            {
                if(countitem(name2itemid("상어의심장")) < 30)
                {
                    set @pos, dialog(0, 0, 0, "상어의심장 갯수가 부족한 것은 아닌가? 30개가 필요하네.");
                    end;
                }
                else
                {
                    set $shark_weapon, 10;
                    addlegend @sd, 24, 7, 0, 1, "무달의 부탁을 들어주었다.";
                    delitem @sd, name2itemid("상어의심장"), 30;
                    message @sd, 3, "상어의심장 주었습니다";
                    additem @sd, name2itemid("용궁의정기5"), 1;
                    set @pos, dialog(0, 0, 1, "이제 용궁의정기는 모두 다 모았네. 가서 볼일을 보시게나.");
                    end;
                }
            }
            else
            {
                set @pos, dialog(0, 0, 1, "......");
            }
        }
    }
    else end;
}