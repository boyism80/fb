{
    set @id, myid();
    set @sd, mysd();
    image 0, 40, 11;
    if(get_job(@sd) == 0 && get_level(@sd) >= 5)
    {
        callfunc "JOBBIGIN", 5, "전사", 1, 40, 11;
    }
    else if(get_job(@sd) == 0 && get_level(@sd) < 5) { set @ret, dialog(0, 0, 1, "아직 너의 정성이 부족하니\\n다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를\\n찾아오도록 하여라."); end; }
    else if(get_job(@sd) == 1 || get_job(@sd) == 5)
    {
first:
        set @list, list1("험난한 길을 걷는 수행자여,\\n무슨 일로 저를 찾으셨소?", "마법 알아보기", "마법 배우기", "마법 지우기", "칭호 받기");
        if(@list == 1)
        {
            //제한레벨
            setarray @spell_reqlvl[1], 0, 0, 10, 15, 18, 23, 25, 30, 32, 35
                        , 45, 50, 54, 62, 63, 65, 78, 80, 99, 99
                        , -1;

            //배우는 마법
            setarray @spell_list$[1], "성황령", "비영사천문", "누리의기원", "대지의기원", "이중공격"
                        , "신의축복", "후면공격", "측면공격", "삼중공격", "하늘의기원"
                        , "신검합일", "백호령", "유인", "사중공격", "건곤대나이"
                        , "검신검귀", "대력검신", "오중공격", "사자후전사", "동귀어진"
                        , "Error";

            //배우는 마법 설명
            setarray @spell_explain$[1], "캐릭터가 죽었을 경우 사용하면 자신이 해당된 성내 '성황당'으로 이동하게 되는 마법이네 "
                        , "이 기술은 동, 서, 남, 북으로 순식간에 이동할 수 있는 마법으로 필수적이면서도 가장 기본적인 마법이네 "
                        , "자신의 체력을 약간 회복하는 기술로써, 자신의 체력만 회복 가능하여 그다지 중요하지 않은 기술이네 "
                        , "약간의 체력을 회복하는 기술로써, 타인의 체력도 회복해 줄 수 있는 유요한 기술로 보이지 않는 적의 위치도 감지 할 수 있는 유용한 기술이네 "
                        , "순간적으로 두번 연속 공격하는 기술로써, 기본공격이 약 2배 증가되는 마법이네 "
                        , "일반 공격의 명중률을 일정시간 동안 높히는 기술로써, 전사의 중요한 기술이네 "
                        , "앞뒤의 적을 동시에 공격할 수 있는 기술로써, 전사의 중요한 기술이네 "
                        , "동시에 좌우에 있는 적을 공격 할 수 있는 기술로써 전사의 중요한 기술이네 "
                        , "순간적으로 세 번 연속 공격하는 기술로써, 굉장히 유용한 기술이라네 공격력이 약 3배 정도 증가하는 기술이라네 "
                        , "자신의 체력을 200만큼 회복하는 기술로써 자신의 체력만 회복 할 수 있는 기술이라네 "
                        , "검에 자신의 혼을 불어넣어 파괴력을 증가시키는 기술로써, 공격력을 향상시키는 중요한 기술이라네 "
                        , "호랑이의 혼을 빌어 순간적으로 파괴력을 증가 시키는 전사의 중요한 기술이라네 "
                        , "타인과 전투중이 아닌 괴수로 하여금 일정시간동안 자신만을 바라보게하도록 하는 기술로써 중요한 기술이라네 "
                        , "순간적으로 네번 연속 공격하는 기술로써, 기본공격이 약 4배 증가되는 마법이네 "
                        , "자신의 체력의 대부분을 사용, 정면의 상대에게 치명적이고 순간적인 피해를 입히는 전사의 필살기라네 "
                        , "검에 혼을 불어넣어 공격력을 강화시키는 마법으로써 전사의 중요한 기술이라네 "
                        , "검에 혼을 불어넣어 공격력을 강화시키는 마법으로써 전사의 중요한 기술이라네 "
                        , "순간적으로 다섯번 연속 공격하는 기술로써, 기본공격이 약 5배 증가되는 기술이네 "
                        , "목소리에 공력을 실어 크게 외치는 기술로써, 서버내 모든 유저들이 자신의 목소리를 들을 수 있게 해주는 기술이네 "
                        , "자신의 모든 체력을 사용해 정면의 상대에게 치명적인 피해를 입히는 전사의 최고 필살 기술이네 "
                        , "Error";

            //그냥 어법에맞게 을, 를 뜰지하는거
            setarray @spell_goods_message$[1], "을", "을", "을", "을", "을"
                            , "을", "을", "을", "을", "을"
                            , "을", "을", "를", "을", "를"
                            , "를", "을", "을", "를", "을"
                            , "Error";

            //재물정보
            setarray @spell_goods$[1], " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 30전과 도토리 70개를 나에게 바쳐야 하느니라."
                         , " 배우기 위해서는 금전 40전과 도토리 70개, 사슴고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과 도토리 80개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 80전과 도토리 80개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과 도토리 80개, 뱀고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 150전과 도토리 80개, 뱀고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 300전과 도토리 80개, 호랑이고기 15개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 200전과 도토리 80개, 웅담 30개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 500전과 도토리 50개, 여우모피 10개, 철도 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 500전과 도토리 50개, 자호의가죽 2개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 500전과 도토리 50개, 자호의가죽 2개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 900전과 도토리 80개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 1500전과 도토리 100개, 현철중검 2개, 호박 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 1500전과 도토리 80개, 철도 1개, 자호의가죽 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 3000전과 도토리 80개, 현철중검 1개, 호박 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 3500전과 도토리 100개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 5000전과 도토리 100개, 진호박 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 10000전과 도토리 100개, 환두대도 1개, 진호박 20개를 나에게 바쳐야 하느니라."
                        , "Error";

            //체크후 리스트에 뜰 마법
            setarray @spell_list1$, 20;
            //체크후 리스트에 뜰 재료
            setarray @spell_goods1$, 20;
            //체크후 그냥 을,를 중에 뭐띄울지
            setarray @spell_goods_message1$, 20;
            //체크후 대화창에 뜰 설명
            setarray @spell_explain1$, 20;
            //체크할수 있는 변수
            set @count, 0; // 마법 리스트에 관한 변수
            set @s_count, 0; // 마법 몇개 배울수 있는지

            //초기화
            for(set @loop, 1; @loop < 21; set @loop, @loop+1)
            {
                set @spell_list1$[@loop], " ";
                set @spell_goods1$[@loop], " ";
                set @spell_goods_message1$[@loop], " ";
                set @spell_explain1$[@loop], " ";
            }
                    
            for(set @loop, 1; @loop < 22; set @loop, @loop+1)
            {
                set @count, @count+1;
                if(@count == 21 || getarraysize(@spell_list$)-1 == @loop) { break; }
                else if(get_level(@sd) < @spell_reqlvl[@loop] && get_level(@sd)+8 >= @spell_reqlvl[@loop] && checkspell(name2spellid(@spell_list$[@loop])) == 52)
                {
                    if(!strcmp(@spell_list1$[@count], " ")) { set @spell_list1$[@count], @spell_list$[@loop]; }
                    if(!strcmp(@spell_goods1$[@count], " ")) { set @spell_goods1$[@count], @spell_goods$[@loop]; }
                    if(!strcmp(@spell_goods_message1$[@count], " ")) { set @spell_goods_message1$[@count], @spell_goods_message$[@loop]; }
                    if(!strcmp(@spell_explain1$[@count], " ")) { set @spell_explain1$[@count], @spell_explain$[@loop]; }
                    if(!strcmp(@spell_list1$[@count], " ") || !strcmp(@spell_goods1$[@count], " ")) { set @count, @count-1; }
                    set @s_count, @s_count+1;
                }
                else { set @count, @count-1; }
            }

            set @message$, "자네 수준이라면 이런\\n마법들을 알아볼 수 있겠군";
            if(@s_count == 0) { set @ret, dialog(0, 0, 0, "내가 알려줄 마법이 없구려..."); goto failed_deletearray; end; }
            else if(@s_count == 1) { set @addspell, list1(@message$, @spell_list1$[1]); }
            else if(@s_count == 2) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2]); }
            else if(@s_count == 3) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3]); }
            else if(@s_count == 4) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4]); }
            else if(@s_count == 5) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5]); }
            else if(@s_count == 6) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6]); }
            else if(@s_count == 7) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7]); }
            else if(@s_count == 8) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8]); }
            else if(@s_count == 9) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9]); }
            else if(@s_count == 10) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10]); }
            else if(@s_count == 11) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11]); }
            else if(@s_count == 12) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12]); }
            else if(@s_count == 13) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13]); }
            else if(@s_count == 14) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14]); }
            else if(@s_count == 15) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15]); }
            else if(@s_count == 16) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16]); }
            else if(@s_count == 17) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17]); }
            else if(@s_count == 18) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18]); }
            else if(@s_count == 19) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19]); }
            else if(@s_count == 20) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20]); }

            if(@addspell != 0) 
            {
                if(!strcmp(@spell_list$[@addspell], " "))
                {
                    goto failed_deletearray;
                    end;
                }
                set @pos, list2(@ans, 1, 0, @spell_explain1$[@addspell] + @spell_list1$[@addspell] + @spell_goods_message1$[@addspell] + @spell_goods1$[@addspell], "처음으로"); 
                if(@pos == 0) { goto failed_deletearray; end; }
                if(@ans == 1) 
                {
                    deletearray @spell_reqlvl[1], getarraysize(@spell_reqlvl);
                    deletearray @spell_list1[1], getarraysize(@spell_list1);
                    deletearray @spell_goods1$[1], getarraysize(@spell_goods1$);
                    deletearray @spell_goods_message$[1], getarraysize(@spell_goods_message$);
                    deletearray @spell_goods_message1$[1], getarraysize(@spell_goods_message1$);
                    deletearray @spell_list$[1], getarraysize(@spell_list$);
                    deletearray @spell_goods$[1], getarraysize(@spell_goods$);
                    deletearray @spell_goods_explain$[1], getarraysize(@spell_goods_explain$);
                    deletearray @spell_goods_explain1$[1], getarraysize(@spell_goods_explain1$);
                    goto first;
                }
            }
            goto failed_deletearray;
            end;
        }
        else if(@list == 2)
        {
            //제한레벨
            setarray @spell_reqlvl[1], 0, 0, 10, 15, 18, 23, 25, 30, 32, 35
                        , 45, 50, 54, 62, 63, 65, 78, 80, 99, 99
                        , -1;

            //배우는 마법
            setarray @spell_list$[1], "성황령", "비영사천문", "누리의기원", "대지의기원", "이중공격"
                        , "신의축복", "후면공격", "측면공격", "삼중공격", "하늘의기원"
                        , "신검합일", "백호령", "유인", "사중공격", "건곤대나이"
                        , "검신검귀", "대력검신", "오중공격", "사자후전사", "동귀어진"
                        , "Error";

            //배우는 마법 설명
            setarray @spell_explain$[1], "캐릭터가 죽었을 경우 사용하면 자신이 해당된 성내 '성황당'으로 이동하게 되는 마법이네 "
                        , "이 기술은 동, 서, 남, 북으로 순식간에 이동할 수 있는 마법으로 필수적이면서도 가장 기본적인 마법이네 "
                        , "자신의 체력을 약간 회복하는 기술로써, 자신의 체력만 회복 가능하여 그다지 중요하지 않은 기술이네 "
                        , "약간의 체력을 회복하는 기술로써, 타인의 체력도 회복해 줄 수 있는 유요한 기술로 보이지 않는 적의 위치도 감지 할 수 있는 유용한 기술이네 "
                        , "순간적으로 두번 연속 공격하는 기술로써, 기본공격이 약 2배 증가되는 마법이네 "
                        , "일반 공격의 명중률을 일정시간 동안 높히는 기술로써, 전사의 중요한 기술이네 "
                        , "앞뒤의 적을 동시에 공격할 수 있는 기술로써, 전사의 중요한 기술이네 "
                        , "동시에 좌우에 있는 적을 공격 할 수 있는 기술로써 전사의 중요한 기술이네 "
                        , "순간적으로 세 번 연속 공격하는 기술로써, 굉장히 유용한 기술이라네 공격력이 약 3배 정도 증가하는 기술이라네 "
                        , "자신의 체력을 200만큼 회복하는 기술로써 자신의 체력만 회복 할 수 있는 기술이라네 "
                        , "검에 자신의 혼을 불어넣어 파괴력을 증가시키는 기술로써, 공격력을 향상시키는 중요한 기술이라네 "
                        , "호랑이의 혼을 빌어 순간적으로 파괴력을 증가 시키는 전사의 중요한 기술이라네 "
                        , "타인과 전투중이 아닌 괴수로 하여금 일정시간동안 자신만을 바라보게하도록 하는 기술로써 중요한 기술이라네 "
                        , "순간적으로 네번 연속 공격하는 기술로써, 기본공격이 약 4배 증가되는 마법이네 "
                        , "자신의 체력의 대부분을 사용, 정면의 상대에게 치명적이고 순간적인 피해를 입히는 전사의 필살기라네 "
                        , "검에 혼을 불어넣어 공격력을 강화시키는 마법으로써 전사의 중요한 기술이라네 "
                        , "검에 혼을 불어넣어 공격력을 강화시키는 마법으로써 전사의 중요한 기술이라네 "
                        , "순간적으로 다섯번 연속 공격하는 기술로써, 기본공격이 약 5배 증가되는 기술이네 "
                        , "목소리에 공력을 실어 크게 외치는 기술로써, 서버내 모든 유저들이 자신의 목소리를 들을 수 있게 해주는 기술이네 "
                        , "자신의 모든 체력을 사용해 정면의 상대에게 치명적인 피해를 입히는 전사의 최고 필살 기술이네 "
                        , "Error";

            //그냥 어법에맞게 을, 를 뜰지하는거
            setarray @spell_goods_message$[1], "을", "을", "을", "을", "을"
                            , "을", "을", "을", "을", "을"
                            , "을", "을", "를", "을", "를"
                            , "를", "을", "을", "를", "을"
                            , "Error";

            //재물정보
            setarray @spell_goods$[1], " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 30전과 도토리 70개를 나에게 바쳐야 하느니라."
                         , " 배우기 위해서는 금전 40전과 도토리 70개, 사슴고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과 도토리 80개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 80전과 도토리 80개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과 도토리 80개, 뱀고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 150전과 도토리 80개, 뱀고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 300전과 도토리 80개, 호랑이고기 15개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 200전과 도토리 80개, 웅담 30개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 500전과 도토리 50개, 여우모피 10개, 철도 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 500전과 도토리 50개, 자호의가죽 2개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 500전과 도토리 50개, 자호의가죽 2개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 900전과 도토리 80개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 1500전과 도토리 100개, 현철중검 2개, 호박 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 1500전과 도토리 80개, 철도 1개, 자호의가죽 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 3000전과 도토리 80개, 현철중검 1개, 호박 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 3500전과 도토리 100개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 5000전과 도토리 100개, 진호박 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 10000전과 도토리 100개, 환두대도 1개, 진호박 20개를 나에게 바쳐야 하느니라."
                        , "Error";


            //재물종류(돈제외)
            setarray @spell_goods_number[1], 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 3, 3, 3, 1, 2, 3, -1;

            //재물아이템
            setarray @spell_goods_item_1$[1], "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리"
                            , "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리"
                            , "도토리", "도토리", "Error";

            setarray @spell_goods_item_2$[1], "토끼고기", "토끼고기", " ", " ", " ", " ", "뱀고기", "뱀고기", "호랑이고기", "웅담", "여우모피", "자호의가죽", "자호의가죽"
                            , " ", "현철중검", "철도", "현철중검", " ", "진호박", "환두대도", "Error";

            setarray @spell_goods_item_3$[1], " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "호박", "자호의가죽", "호박"
                            , " ", " ", "진호박", "Error";

            //재물금전
            setarray @spell_goods_money[1], 0, 0, 30, 40, 100, 80, 100, 150, 300, 200, 500, 500, 500, 900, 1500, 1500, 3000
                        , 3500, 5000, 10000, -1;

            //재물갯수
            setarray @spell_goods_value_1[1], 10, 10, 70, 70, 80, 80, 80, 80, 80, 80, 50, 50, 50, 80, 100, 80, 80, 100, 100, 100, -1;
            setarray @spell_goods_value_2[1], 10, 10, 0, 0, 0, 0, 20, 20, 15, 30, 10, 2, 2, 0, 2, 1, 1, 0, 20, 1, -1;
            setarray @spell_goods_value_3[1], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 10, 0, 0, 20, -1;
                    
            //체크후 리스트에 뜰 마법
            setarray @spell_list1$, 20;
            //체크후 리스트에 뜰 재료
            setarray @spell_goods1$, 20;
            //체크후 그냥 을,를 중에 뭐띄울지
            setarray @spell_goods_message1$, 20;
            //체크후 대화창에 뜰 설명
            setarray @spell_explain1$, 20;
            //체크후 필요한 재물 종류
            setarray @spell_goods_number1, 20;
            //체크후 필요한 재물 아이템
            setarray @spell_goods_item1$, 20;
            setarray @spell_goods_item2$, 20;
            setarray @spell_goods_item3$, 20;
            //체크후 필요한 금전
            setarray @spell_goods_money1, 20;
            //체크후 필요한 재물 갯수
            setarray @spell_goods_value1, 20;
            setarray @spell_goods_value2, 20;
            setarray @spell_goods_value3, 20;
            //체크할수 있는 변수
            set @count, 0; // 마법 리스트에 관한 변수
            set @s_count, 0; // 마법 몇개 배울수 있는지
        
            //초기화
            for(set @loop, 1; @loop < 21; set @loop, @loop+1)
            {
                set @spell_list1$[@loop], " ";
                set @spell_goods1$[@loop], " ";
                set @spell_goods_message1$[@loop], " ";
                set @spell_explain1$[@loop], " ";
                set @spell_goods_number1[@loop], 0;
                set @spell_goods_item1$[@loop], " ";
                set @spell_goods_item2$[@loop], " ";
                set @spell_goods_item3$[@loop], " ";
                set @spell_goods_money1[@loop], 0;
                set @spell_goods_value1[@loop], 0;
                set @spell_goods_value2[@loop], 0;
                set @spell_goods_value3[@loop], 0;
            }

            for(set @loop, 1; @loop < 22; set @loop, @loop+1)
            {
                set @count, @count+1;
                if(@count == 21 || getarraysize(@spell_list$)-1 == @loop) { break; }
                else if(get_level(@sd) >= @spell_reqlvl[@loop] && checkspell(name2spellid(@spell_list$[@loop])) == 52)
                {
                    if(!strcmp(@spell_list1$[@count], " ")) { set @spell_list1$[@count], @spell_list$[@loop]; }
                    if(!strcmp(@spell_goods1$[@count], " ")) { set @spell_goods1$[@count], @spell_goods$[@loop]; }
                    if(!strcmp(@spell_goods_message1$[@count], " ")) { set @spell_goods_message1$[@count], @spell_goods_message$[@loop]; }
                    if(!strcmp(@spell_explain1$[@count], " ")) { set @spell_explain1$[@count], @spell_explain$[@loop]; }
                    if(!strcmp(@spell_goods_item1$[@count], " ")) { set @spell_goods_item1$[@count], @spell_goods_item_1$[@loop]; }
                    if(!strcmp(@spell_goods_item2$[@count], " ")) { set @spell_goods_item2$[@count], @spell_goods_item_2$[@loop]; }
                    if(!strcmp(@spell_goods_item3$[@count], " ")) { set @spell_goods_item3$[@count], @spell_goods_item_3$[@loop]; }
                    if(@spell_goods_number1[@loop] == 0) { set @spell_goods_number1[@count], @spell_goods_number[@loop]; }
                    if(@spell_goods_money1[@loop] == 0) { set @spell_goods_money1[@count], @spell_goods_money[@loop]; }
                    if(@spell_goods_value1[@loop] == 0) { set @spell_goods_value1[@count], @spell_goods_value_1[@loop]; }
                    if(@spell_goods_value2[@loop] == 0) { set @spell_goods_value2[@count], @spell_goods_value_2[@loop]; }
                    if(@spell_goods_value3[@loop] == 0) { set @spell_goods_value3[@count], @spell_goods_value_3[@loop]; }
                    if(!strcmp(@spell_list1$[@count], " ") || !strcmp(@spell_goods1$[@count], " ")) { set @count, @count-1; }
                    set @s_count, @s_count+1;
                }
                else { set @count, @count-1; }
            }

            set @message$, "자네 수준이라면 이런\\n마법들을 배울 수 있겠군.";
            if(@s_count == 0) { set @ret, dialog(0, 0, 0, "내가 가르칠 마법이 없구려..."); goto failed_deletearray; end; }
            else if(@s_count == 1) { set @addspell, list1(@message$, @spell_list1$[1]); }
            else if(@s_count == 2) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2]); }
            else if(@s_count == 3) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3]); }
            else if(@s_count == 4) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4]); }
            else if(@s_count == 5) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5]); }
            else if(@s_count == 6) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6]); }
            else if(@s_count == 7) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7]); }
            else if(@s_count == 8) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8]); }
            else if(@s_count == 9) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9]); }
            else if(@s_count == 10) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10]); }
            else if(@s_count == 11) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11]); }
            else if(@s_count == 12) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12]); }
            else if(@s_count == 13) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13]); }
            else if(@s_count == 14) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14]); }
            else if(@s_count == 15) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15]); }
            else if(@s_count == 16) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16]); }
            else if(@s_count == 17) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17]); }
            else if(@s_count == 18) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18]); }
            else if(@s_count == 19) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19]); }
            else if(@s_count == 20) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20]); }

            if(@addspell != 0) 
            {
                if(!strcmp(@spell_list$[@addspell], " "))
                {
                    goto failed_deletearray;
                    end;
                }
                set @pos, list2(@ans, 1, 0, @spell_explain1$[@addspell] + @spell_list1$[@addspell] + @spell_goods_message1$[@addspell] + @spell_goods1$[@addspell], "예", "아니오"); 
                if(@pos == 0) { goto failed_deletearray; end; }
                if(@ans == 1) 
                {
                    set @failed_message$, "필요한 것들을 구해보도록 하게. 자네라면 할 수 있겠지?";
                    if(@spell_goods_number1[@addspell] == 1)
                    {
                        if(countitem(name2itemid(@spell_goods_item1$[@addspell])) < @spell_goods_value1[@addspell])
                        {
                            set @pos, dialog(0, 0, 0, @failed_message$);
                            goto failed_deletearray;
                            end;
                        }
                    }
                    else if(@spell_goods_number1[@addspell] == 2)
                    {
                        if(countitem(name2itemid(@spell_goods_item1$[@addspell])) < @spell_goods_value1[@addspell] || countitem(name2itemid(@spell_goods_item2$[@addspell])) < @spell_goods_value2[@addspell])
                        {
                            set @pos, dialog(0, 0, 0, @failed_message$);
                            goto failed_deletearray;
                            end;
                        }
                    }
                    else if(@spell_goods_number1[@addspell] == 3)
                    {
                        if(countitem(name2itemid(@spell_goods_item1$[@addspell])) < @spell_goods_value1[@addspell] || countitem(name2itemid(@spell_goods_item2$[@addspell])) < @spell_goods_value2[@addspell] || countitem(name2itemid(@spell_goods_item3$[@addspell])) < @spell_goods_value3[@addspell])
                        {
                            set @pos, dialog(0, 0, 0, @failed_message$);
                            goto failed_deletearray;
                            end;
                        }
                    }

                    if(@spell_goods_money1[@addspell] > 0)
                    {
                        if(get_money(@sd) < @spell_goods_money1[@addspell])
                        {
                            set @pos, dialog(0, 0, 0, @failed_message$);
                            goto failed_deletearray;
                            end;
                        }
                        else
                        {
                            set_money @sd, get_money(@sd)-@spell_goods_money1[@addspell];
                            updatestatus @sd, 2;
                        }
                    }

                    if(@spell_goods_number1[@addspell] == 1)
                    {
                        for(set @loop, 0; @loop < @spell_goods_value1[@addspell]; set @loop, @loop+1) delitem @sd, name2itemid(@spell_goods_item1$[@addspell]), 1;
                        message @sd, 3, @spell_goods_item1$[@addspell] + " 주었습니다";
                    }
                    else if(@spell_goods_number1[@addspell] == 2)
                    {
                        for(set @loop, 0; @loop < @spell_goods_value1[@addspell]; set @loop, @loop+1) delitem @sd, name2itemid(@spell_goods_item1$[@addspell]), 1;
                        for(set @loop, 0; @loop < @spell_goods_value2[@addspell]; set @loop, @loop+1) delitem @sd, name2itemid(@spell_goods_item2$[@addspell]), 1;
                        message @sd, 3, @spell_goods_item1$[@addspell] + " 주었습니다";
                        message @sd, 3, @spell_goods_item2$[@addspell] + " 주었습니다";
                    }
                    else if(@spell_goods_number1[@addspell] == 3)
                    {
                        for(set @loop, 0; @loop < @spell_goods_value1[@addspell]; set @loop, @loop+1) delitem @sd, name2itemid(@spell_goods_item1$[@addspell]), 1;
                        for(set @loop, 0; @loop < @spell_goods_value2[@addspell]; set @loop, @loop+1) delitem @sd, name2itemid(@spell_goods_item2$[@addspell]), 1;
                        for(set @loop, 0; @loop < @spell_goods_value3[@addspell]; set @loop, @loop+1) delitem @sd, name2itemid(@spell_goods_item3$[@addspell]), 1;
                        message @sd, 3, @spell_goods_item1$[@addspell] + " 주었습니다";
                        message @sd, 3, @spell_goods_item2$[@addspell] + " 주었습니다";
                        message @sd, 3, @spell_goods_item3$[@addspell] + " 주었습니다";
                    }

                    addspell name2spellid(@spell_list1$[@addspell]); 
                    set @re_magic_list, list1(@spell_list1$[@addspell]+" 가르쳐주었네\\n", "처음으로");
                    if(@re_magic_list == 1)
                    {
                        deletearray @spell_reqlvl[1], getarraysize(@spell_reqlvl);
                        deletearray @spell_list1[1], getarraysize(@spell_list1);
                        deletearray @spell_goods1$[1], getarraysize(@spell_goods1$);
                        deletearray @spell_goods_message$[1], getarraysize(@spell_goods_message$);
                        deletearray @spell_goods_message1$[1], getarraysize(@spell_goods_message1$);
                        deletearray @spell_list$[1], getarraysize(@spell_list$);
                        deletearray @spell_goods$[1], getarraysize(@spell_goods$);
                        deletearray @spell_goods_explain$[1], getarraysize(@spell_goods_explain$);
                        deletearray @spell_goods_number[1], getarraysize(@spell_goods_number);
                        deletearray @spell_goods_item_1$[1], getarraysize(@spell_goods_item_1$);
                        deletearray @spell_goods_item_2$[1], getarraysize(@spell_goods_item_2$);
                        deletearray @spell_goods_item_3$[1], getarraysize(@spell_goods_item_3$);
                        deletearray @spell_goods_money[1], getarraysize(@spell_goods_money);
                        deletearray @spell_goods_value_1[1], getarraysize(@spell_goods_vlaue_1);
                        deletearray @spell_goods_value_2[1], getarraysize(@spell_goods_vlaue_2);
                        deletearray @spell_goods_value_3[1], getarraysize(@spell_goods_vlaue_3);
                        deletearray @spell_goods_explain1$[1], getarraysize(@spell_goods_explain1$);
                        deletearray @spell_goods_number1[1], getarraysize(@spell_goods_number1);
                        deletearray @spell_goods_item1$[1], getarraysize(@spell_goods_item1$);
                        deletearray @spell_goods_item2$[1], getarraysize(@spell_goods_item2$);
                        deletearray @spell_goods_item3$[1],  getarraysize(@spell_goods_item3$);
                        deletearray @spell_goods_money1[1], getarraysize(@spell_goods_money1);
                        deletearray @spell_goods_value1[1], getarraysize(@spell_goods_vlaue1);
                        deletearray @spell_goods_value2[1], getarraysize(@spell_goods_vlaue2);
                        deletearray @spell_goods_value3[1], getarraysize(@spell_goods_vlaue3);
                        goto first;
                    }
                    else
                    {
                        goto failed_deletearray;
                        end;
                    }                        
                } 
                else if(@ans == 2) 
                {
                    set @failed, list1("이 모든 것은 네 탓이니,\\n다음에 이 곳에 올 때는 더\\n굳은 각오를 가지고 오도록\\n하거라."); 
                    goto failed_deletearray;
                    end;
                }
            }
            goto failed_deletearray;
            end;
        }
        else if(@list == 3) 
        { 
            set @message$, "지금 네가 지울 수 있는\\n마법은 다음과 같단다. 다시\\n한 번 심사 숙고 하고\\n지우도록 하여라.";
            set @delslot, listspell(@message$);
            delspell @delslot;
            set @re_magic_list, list1("배움의 길은 끝이 없으니\\n더더욱 노력하는 자세를\\n갖도록 하거라", "처음으로");
            if(@re_magic_list==1) { goto first; }
        }
        else if(@list == 4)
        {
            callfunc "JOBTITLE";
        }
        else end;
    }
    else
    {
        set @list, list1("험난한 길을 걷는 수행자여,\\n무슨 일로 저를 찾으셨소?", "전사직업가지기");
        if(@list == 1) { set @ret, dialog(0, 0, 1, "이미 직업이 있지 않느냐?\\n한번 선택한 직업은 바꿀 수\\n없느니라."); }
    }

