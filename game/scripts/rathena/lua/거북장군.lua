{
    set @sd, mysd();
    set @id, myid();
    set @ret, list1("안녕하세요? 어떻게 오셨나요?", "별주부전", "상어장군의무기");
    if(@ret == 1)
    {
        if(get_level(@sd) < 30)
        {
            set @pos, dialog(0, 0, 0, "자네가 아직 이 일을 도우기엔 조금 역부족이라 생각이 되는군.");
            end;
        }
        else
        {
            if($rabbit_liver == 0)
            {
COS001:
                set @pos, dialog(0, 0, 1, "휴우우.. 용왕님이 편찮으셔서 큰일입니다. 요즘 용궁은 용왕님이 편찮으셔서 다들 걱정하고 있답니다.");
                if(@pos == 0) end;
COS002:
                set @pos, dialog(0, 1, 1, "들리는 소문엔 지상에 사는 토끼라는 동물의 간이 용왕님의 병에 좋은 효과가 있다고 하던데....");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS001;
COS003:
                set @pos, dialog(0, 1, 1, get_name(@sd) + "님.. 용왕님을 위해서 토끼의 간을 구해다 주실 수 없겠습니까?");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS002;
COS004:
                set @pos, list2(@list, 0, 1, "구해주시겠습니까?", "당연히 구해다 드려야죠.", "요즘 좀 바빠서..");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS003;
                else if(@list == 1)
                {
                    set $rabbit_liver, 1;
                    addlegend @sd, 20, 7, 0, 1, "토깽이의 행방을 알아보자.";
COS005:
                    set @pos, dialog(0, 0, 1, get_name(@sd) + "님 정말 감사합니다.. 제발 토끼의 간을 구해 주세요.");
                    if(@pos == 0) end;
COS006:
                    set @pos, dialog(0, 1, 1, "장터의 장돌뱅이가 토끼가 어디에 있는지 알고 있다고 합니다.. 그럼 부탁드립니다.");
                    if(@pos == -1) goto COS005;
                    end;
                }
                else end;
            }
            if($rabbit_liver == 1)
            {
                if(countitem(name2itemid("토끼의간")) == 0)
                {
                    set @pos, dialog(0, 0, 0, "아직 토끼의간을 구하시지 못하셨군요..");
                    end;
                }
                else
                {
COS007:
                    set @pos, dialog(0, 0, 1, "토끼의간을 구해오셨군요! 용왕님의 병에 토끼의 간을 써도 될까요?");
                    if(@pos == 0) end;
COS008:
                    set @pos, list2(@liver, 0, 0, "토끼의 간을 주시겠어요?", "네, 가져가세요.", "아뇨.. 제가 가지고 있을래요.");
                    if(@pos == 0) end;
                    else if(@liver != 1) end;

                    set $rabbit_liver, 2;
                    delitem @sd, name2itemid("토끼의간"), 1;
                    message @sd, 3, "토끼의간 주었습니다";
                    additem @sd, name2itemid("주홍투구"), 1;
                    addlegend @sd, 20, 7, 0, 1, "거북장군의 부탁을 들어주었다.";
COS009:
                    set @pos, dialog(0, 0, 1, "아아.. 감사합니다. 이것으로 용왕님도 건강을 회복하실 수 있겠군요.");
                    if(@pos == 0) end;
COS010:
                    set @pos, dialog(0, 1, 1, get_name(@sd) + "님은 저희 용궁의 은인이십니다. 감사의 의미에서 투구를 하나 드리겠습니다.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS009;
COS011:
                    set @pos, dialog(0, 1, 0, "그럼 이 투루를 잘 쓰시길...");
                    if(@pos == -1) goto COS010;
                    end;
                }
            }
            else
            {
                set @pos, dialog(0, 0, 1, get_name(@sd) + "님, 저번엔 정말 감사했습니다.");
                end;
            }
        }
    }
    else if(@ret == 2)
    {
        if(get_level(@sd) < 99)
        {
            set @pos, dialog(0, 0, 0, "자네가 아직 이 일을 도우기엔 조금 역부족이라 생각이 되는군.");
            end;
        }
        else
        {
            if($shark_weapon == 0)
            {
COS012:
                set @pos, dialog(0, 0, 1, "상어장군이 사용하던 무기의 제작법이 드디어 알려졌다는 소문이 있다네...\\n\\n그 무기의 제작 방법을 아는 사람이 았다는 것 같더군.");
                if(@pos == 0) end;
COS013:
                set @pos, dialog(0, 1, 1, "다시 상어장군용 무기를 만들기 위해서 그 제작방법이 꼭 필요하다네.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS012;
COS014:
                set @pos, list2(@shark, 0, 0, "자네가 좀 알아봐 줄 수 있는가?", "물론입니다...", "별로 흥미가 안내켜서..");
                if(@pos == 0) end;
                else if(@shark != 1) end;

                set $shark_weapon, 1;
                addlegend @sd, 24, 7, 0, 1, "상어장군의무기 퀘스트를 받다.";
COS015:
                set @pos, dialog(0, 0, 1, "좋아. 그런데 사실은 나도 누가 알고 있는지는 자세히는 모른다네.");
                if(@pos == 0) end;
COS016:
                set @pos, dialog(0, 1, 1, "단지 지나가는 말로 아주 추운 지방에서 누군가 알고 있다고 들은 기억이 있어서 말이야..\\n\\n자세한 건 자네가 직접 찾아봐야 할거야...\\n\\n그럼 부탁하네.");
                if(@pos == -1) goto COS015;
                end;
            }
            if($shark_weapon >= 1 && $shark_weapon <= 10)
            {
                set @pos ,dialog(0, 0, 1, "아주 추운곳에 있다고 들었는데...");
                end;
            }
            if($shark_weapon == 11)
            {
                if(countitem(name2itemid("무기제조법")) == 0)
                {
                    set @pos, dialog(0, 0, 0, "아직 아무것도 얻어오지 못했나?");
                    end;
                }
                else
                {
COS017:
                    set @pos, dialog(0, 0, 1, "어디 그럼 한번 볼까.");
                    if(@pos == 0) end;
COS018:
                    set @pos, dialog(0, 1, 1, "으음...으으음....으으으으음.....!!!\\n\\n아. 이제야 제조법을 알았네.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS017;
COS019:
                    set @pos, dialog(0, 1, 1, "이걸 만들기 위해서 상어의핵 3개와 불의수정 3개가 필요하다네.\\n\\n자네가 재료를 다 모아 온다면 내가 직접 만들어 주도록 하겠네.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS018;

                    delitem @sd, name2itemid("무기제조법"), 1;
                    message @sd, 3, "무기제조법 주었습니다";

                    set $shark_weapon, 12;
                    addlegend @sd, 24, 7, 0, 1, "상어장군 무기의 재료를 구하자.";
                    end;
                }
            }
            if($shark_weapon == 12)
            {
                if(countitem(name2itemid("상어의핵")) < 3 || countitem(name2itemid("불의수정")) < 3)
                {
                    set @pos, dialog(0, 0, 0, "재료가 부족한 것 같군. 상어의핵 3개와 불의수정 3개를 구해다 주시게.");
                    end;
                }
                else
                {
                    if(rand(1, 100) <= 60)
                    {
                        set $shark_weapon, 13;
                        for(set @loop, 0; @loop < 3; set @loop, @loop+1)
                        {
                            delitem @sd, name2itemid("상어의핵"), 1;
                            delitem @sd, name2itemid("불의수정"), 1;
                        }
                        message @sd, 3, "상어의핵 주었습니다";
                        message @sd, 3, "불의수정 주었습니다";
                        additem @sd, name2itemid("괴력선창"), 1;
                        addlegend @sd, 24, 6, 0, 1, "상어장군 무기를 만들다!";
COS020:
                        set @pos, dialog(0, 0, 1, "오. 드디어 만들었군...");
                        if(@pos == 0) end;
COS021:
                        set @pos, dialog(0, 1, 1, "이것이 바로 괴력선창이라는 것일세.");
                        if(@pos == 0) end;
                        else if(@pos == -1) goto COS020;
COS022:
                        set @pos, dialog(0, 1, 0, "이걸 자네에게 줄 터이니 부디 유용하게 사용하게나.");
                        if(@pos == -1) goto COS021;
                        end;
                    }
                    else
                    {
                        for(set @loop, 0; @loop < 3; set @loop, @loop+1)
                        {
                            delitem @sd, name2itemid("상어의핵"), 1;
                            delitem @sd, name2itemid("불의수정"), 1;
                        }
                        message @sd, 3, "상어의핵 주었습니다";
                        message @sd, 3, "불의수정 주었습니다";

                        set @pos, dialog(0, 0, 0, "이런.. 재료가 그만 모두 뭉개져버렸군..");
                        end;
                    }
                }
            }
            else
            {
                set @pos, dialog(0, 0, 1, "괴력선창은 유용하게 사용하고 있나?");
                end;
            }
        }
    }
    else end;
}