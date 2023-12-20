{
    set @sd, mysd();
    set @id, myid();

    image 0, 4, 4;
COS001:
    set @list, list1("안녕하세요. 어떻게 오셨나요?", "결혼을 하려고 왔습니다", "이혼을 하려고 왔습니다");
    if(@list == 1)
    {
        if(get_level(@sd) < 21)
        {
            set @pos, dialog(0, 0, 0, "레벨 21이상만 결혼할 수 있습니다.");
            end;
        }
        else
        {
            set @nt, callfunc("Server_Nowtime");
            if($marry_time > @nt) { set @pos, dialog(0, 0, 0, "" + ($marry_time-@nt) + "초 후에 다시 결혼이 가능하십니다."); end; }
            else
            {
                if($marry_check != 0)
                {
                    set @pos, dialog(0, 0, 0, "이미 결혼을 하셨습니다.");
                    end;
                }
                else
                {
                    if(get_gender(@sd) == 1)
                    {
                        set @pos, dialog(0, 0, 0, "결혼은 남자만이 신청하실 수 있습니다.");
                        end;
                    }
                    else
                    {
                        set @pos, input2(@marry$, 0, 6, "결혼하실 이름을 입력해 주십시오.", "나와 결혼할 상대의 이름은:", "입니다.");
                        if(@pos == 0) end;
                        else if(@pos == -1) goto COS001;

                        set @target_id, name2id(@marry$);
                        set @target_sd, name2sd(@marry$);
                        if(@target_sd > 0)
                        {
                            set @choice, list1("정말 " + @marry$ + "님과 결혼하시겠습니까?", "예", "아뇨");
                            if(@choice == 1)
                            {
                                if(get_x(@id) == 12 && get_y(@id) == 8 && get_x(@target_id) == 18 && get_y(@target_id) == 8)
                                {
                                    if(@target_sd <= 0) end;
    
                                    if(get_level(@target_sd) < 21)
                                    {
                                        set @pos, dialog(0, 0, 0, "결혼할 상대의 레벨이 21이상이여야 합니다.");
                                        end;
                                    }
                                    if(get_gender(@sd) == get_gender(@target_sd))
                                    {
                                        set @pos, dialog(0, 0, 0, "같은 성별끼리는 결혼할 수 없습니다.");
                                        end;
                                    }
    
                                    set #marry_check_success, 0;
                                    spellaether @target_sd, name2spellid("결혼체크"), 1;
                                    sleep(1000);
                                    if(#marry_check_success == 1)
                                    {
                                        set @pos, dialog(0, 0, 0, "이미 결혼한 상대와는 결혼을 할 수 없습니다.");
                                        end;
                                    }
                                    else
                                        {
                                        set @pos, dialog(0, 0, 1, @marry$ + "님께 결혼 동의문을 보냈습니다. 잠시만 기다려 주세요.");
                                        if(@pos == 0) end;

                                        if(@target_sd <= 0) end;
    
                                        set #marry_session_2, @sd;
                                        runscript @target_sd, "결혼동의문";
                                        end;
                                    }
                                }
                                else
                                {
                                    set @pos, dialog(0, 0, 0, "신랑은 왼쪽, 신부는 오른쪽 알맞은 위치에 서 주세요.");
                                    end;
                                }
                            }
                        }
                        else
                        {
                            set @pos, dialog(0, 0, 0, @marry$ + "님은 바람의나라에 없습니다.");
                            end;
                        }
                    }
                }
            }
        }
    }
    else if(@list == 2)
    {
        if($marry_check != 1)
        {
            set @pos, dialog(0, 0, 0, "결혼을 한 사람만이 이혼을 할 수 있습니다");
            end;    
        }
        else
        {
            set @list, list1("정말 배우자 " + $marry_name$ + "님과 이혼하시겠습니까?", "예", "아뇨");
            if(@list == 1)
            {
                set @honey$, $marry_name$;
                set @target_id, name2id(@honey$);
                set @target_sd, name2sd(@honey$);
                if(@target_sd > 0)
                {
                    if(get_name(@target_sd) == get_name(@target_sd))
                    {
                        if(get_x(@id) == 12 && get_y(@id) == 8 && get_x(@target_id) == 18 && get_y(@target_id) == 8)
                        {
                            set @pos, dialog(0, 0, 1, get_name(@target_sd) + "님께 이혼동의문을 보냈습니다. 승낙을 할시 이혼이 성사됩니다.");
                            if(@pos == 0) end;

                            if(@target_sd <= 0) end;

                            set #marry_session, @sd;
                            runscript @target_sd, "이혼동의문";
                            end;
                        }
                        else
                        {
                            set @pos, dialog(0, 0, 0, "이혼하시려는 두 분은 알맞은 위치에 서 주세요. 이혼을 신청하시는 분이 왼쪽에 서야 합니다.");
                            end;
                        }
                    }
                    else
                    {
                        set @pos, dialog(0, 0, 0, "자신과 결혼한 배우자만이 이혼할 수 있습니다");
                        end;
                    }
                }
                else
                {
                    set @pos, dialog(0, 0, 0, $marry_name$ + "님은 바람의나라에 없습니다.");
                    end;
                }
            }
        }
    }
}