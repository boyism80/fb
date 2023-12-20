{
    set @id, myid();
    set @sd, mysd();
    set @limlvl, getarg(0);
    set @jobname$, getarg(1);
    set @job, getarg(2);
    image 0, getarg(3), getarg(4);
    if(get_job(@sd) == 0 && get_level(@sd) >= @limlvl)
    {
        if(@job == 1)
            set @list, list1("험난한 길을 걷는 수행자여,\\n무슨 일로 저를 찾으셨소?", "전사직업가지기");
        else if(@job == 2)
            set @list, list1("험난한 길을 걷는 수행자여,\\n무슨 일로 저를 찾으셨소?", "도적직업가지기");
        else if(@job == 3)
            set @list, list1("험난한 길을 걷는 수행자여,\\n무슨 일로 저를 찾으셨소?", "술사직업가지기");
        else if(@job == 4)
            set @list, list1("험난한 길을 걷는 수행자여,\\n무슨 일로 저를 찾으셨소?", "도사직업가지기");

        if(@list == 1)
        {
            set @ret, dialog(0, 0, 1, @jobname$ + "의 길을 가려면, 몇 가지\\n맹세를 해야하느니.");
            if(@ret == 0) end;
            set @list1_1, list1("첫째로, 하늘에서 굽어보고 계신\\n천제(天帝) 앞에 복종을 맹세하겠느냐?", "예", "아니오");
            if(@list1_1 == 1)
            {
                set @list1_2, list1("둘째로, 험난한 " + @jobname$ + "수련의 길에\\n너의 평생을 바칠 것을\\n맹세하겠느냐?", "예", "아니오");
                if(@list1_2 == 1)
                {
                    set @list1_3, list1("셋째로, 불의를 보고 그냥\\n지나치지 않을 것을 맹세하겠느냐?", "예", "아니오");
                    if(@list1_3 == 1) 
                    {
                        set @list1_4, list1("훌륭하군. 그렇다면, 지금까지의\\n맹세를 증명하기 위해 도토리를 10개 바치거라.", "예", "아니오");
                        if(@list1_4 == 1)
                        {
                            set @itemexist, countitem(name2itemid("도토리"));
                            if(@itemexist < 10) { set @ret, dialog(0, 0, 1, "아직 너의 정성이 부족하니\\n다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를\\n찾아오도록 하여라."); end; }
                            else
                            {
                                if($nakrang8 == 1)
                                {
                                    set_job @sd, @job;
                                    addspell name2spellid("비영사천문");
                                    addspell name2spellid("성황령");
                                    delitem @sd, name2itemid("낙랑의두루마리8"), 1;
                                    delitem @sd, name2itemid("도토리"), 10;
                                    message @sd, 3, "낙랑의두루마리8 주었습니다";
                                    message @sd, 3, "도토리 주었습니다";
                                    addlegend @sd, 1, 7, 0, 1, "낙랑의두루마리 완료!";
                                    set $nakrang8, 0;
                                }
                                else
                                {
                                    set @ret, dialog(0, 0, 1, "낙랑의두루마리부터\\n완수하고 오게나.");
                                    end;
                                }

                                for(set @loop, 0; @loop < 8; set @loop, @loop+1)
                                {
                                    set_unequipitem @sd, @loop;
                                }

                                if(@job == 3)
                                {
                                    set_s_mindam @sd, @job*4;
                                    set_s_maxdam @sd, @job*4;
                                    set_l_mindam @sd, @job*4;
                                    set_l_maxdam @sd, @job*4;
                                }
                                else if(@job == 4)
                                {
                                    set_s_mindam @sd, @job*3;
                                    set_s_maxdam @sd, @job*3;
                                    set_l_mindam @sd, @job*3;
                                    set_l_maxdam @sd, @job*3;
                                }

COS00001:
                                set @ret, dialog(0, 0, 1, "자네는 이제 " + @jobname$ + "으로써의\\n끝없는 길을 가게 되었네!");
                                if(@ret == 0) end;
COS00002:
                                set @ret, dialog(0, 1, 1, "그 길은 멀고도 험난할 테니,\\n마음을 굳건히 하거라.");
                                if(@ret == -1) goto COS0001;
                                else if(@ret == 0) end;
COS00003:
                                set @ret, dialog(0, 1, 1, "자네의 기나긴 여정에 큰\\n도움을 줄 마법을 두 가지를\\n알려 줄테니, 꼭 기억하고\\n유용하게 사용하도록 하여라..");
                                if(@ret == -1) goto COS0002;
                                else if(@ret == 0) end;
COS00004:
                                set @ret, dialog(0, 1, 1, "'성황령'은 몬스터를\\n사냥하다가 죽었을 경우,\\n사용하는 마법이니라. 죽었을\\n때, '성황령'을 사용하면,\\n원하는 방향의 성황당으로 올\\n수 있을 것이야..");
                                if(@ret == -1) goto COS0003;
                                else if(@ret == 0) end;
COS00005:
                                set @ret, dialog(0, 1, 1, "'비영사천문'은 각 성의\\n동,서,남,북 사대문으로 바로\\n갈 수 있는 기술일세.");
                                if(@ret == -1) goto COS0004;
                                else if(@ret == 0) end;
COS00006:
                                set @ret, dialog(0, 1, 1, "이 두가지 마법을 꼭\\n기억하고 유용하게\\n사용하도록 하게.");        
                                if(@ret == -1) goto COS0005;
                                else if(@ret == 0) end;
                            }
                        }
                        else if(@list1_4 == 2){ set @ret, dialog(0, 0, 1, "아직 너의 정성이 부족하니\\n다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를\\n찾아오도록 하여라."); end; }
                    }
                    else if(@list1_3 == 2){ set @ret, dialog(0, 0, 1, "아직 너의 정성이 부족하니\\n다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를\\n찾아오도록 하여라."); end; }
                }
                else if(@list1_2 == 2){ set @ret, dialog(0, 0, 1, "아직 너의 정성이 부족하니\\n다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를\\n찾아오도록 하여라."); end; }
            }
            else if(@list1_1 == 2){ ENDA: set @ret, dialog(0, 0, 1, "아직 너의 정성이 부족하니\\n다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를\\n찾아오도록 하여라."); end; }
        }
    }
}