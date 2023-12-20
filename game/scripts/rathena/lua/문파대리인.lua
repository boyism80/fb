{
    image 0, 220, 0;
    set @sd, mysd();
    set @id, myid();
    set @money, 2000000;
    set @limit_member, 5;
    set @menu, menu("안녕하세요. 무엇을 도와드릴까요?", "문파 제작", "문파 관리");
    if(@menu == 1) goto make;
    else if(@menu == 2) goto setting;
    else end;
    end;
make:
    if(clan_exist(@sd) != 0)
    {
        set @pos, dialog(1, 0, 1, "이미 문파에 활동중이십니다.");
        end;
    }

    if(get_level(@sd) < 99)
    {
        set @pos, dialog(1, 0, 1, "레벨 99이상만 문파를 제작할 수 있습니다.");
        end;
    }

    set @menu, menu("문파를 제작하는데는, " + @money + "만전이 필요합니다. 정말로 만드시겠습니까?", "네", "아니요");
    if(@menu == 1)
    {
        set @menu, menu("정말로요?", "네", "아니요");
        if(@menu == 1)
        {
AAA_001:
            set @pos, dialog(1, 0, 1, "<주의사항>\\n문파를 정하기 이전에, 파티원 " + (@limit_member-1) + "명이 있는지 확인해 주세요. 파티원이 자신을 제외한 " + (@limit_member-1) + "명이 존재하지 않으면 문파를 창설할 수 없습니다.");
            if(@pos == 0) end;
            if(party(@sd) == 0) { set @pos, dialog(1, 0, 0, "자신을 포함한 " + @limit_member + "명이상의 파티원을 모아서 와주세요."); end; }
            else
            {
                for(set @i, 0; @i < partymax(); set @i, @i+1)
                {
                    set @target_sd, party_member(@sd, @i);
                    if(@target_sd > 0)
                    {
                        if(clan_exist(@target_sd) != 1 && get_mapid(@sd) == get_mapid(@target_sd))
                        {
                            set @member_count, @member_count+1;
                        }
                    }
                }
                if(@member_count < @limit_member) { set @pos, dialog(1, 0, 0, "파티원중 한명이 문파에 이미 속해 있거나, 또는 같은 맵에 존재하지 않던가, 파티원이 부족합니다."); end; }
            }
AAA_002:
            set @pos, input2(@input$, 1, 12, "만들 문파 이름을 정하시기 바랍니다.", "", "");
            if(@pos == 0) end;
            else if(@pos == -1) goto AAA_001;
AAA_003:
            set @pos, dialog(1, 1, 1, @input$ + "이라고 문파이름을 정하셨습니다.\\n확실하면 '다음'을 눌르시고, 그렇지 않으면 종료해주십시오.");
            if(@pos == 0) end;
            else if(@pos == -1) goto AAA_002;
AAA_004:
            set @list, list1(@input$ + "이라고 문파이름을 정하셨습니다. 정말로 맞습니까? ", "네", "아니요");
            if(@list == 1)
            {
AAA_005:
                set @pos, dialog(0, 0, 1, "그럼 문파를 제작하겠습니다. 잠시만 기다려 주십시요.");
                if(get_money(@sd) < @money) { set @pos, dialog(0, 0, 0, "문파를 제작할 돈이 부족합니다."); end; }
                if(@pos == 0) end;
                else if(@pos == -1) goto AAA_004;
AAA_006:
                //문파 세팅
                set @err, makeclan(@input$, 0, get_name(@sd), "0");
                if(@err == 1) { set @pos, dialog(1, 0, 0, "문파이름이 중복되었습니다."); end; }
                else if(@err == 2) { set @pos, dialog(1, 0, 0, "문파장이 존재하지 않습니다."); end; }
                else if(@err == 3) { set @pos, dialog(1, 0, 0, "이미 문파에 가입되어있습니다."); end; }
                else
                {
                    set @member_count, 0;
                    if(party(@sd) == 0) { set @pos, dialog(1, 0, 0, "자신을 포함한 " + @limit_member + "명이상의 파티원을 모아서 와주세요."); end; }
                    else
                    {
                        for(set @i, 0; @i < partymax(); set @i, @i+1)
                        {
                            set @target_sd, party_member(@sd, @i);
                            if(@target_sd > 0 && @target_sd != @sd)
                            {
                                if(clan_exist(@target_sd) != 1 && get_mapid(@sd) == get_mapid(@target_sd))
                                {
                                    set @member_count, @member_count+1;
                                }
                            }
                        }
                    }

                    set #clanname$, @input$;
                    for(set @i, 0; @i < partymax(); set @i, @i+1)
                    {
                        set @target_sd, party_member(@sd, @i);
                        if(@sd != @target_sd && @target_sd > 0)
                        {
                            if(clan_exist(@target_sd) != 1 && get_mapid(@sd) == get_mapid(@target_sd))
                            {
                                runscript @target_sd, "문파강제가입";
                            }
                        }
                    }
                    set_money @sd, get_money(@sd)-@money;
                    updatestatus @sd, 2;
                    broadcast 5, get_name(@sd) + "님이 " + @input$ + "문파를 제작하셨습니다. 축하합니다.";
                    set @pos, dialog(1, 0, 0, "문파를 제작하셨습니다.\\n문파 관리 부분을 이용하실 수 있습니다.");
                    end;
                }
            }
            else end;
        }
        else end;
    }
    else end;
    end;
setting:
    if(clan_exist(@sd) == 0)
    {
        set @pos, dialog(1, 0, 1, "문파에 가입하시지 않았습니다.");
        end;
    }
    else if(strcmp(get_clanmaster(@sd), get_name(@sd)) != 0 && strcmp(get_clanlowmaster(@sd), get_name(@sd)) != 0)
    {
        set @menu, menu("안녕하세요? " + get_clanname(@sd) + "의 " + get_name(@sd) + "회원님 무슨 일로 오셨나요?", "문파 탈퇴");
        if(@menu == 1)
        {
            set @menu, menu("탈퇴시 금전 20만전이 필요합니다.\\n정말로 탈퇴하시겠습니까?", "네", "아니요");
            if(@menu == 1)
            {
                if(get_money(@sd) < 200000) { set @pos, dialog(0, 0, 0, "문파를 탈퇴할 돈이 부족합니다."); end; }
                set_money @sd, get_money(@sd)-200000;
                updatestatus @sd, 2;

                set @err, del_clan(@sd, get_clanname(@sd), get_name(@sd));
                if(@err == 0) { set @pos, dialog(0, 0, 0, "정상적으로 탈퇴되었습니다."); end; }
                else { set @pos, dialog(0, 0, 0, "에러가 발생하였습니다."); end; }
            }
            else end;
        }
        else end;
    }
    else
    {
POC_000:
        set @menu, menu("안녕하세요? " + get_clanname(@sd) + "의 " + get_name(@sd) + "회원님 무슨 일로 오셨나요?", "문파 현황", "문파성 관리", "문파원 초대", "문파원 축출", "문파장 이임", "부문주 설정", "부문주 해임", "문파타이틀 설정");
        if(@menu == 1)
        {
            set @pos, dialog(1, 0, 1, "청룡의성 점령 문파: " + #dragon_castle$ + " 문주 이름: " + #dragon_castle_master$ + "\\n주작의성 점령 문파: " + #bird_castle$ + " 문주 이름: " + #bird_castle_master$ + "\\n백호의성 점령 문파: " + #tiger_castle$ + " 문주 이름: " + #tiger_castle_master$ + "\\n현무의성 점령 문파: " + #turtle_castle$ + " 문주 이름: " + #turtle_castle_master$);
            if(@pos == 0) end;
            else if(@pos == 1) goto POC_000;
        }
        else if(@menu == 2)
        {
            if(!strcmp(get_clanname(@sd), #dragon_castle$))
            {
                set @pos, dialog(1, 0, 1, "현재 청룡의성을 보유하고 있습니다.");
                if(@pos == 0) end;
                else if(@pos == 1) goto POC_000;
            }
            else if(!strcmp(get_clanname(@sd), #bird_castle$))
            {
                set @pos, dialog(1, 0, 1, "현재 주작의성을 보유하고 있습니다.");
                if(@pos == 0) end;
                else if(@pos == 1) goto POC_000;
            }
            else if(!strcmp(get_clanname(@sd), #tiger_castle$))
            {
                set @pos, dialog(1, 0, 1, "현재 백호의성을 보유하고 있습니다.");
                if(@pos == 0) end;
                else if(@pos == 1) goto POC_000;
            }
            else if(!strcmp(get_clanname(@sd), #turtle_castle$))
            {
                set @pos, dialog(1, 0, 1, "현재 현무의성을 보유하고 있습니다.");
                if(@pos == 0) end;
                else if(@pos == 1) goto POC_000;
            }
            else
            {
                set @pos, dialog(1, 0, 1, "현재 보유하고 있는 성이 없습니다.");
                if(@pos == 0) end;
                else if(@pos == 1) goto POC_000;
            }
        }
        else if(@menu == 3)
        {
POC_001:
            set @pos, input2(@invite$, 0, 12, "가입하고자 하는 사람의 이름을 적어주세요.", "", "");
            if(@pos == 0) end;

            if(name2sd(@invite$) == 0)
            {
                set @pos, dialog(0, 0, 1, "현재 접속해 있지 않거나, 없는 아이디입니다.");
                end;
            }
            set @target_sd, name2sd(@invite$);
            if(get_mapid(@sd) != get_mapid(@target_sd))
            {
                set @pos, dialog(0, 0, 1, "대상이 같은 맵에 있지 않습니다.");
                end;
            }

            if(clan_exist(@target_sd) != 0)
            {
                set @pos, dialog(0, 0, 1, "이미 문파에 가입되어있습니다.");
                end;
            }
            set #clanname$, get_clanname(@sd);
            runscript @target_sd, "문파가입요청";
            end;
        }
        else if(@menu == 4)
        {
POD_001:
            set @pos, input2(@invite$, 1, 12, "축출하고자 하는 사람의 이름을 적어주세요.", "", "");
            if(@pos == 0) end;
            else if(@pos == -1) goto POC_000;

            if(!strcmp(get_clanmaster(@sd), @invite$) || !strcmp(get_clanlowmaster(@sd), @invite$)) { set @pos, dialog(0, 0, 0, "문파장 또는 부문파장은 탈퇴할 수 없습니다."); end; }

            if(name2sd(@invite$) == 0)
            {
                set @err, del_clan(0, get_clanname(@sd), @invite$);
                if(@err == 0) { set @pos, dialog(0, 0, 0, "정상적으로 탈퇴되었습니다."); end; }
                else { set @pos, dialog(0, 0, 0, "그런 문파원은 존재하지 않습니다."); end; }
            }
            else
            {
                set @target_sd, name2sd(@invite$);
                set @err, del_clan(@target_sd, get_clanname(@target_sd), @invite$);
                if(@err == 0) { set @pos, dialog(0, 0, 0, "정상적으로 탈퇴되었습니다."); end; }
                else { set @pos, dialog(0, 0, 0, "그런 문파원은 존재하지 않습니다."); end; }
            }
        }
        else if(@menu == 5)
        {
            set @pos, dialog(1, 0, 0, "개발중입니다..");
            end;
        }
        else if(@menu == 6)
        {
            if(strcmp(get_clanmaster(@sd), get_name(@sd)) == 0)
            {
POE_001:
                set @pos, dialog(1, 0, 1, "※주의※\\n\\n  설정하고자 하는 부문주는 같은 위치에 있어야 하며, 문파가 같아야 합니다.");
                if(@pos == 0) end;
POE_002:
                set @pos, input2(@invite$, 0, 6, "임명하고자 하는 부문주를 고르세요. 부문주는 같은 문파여야 합니다.", "", "");
                if(@pos == 0) end;

                set @target_sd, name2sd(@invite$);
                if(@target_sd > 0)
                {
                    if(get_clanname(@sd) == get_clanname(@target_sd))
                    {
                        if(@sd != @target_sd)
                        {
                            set_clanlowmaster @sd, @invite$;
                            set @pos, dialog(0, 0, 1, @invite$ + "님이 부문주로 임명되었습니다");
                            end;
                        }
                        else
                        {
                            set @pos, dialog(0, 0, 0, "자신을 부문주로 설정할 수 없습니다.");
                            end;
                        }
                    }
                    else
                    {
                        set @pos, dialog(0, 0, 0, "같은 문파원이 아닙니다.");
                        end;
                    }
                }
                else
                {
                    set @pos, dialog(0, 0, 0, @invite$ + "님은 바람의나라에 없습니다");
                    end;
                }
            }
            else
            {
                set @pos, dialog(0, 0, 0, "문주가 아닙니다.");
                end;
            }
        }
        else if(@menu == 7)
        {
            if(strcmp(get_clanmaster(@sd), get_name(@sd)) == 0)
            {
                set_clanlowmaster @sd, get_name(@sd);
                set @pos, dialog(0, 0, 1, "부문주가 해임되었습니다");
                end;
            }
            else
            {
                set @pos, dialog(0, 0, 0, "문주가 아닙니다.");
                end;
            }
        }
        else if(@menu == 8)
        {
            if(strcmp(get_clanmaster(@sd), get_name(@sd)) == 0)
            {
                set @pos, input2(@username$, 0, 6, "문파타이틀을 설정해줄 유저를 입력해주세요.", " ", " ");
                if(@pos == 0) end;

                set @target_sd, name2sd(@username$);
                if(@target_sd > 0)
                {
                    set @pos, input2(@clantitle$, 0, 10, "지정할 문파타이틀을 써주세요.", "문파타이틀:", "으로 지정하겠습니다.");
                    if(@pos == 0) end;
    
                    if(get_clanname(@sd) == get_clanname(@target_sd))
                    {
                        set_clantitle @target_sd, "                               " + @clantitle$;
                        end;
                    }
                    else
                    {
                        set @pos, dialog(0, 0, 0, "같은 문파원이 아닙니다.");
                        end;
                    }
                }
                else
                {
                    set @pos, dialog(0, 0, 0, @username$ + "님은 바람의나라에 없습니다.");
                    end;
                }
            }
            else
            {
                set @pos, dialog(0, 0, 0, "문파장이 아닙니다.");
                end;
            }
        }
    }
}