failed_deletearray:
    deletearray @spell_reqlvl[1], getarraysize(@spell_reqlvl);
    deletearray @spell_list1[1], getarraysize(@spell_list1);
    deletearray @spell_goods1$[1], getarraysize(@spell_goods1$);
    deletearray @spell_list$[1], getarraysize(@spell_list$);
    deletearray @spell_goods$[1], getarraysize(@spell_goods$);
    deletearray @spell_goods_message$[1], getarraysize(@spell_goods_message$);
    deletearray @spell_goods_message1$[1], getarraysize(@spell_goods_message1$);
    deletearray @spell_goods_explain$[1], getarraysize(@spell_goods_explain$);
    deletearray @spell_goods_number[1], getarraysize(@spell_goods_number);
    deletearray @spell_goods_item_1$[1], getarraysize(@spell_goods_item_1$);
    deletearray @spell_goods_item_2$[1], getarraysize(@spell_goods_item_2$);
    deletearray @spell_goods_item_3$[1], getarraysize(@spell_goods_item_3$);
    deletearray @spell_goods_money[1], getarraysize(@spell_goods_money);
    deletearray @spell_goods_value_1[1], getarraysize(@spell_goods_vlaue_1);
    deletearray @spell_goods_value_2[1], getarraysize(@spell_goods_vlaue_2);
    deletearray @spell_goods_value_3[1], getarraysize(@spell_goods_vlaue_3);
    deletearray @spell_goods_explain1$[1], getarraysize(@spell_goods_explain1$);
    deletearray @spell_goods_number1[1], getarraysize(@spell_goods_number1);
    deletearray @spell_goods_item1$[1], getarraysize(@spell_goods_item1$);
    deletearray @spell_goods_item2$[1], getarraysize(@spell_goods_item2$);
    deletearray @spell_goods_item3$[1],  getarraysize(@spell_goods_item3$);
    deletearray @spell_goods_money1[1], getarraysize(@spell_goods_money1);
    deletearray @spell_goods_value1[1], getarraysize(@spell_goods_vlaue1);
    deletearray @spell_goods_value2[1], getarraysize(@spell_goods_vlaue2);
    deletearray @spell_goods_value3[1], getarraysize(@spell_goods_vlaue3);
}