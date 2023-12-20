{
    set @id, myid();
    set @sd, mysd();
    image 0, 41, 11;
    if(get_job(@sd) == 0 && get_level(@sd) >= 5)
    {
        callfunc "JOBBIGIN", 5, "술사", 3, 40, 11;
    }
    else if(get_job(@sd) == 0 && get_level(@sd) < 5) { set @ret, dialog(0, 0, 1, "아직 너의 정성이 부족하니\\n다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를\\n찾아오도록 하여라."); end; }
    else if(get_job(@sd) == 3 || get_job(@sd) == 5)
    {
first:
        set @list, list1("험난한 길을 걷는 수행자여,\\n무슨 일로 저를 찾으셨소?", "마법 알아보기", "마법 배우기", "마법 지우기", "칭호 받기");
        if(@list == 1)
        {
            //제한레벨
            setarray @spell_reqlvl[1], 0, 0, 5, 6, 8, 10, 13, 14, 16, 16
                        , 17, 18, 19, 20, 23, 24, 25, 27, 30, 31
                        , 31, 32, 34, 35, 36, 40, 43, 45, 50, 53
                        , 55, 58, 60, 62, 65, 74, 85, 99, 99
                        , -1;

            //배운 마법
            setarray @spell_list$[1], "성황령", "비영사천문", "누리의기원", "-1", "대지의기원"
                        , "무장", "귀환", "-2", "바다의기원", "-3"
                        , "누리의힘", "동해의기원", "대지의힘", "출두", "하늘의기원"
                        , "-4", "해독", "천공의기원", "소환", "활력"
                        , "-5", "보호", "공력증강", "구름의기원", "-6"
                        , "야수", "퇴마주", "-7", "맹수", "중독"
                        , "저주", "마비", "절망", "-8", "혼돈"
                        , "-9", "-10", "사자후술사", "헬파이어"
                        , "Error";

            //배우는 마법 설명
            setarray @spell_explain$[1], "캐릭터가 죽었을 경우 사용하면 자신이 해당된 성내 '성황당'으로 이동하게 되는 마법이네 "
                        , "이 기술은 동, 서, 남, 북으로 순식간에 이동할 수 있는 마법으로 필수적이면서도 가장 기본적인 마법이네 "
                        , "자신의 체력을 약간 회복하는 기술로써, 자신의 체력만 회복 가능하여 그다지 중요하지 않은 기술이네 "
                        , "신수 속성을 가진 원거리 마법으로써, 멀리서도 적을 공격할 수 있는 기술이네 "
                        , "약간의 체력을 회복하는 기술로써, 타인의 체력도 회복해 줄 수 있는 유요한 기술로 보이지 않는 적의 위치도 감지 할 수 있는 유용한 기술이네 "
                        , "자신이나 타인의 갑옷 강도를 일정시간 높여 피해를 줄이는 마법이네 "
                        , "자신의 나라 주막으로 이동하는 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "자신의 체력을 100만큼 회복하는 기술로써, 기초적인 회복마법이라네 "
                        , "첫번째 신수마법의 다음 단계 기술로써, 적에게 더욱 강력한 공격을 가할수 있는 마법이네 "
                        , "자신의 마력을 불어넣어 힘을 일정시간 동안 높이는 마법이네 "
                        , "자신이나 타인의 체력을 100만큼 회복시키는 마법이네 "
                        , "자신이나 타인에게 마력을 불어넣어 힘을 일정시간 동안 높히는 기술이네 "
                        , "자신보다 낮은 레벨의 사람에게 순간적으로 이동하는 마법이네 "
                        , "자신의 체력을 200만큼 회복시키는 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 한층 강해진 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "중독을 푸는 회복 계열의 마법이네 "
                        , "자신이나 타인의 체력을 200만큼 회복시키는 마법이네 "
                        , "자신보다 레벨이 낮은 사람을 자신의 앞으로 불러오는 마법이네 "
                        , "마비를 푸는 회복계열의 마법이네 "
                        , "두번째 신수마법의 다음 단계 기술로써, 적에게 더욱 강력한 공격을 가할수 있는 마법이네 "
                        , "일정시간 동안 모든 공격으로부터의 피해를 절반으로 줄여주는 마법이네 "
                        , "소량의 마력과 일부 체력을 소모해, 마력을 완전히 회복시키는 마법이네 "
                        , "자신이나 타인의 체력을 500만큼 회복시켜주는 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 한층 강해진 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "여우,늑대로 변신하는 도적의 야수와 동일한 효과를 가지는 마법이네 "
                        , "저주를 푸는 회복계열의 마법이네 "
                        , "세번째 신수마법의 다음 단계 기술로써, 적에게 더욱 강력한 공격을 가할수 있는 마법이네 "
                        , "곰,호랑이로 변신하는 마법이네 "
                        , "대상을 맹독에 노출시켜 중독 상태로 만드는 저주 계열의 마법이네 "
                        , "대상을 저주하여 방어력을 낮추는 저주 계열의 마법이네 "
                        , "상대를 얼려 움직이지 못하게 만드는 저주 계열의 마법이네 "
                        , "상대의 시력을 일정시간 동안 빼앗는 저주 계열의 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 한층 가해진 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "희생자의 정신을 혼란시켜 아군을 공격하게 만드는 마법이네 "
                        , "신수 속성을 가진 주술사의 최강의 원거리 공격 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 한층 강해진 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "목소리에 공력을 실어 크게 외치는 기술로써, 서버내 모든 유저들이 자신의 목소리를 들을 수 있게 해주는 기술이네 "
                        , "주술사의 필살기로 모든 마력을 순간적으로 소모해 그에 비례하는 피해를 입히는 마법이네 "
                        , "Error";

            //그냥 어법에맞게 을, 를 뜰지하는거
            setarray @spell_goods_message$[1], "을", "을", "을", "를", "을"
                            , "을", "을", "을", "을", "를"
                            , "을", "을", "을", "를", "을"
                            , "를", "을", "을", "을", "을"
                            , "를", "를", "을", "을", "을"
                            , "를", "를", "를", "를", "을"
                            , "를", "를", "을", "을", "을"
                            , "를", "을", "를", "를"
                            , "Error";

            //재물정보
            setarray @spell_goods$[1]," 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 80전과, 도토리 60개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 30개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 50개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 70전과, 도토리 50개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 70개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 70개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 50개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 70개, 뱀고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 50개, 뱀고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 50개, 뱀고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 30개, 곰가죽 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 60개, 녹용 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 80개, 녹용 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 80개, 뱀고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 60개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 70개, 호랑이고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 200전과, 도토리 70개, 호랑이고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 100개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 100개, 웅담 10개, 여우모피 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 100개, 여우모피 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 200전과, 도토리 50개, 여우모피 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 200전과, 도토리 50개, 여우모피 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 500전과, 도토리 50개, 철도 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 70개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 2000전과, 도토리 70개, 좋은뱀고기 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 250전과, 도토리 70개, 자호의가죽 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 300전과, 도토리 70개, 자호의가죽 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 400전과, 도토리 80개, 호박 1개를 나에게 바쳐야 하느니라." 
                        , " 배우기 위해서는 금전 500전과, 도토리 70개, 호박 1개를 나에게 바쳐야 하느니라." 
                        , " 배우기 위해서는 금전 600전과, 도토리 90개, 호박 2개, 해골죽장 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 2000전과, 도토리 100개, 진호박 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 2000전과, 도토리 100개, 진호박 2개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 5000전과, 도토리 100개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 5000전과, 도토리 100개, 진호박 10개, 대마령봉 1개를 나에게 바쳐야 하느니라."
                        , "Error";

            //체크후 리스트에 뜰 마법
            setarray @spell_list1$, 39;
            //체크후 리스트에 뜰 재료
            setarray @spell_goods1$, 39;
            //체크후 그냥 을,를 중에 뭐띄울지
            setarray @spell_goods_message1$, 39;
            //체크후 대화창에 뜰 설명
            setarray @spell_explain1$, 39;
            //체크할수 있는 변수
            set @count, 0; // 마법 리스트에 관한 변수
            set @s_count, 0; // 마법 몇개 배울수 있는지
        
            //초기화
            for (set @loop, 1; @loop < 40; set @loop, @loop+1)
            {
                set @spell_list1$[@loop], " ";
                set @spell_goods1$[@loop], " ";
                set @spell_goods_message1$[@loop], " ";
                set @spell_explain1$[@loop], " ";
            }

            //먼저 spell_list$에 재대로 넣는다.
            switch(get_totem(@sd))
            {
            case 0:
                set @spell_list$[4], "화염주"; set @spell_list$[8], "화염주'첨"; set @spell_list$[10], "화영열주"; set @spell_list$[16], "화영열주'첨"; set @spell_list$[21], "화열참주"; set @spell_list$[25], "화열참주'첨"; set @spell_list$[28], "진화열참주"; set @spell_list$[34], "진화열참주'첨"; set @spell_list$[36], "극진화열참주"; set @spell_list$[37], "극진화열참주'첨";
                break;
            case 1:
                set @spell_list$[4], "백열주"; set @spell_list$[8], "백열주'첨"; set @spell_list$[10], "백령주"; set @spell_list$[16], "백령주'첨"; set @spell_list$[21], "백열참주"; set @spell_list$[25], "백열참주'첨"; set @spell_list$[28], "진백열참주"; set @spell_list$[34], "진백열참주'첨"; set @spell_list$[36], "극진백열참주"; set @spell_list$[37], "극진백열참주'첨";
                break;
            case 2:
                set @spell_list$[4], "자무주"; set @spell_list$[8], "자무주'첨"; set @spell_list$[10], "자영무주"; set @spell_list$[16], "자영무주'첨"; set @spell_list$[21], "자천무주"; set @spell_list$[25], "자천무주'첨"; set @spell_list$[28], "진자천무주"; set @spell_list$[34], "진자천무주'첨"; set @spell_list$[36], "극진자천무주"; set @spell_list$[37], "극진자천무주'첨";
                break;
            case 3:
                set @spell_list$[4], "뢰진주"; set @spell_list$[8], "뢰진주'첨"; set @spell_list$[10], "뢰격주"; set @spell_list$[16], "뢰격주'첨"; set @spell_list$[21], "뢰격참주"; set @spell_list$[25], "뢰격참주'첨"; set @spell_list$[28], "진뢰격참주"; set @spell_list$[34], "진뢰격참주'첨"; set @spell_list$[36], "극진뢰격참주"; set @spell_list$[37], "극진뢰격참주'첨";
                break;
            }

            for (set @loop, 1; @loop < 40; set @loop, @loop+1)
            {
                set @count, @count+1;
                if(@count == 39 || getarraysize(@spell_list$)-1 == @loop) { break; }
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

            set @message$, "자네 수준이라면 이런\\n마법들을 알아볼 수 있겠군.";
            if(@count == 0) { set @ret, dialog(0, 0, 0, "내가 알려줄 마법이 없구려..."); goto failed_deletearray; end; }
            else if(@count == 1) { set @addspell, list1(@message$, @spell_list1$[1]); }
            else if(@count == 2) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2]); }
            else if(@count == 3) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3]); }
            else if(@count == 4) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4]); }
            else if(@count == 5) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5]); }
            else if(@count == 6) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6]); }
            else if(@count == 7) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7]); }
            else if(@count == 8) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8]); }
            else if(@count == 9) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9]); }
            else if(@count == 10) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10]); }
            else if(@count == 11) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11]); }
            else if(@count == 12) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12]); }
            else if(@count == 13) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13]); }
            else if(@count == 14) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14]); }
            else if(@count == 15) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15]); }
            else if(@count == 16) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16]); }
            else if(@count == 17) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17]); }
            else if(@count == 18) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18]); }
            else if(@count == 19) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19]); }
            else if(@count == 20) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20]); }
            else if(@count == 21) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21]); }
            else if(@count == 22) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22]); }
            else if(@count == 23) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23]); }
            else if(@count == 24) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24]); }
            else if(@count == 25) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25]); }
            else if(@count == 26) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26]); }
            else if(@count == 27) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27]); }
            else if(@count == 28) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28]); }
            else if(@count == 29) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29]); }
            else if(@count == 30) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30]); }
            else if(@count == 31) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31]); }
            else if(@count == 32) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32]); }
            else if(@count == 33) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33]); }
            else if(@count == 34) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34]); }
            else if(@count == 35) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35]); }
            else if(@count == 36) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35], @spell_list1$[36]); }
            else if(@count == 37) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35], @spell_list1$[36], @spell_list1$[37]); }
            else if(@count == 38) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35], @spell_list1$[36], @spell_list1$[37], @spell_list1$[38]); }
            else if(@count == 39) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35], @spell_list1$[36], @spell_list1$[37], @spell_list1$[38], @spell_list1$[39]); }

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
        }
        else if(@list == 2)
        {
            //제한레벨
            setarray @spell_reqlvl[1], 0, 0, 5, 6, 8, 10, 13, 14, 16, 16
                        , 17, 18, 19, 20, 23, 24, 25, 27, 30, 31
                        , 31, 32, 34, 35, 36, 40, 43, 45, 50, 53
                        , 55, 58, 60, 62, 65, 74, 85, 99, 99
                        , -1;

            //배운 마법
            setarray @spell_list$[1], "성황령", "비영사천문", "누리의기원", "-1", "대지의기원"
                        , "무장", "귀환", "-2", "바다의기원", "-3"
                        , "누리의힘", "동해의기원", "대지의힘", "출두", "하늘의기원"
                        , "-4", "해독", "천공의기원", "소환", "활력"
                        , "-5", "보호", "공력증강", "구름의기원", "-6"
                        , "야수", "퇴마주", "-7", "맹수", "중독"
                        , "저주", "마비", "절망", "-8", "혼돈"
                        , "-9", "-10", "사자후술사", "헬파이어"
                        , "Error";

            //배우는 마법 설명
            setarray @spell_explain$[1], "캐릭터가 죽었을 경우 사용하면 자신이 해당된 성내 '성황당'으로 이동하게 되는 마법이네 "
                        , "이 기술은 동, 서, 남, 북으로 순식간에 이동할 수 있는 마법으로 필수적이면서도 가장 기본적인 마법이네 "
                        , "자신의 체력을 약간 회복하는 기술로써, 자신의 체력만 회복 가능하여 그다지 중요하지 않은 기술이네 "
                        , "신수 속성을 가진 원거리 마법으로써, 멀리서도 적을 공격할 수 있는 기술이네 "
                        , "약간의 체력을 회복하는 기술로써, 타인의 체력도 회복해 줄 수 있는 유요한 기술로 보이지 않는 적의 위치도 감지 할 수 있는 유용한 기술이네 "
                        , "자신이나 타인의 갑옷 강도를 일정시간 높여 피해를 줄이는 마법이네 "
                        , "자신의 나라 주막으로 이동하는 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "자신의 체력을 100만큼 회복하는 기술로써, 기초적인 회복마법이라네 "
                        , "첫번째 신수마법의 다음 단계 기술로써, 적에게 더욱 강력한 공격을 가할수 있는 마법이네 "
                        , "자신의 마력을 불어넣어 힘을 일정시간 동안 높이는 마법이네 "
                        , "자신이나 타인의 체력을 100만큼 회복시키는 마법이네 "
                        , "자신이나 타인에게 마력을 불어넣어 힘을 일정시간 동안 높히는 기술이네 "
                        , "자신보다 낮은 레벨의 사람에게 순간적으로 이동하는 마법이네 "
                        , "자신의 체력을 200만큼 회복시키는 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 한층 강해진 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "중독을 푸는 회복 계열의 마법이네 "
                        , "자신이나 타인의 체력을 200만큼 회복시키는 마법이네 "
                        , "자신보다 레벨이 낮은 사람을 자신의 앞으로 불러오는 마법이네 "
                        , "마비를 푸는 회복계열의 마법이네 "
                        , "두번째 신수마법의 다음 단계 기술로써, 적에게 더욱 강력한 공격을 가할수 있는 마법이네 "
                        , "일정시간 동안 모든 공격으로부터의 피해를 절반으로 줄여주는 마법이네 "
                        , "소량의 마력과 일부 체력을 소모해, 마력을 완전히 회복시키는 마법이네 "
                        , "자신이나 타인의 체력을 500만큼 회복시켜주는 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 한층 강해진 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "여우,늑대로 변신하는 도적의 야수와 동일한 효과를 가지는 마법이네 "
                        , "저주를 푸는 회복계열의 마법이네 "
                        , "세번째 신수마법의 다음 단계 기술로써, 적에게 더욱 강력한 공격을 가할수 있는 마법이네 "
                        , "곰,호랑이로 변신하는 마법이네 "
                        , "대상을 맹독에 노출시켜 중독 상태로 만드는 저주 계열의 마법이네 "
                        , "대상을 저주하여 방어력을 낮추는 저주 계열의 마법이네 "
                        , "상대를 얼려 움직이지 못하게 만드는 저주 계열의 마법이네 "
                        , "상대의 시력을 일정시간 동안 빼앗는 저주 계열의 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 한층 가해진 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "희생자의 정신을 혼란시켜 아군을 공격하게 만드는 마법이네 "
                        , "신수 속성을 가진 주술사의 최강의 원거리 공격 마법이네 "
                        , "자신의 전후좌우 방향으로 동시에 한층 강해진 신수마법을 시전하는 마법으로 많은 적을 공격 할 수 있다네 "
                        , "목소리에 공력을 실어 크게 외치는 기술로써, 서버내 모든 유저들이 자신의 목소리를 들을 수 있게 해주는 기술이네 "
                        , "주술사의 필살기로 모든 마력을 순간적으로 소모해 그에 비례하는 피해를 입히는 마법이네 "
                        , "Error";

            //그냥 어법에맞게 을, 를 뜰지하는거
            setarray @spell_goods_message$[1], "을", "을", "을", "를", "을"
                            , "을", "을", "을", "을", "를"
                            , "을", "을", "을", "를", "을"
                            , "를", "을", "을", "을", "을"
                            , "를", "를", "을", "을", "을"
                            , "를", "를", "를", "를", "을"
                            , "를", "를", "을", "을", "을"
                            , "를", "을", "를", "를"
                            , "Error";

            //재물정보
            setarray @spell_goods$[1]," 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 10개, 토끼고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 80전과, 도토리 60개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 30개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 50개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 70전과, 도토리 50개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 70개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 70개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 50개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 70개, 뱀고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 50개, 뱀고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 50개, 뱀고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 30개, 곰가죽 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 60개, 녹용 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 80개, 녹용 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 50전과, 도토리 80개, 뱀고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 60개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 70개, 호랑이고기 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 200전과, 도토리 70개, 호랑이고기 20개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 100개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 도토리 100개, 웅담 10개, 여우모피 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 100개, 여우모피 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 200전과, 도토리 50개, 여우모피 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 200전과, 도토리 50개, 여우모피 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 500전과, 도토리 50개, 철도 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 100전과, 도토리 70개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 2000전과, 도토리 70개, 좋은뱀고기 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 250전과, 도토리 70개, 자호의가죽 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 300전과, 도토리 70개, 자호의가죽 10개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 400전과, 도토리 80개, 호박 1개를 나에게 바쳐야 하느니라." 
                        , " 배우기 위해서는 금전 500전과, 도토리 70개, 호박 1개를 나에게 바쳐야 하느니라." 
                        , " 배우기 위해서는 금전 600전과, 도토리 90개, 호박 2개, 해골죽장 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 2000전과, 도토리 100개, 진호박 1개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 2000전과, 도토리 100개, 진호박 2개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 5000전과, 도토리 100개를 나에게 바쳐야 하느니라."
                        , " 배우기 위해서는 금전 5000전과, 도토리 100개, 진호박 10개, 대마령봉 1개를 나에게 바쳐야 하느니라."
                        , "Error";

            //재물종류(돈제외)
            setarray @spell_goods_number[1], 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 3, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 3, 2, 2, 1, 3, -1;

            //재물아이템
            setarray @spell_goods_item_1$[1], "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리"
                            , "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리"
                            , "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리"
                            , "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리", "도토리"
                            , "도토리", "도토리", "도토리", "Error";

            setarray @spell_goods_item_2$[1], "토끼고기", "토끼고기", "토끼고기", "토끼고기", "토끼고기", " ", " ", " ", " ", " ", " ", " ", "뱀고기", "뱀고기", "뱀고기"
                            , "곰가죽", "녹용", "녹용", "뱀고기", " ", "호랑이고기", "호랑이고기", " ", "웅담", "여우모피", "여우모피", "여우모피"
                            , "철도", " ", "좋은뱀고기", "자호의가죽", "자호의가죽", "호박", "호박", "호박", "진호박" 
                            , "진호박", " ", "진호박", "Error"; 

            setarray @spell_goods_item_3$[1], " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " "
                            , " ", " ", " ", " ", " ", " ", " ", " ", "여우모피", " ", " ", " ", " ", " ", " "
                            , " ", " ", " ", " ", "해골죽장", " ", " ", " ", "대마령봉", "Error";

            //재물금전
            setarray @spell_goods_money[1], 0, 0, 0, 0, 0, 80, 50, 0, 70, 50, 50, 50, 100, 50, 50, 50, 100, 100, 50
                        , 100, 0, 200, 0, 0, 100, 200, 200, 500, 100, 200, 250, 300, 400, 500, 600
                        , 2000, 2000, 5000, 5000, -1;

            //재물갯수
            setarray @spell_goods_value_1[1], 10, 10, 10, 10, 10, 60, 30, 50, 50, 70, 70, 50, 70, 50, 50, 30, 60, 80, 80, 60, 70, 70, 100, 100, 100, 50, 50, 50, 70, 70, 70, 70, 80, 70, 90, 100, 100, 100, 100, -1;
            setarray @spell_goods_value_2[1], 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 20, 20, 10, 10, 10, 20, 10, 0, 10, 20, 0, 10, 10, 1, 10, 1, 0, 1, 10, 10, 1, 1, 2, 1, 2, 0, 10, -1;
            setarray @spell_goods_value_3[1], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, -1;
                    
            //체크후 리스트에 뜰 마법
            setarray @spell_list1$, 39;
            //체크후 리스트에 뜰 재료
            setarray @spell_goods1$, 39;
            //체크후 그냥 을,를 중에 뭐띄울지
            setarray @spell_goods_message1$, 39;
            //체크후 대화창에 뜰 설명
            setarray @spell_explain1$, 39;
            //체크후 필요한 재물 종류
            setarray @spell_goods_number1, 39;
            //체크후 필요한 재물 아이템
            setarray @spell_goods_item1$, 39;
            setarray @spell_goods_item2$, 39;
            setarray @spell_goods_item3$, 39;
            //체크후 필요한 금전
            setarray @spell_goods_money1, 39;
            //체크후 필요한 재물 갯수
            setarray @spell_goods_value1, 39;
            setarray @spell_goods_value2, 39;
            setarray @spell_goods_value3, 39;
            //체크할수 있는 변수
            set @count, 0; // 마법 리스트에 관한 변수
            set @s_count, 0; // 마법 몇개 배울수 있는지
        
            //초기화
            for (set @loop, 1; @loop < 40; set @loop, @loop+1)
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

            //먼저 spell_list$에 재대로 넣는다.
            switch(get_totem(@sd))
            {
            case 0:
                set @spell_list$[4], "화염주"; set @spell_list$[8], "화염주'첨"; set @spell_list$[10], "화영열주"; set @spell_list$[16], "화영열주'첨"; set @spell_list$[21], "화열참주"; set @spell_list$[25], "화열참주'첨"; set @spell_list$[28], "진화열참주"; set @spell_list$[34], "진화열참주'첨"; set @spell_list$[36], "극진화열참주"; set @spell_list$[37], "극진화열참주'첨";
                break;
            case 1:
                set @spell_list$[4], "백열주"; set @spell_list$[8], "백열주'첨"; set @spell_list$[10], "백령주"; set @spell_list$[16], "백령주'첨"; set @spell_list$[21], "백열참주"; set @spell_list$[25], "백열참주'첨"; set @spell_list$[28], "진백열참주"; set @spell_list$[34], "진백열참주'첨"; set @spell_list$[36], "극진백열참주"; set @spell_list$[37], "극진백열참주'첨";
                break;
            case 2:
                set @spell_list$[4], "자무주"; set @spell_list$[8], "자무주'첨"; set @spell_list$[10], "자영무주"; set @spell_list$[16], "자영무주'첨"; set @spell_list$[21], "자천무주"; set @spell_list$[25], "자천무주'첨"; set @spell_list$[28], "진자천무주"; set @spell_list$[34], "진자천무주'첨"; set @spell_list$[36], "극진자천무주"; set @spell_list$[37], "극진자천무주'첨";
                break;
            case 3:
                set @spell_list$[4], "뢰진주"; set @spell_list$[8], "뢰진주'첨"; set @spell_list$[10], "뢰격주"; set @spell_list$[16], "뢰격주'첨"; set @spell_list$[21], "뢰격참주"; set @spell_list$[25], "뢰격참주'첨"; set @spell_list$[28], "진뢰격참주"; set @spell_list$[34], "진뢰격참주'첨"; set @spell_list$[36], "극진뢰격참주"; set @spell_list$[37], "극진뢰격참주'첨";
                break;
            }

            for (set @loop, 1; @loop < 40; set @loop, @loop+1)
            {
                set @count, @count+1;
                if(@count == 39 || getarraysize(@spell_list$)-1 == @loop) { break; }
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
            if(@count == 0) { set @ret, dialog(0, 0, 0, "내가 가르칠 마법이 없구려..."); goto failed_deletearray; end; }
            else if(@count == 1) { set @addspell, list1(@message$, @spell_list1$[1]); }
            else if(@count == 2) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2]); }
            else if(@count == 3) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3]); }
            else if(@count == 4) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4]); }
            else if(@count == 5) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5]); }
            else if(@count == 6) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6]); }
            else if(@count == 7) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7]); }
            else if(@count == 8) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8]); }
            else if(@count == 9) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9]); }
            else if(@count == 10) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10]); }
            else if(@count == 11) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11]); }
            else if(@count == 12) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12]); }
            else if(@count == 13) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13]); }
            else if(@count == 14) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14]); }
            else if(@count == 15) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15]); }
            else if(@count == 16) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16]); }
            else if(@count == 17) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17]); }
            else if(@count == 18) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18]); }
            else if(@count == 19) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19]); }
            else if(@count == 20) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20]); }
            else if(@count == 21) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21]); }
            else if(@count == 22) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22]); }
            else if(@count == 23) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23]); }
            else if(@count == 24) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24]); }
            else if(@count == 25) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25]); }
            else if(@count == 26) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26]); }
            else if(@count == 27) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27]); }
            else if(@count == 28) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28]); }
            else if(@count == 29) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29]); }
            else if(@count == 30) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30]); }
            else if(@count == 31) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31]); }
            else if(@count == 32) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32]); }
            else if(@count == 33) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33]); }
            else if(@count == 34) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34]); }
            else if(@count == 35) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35]); }
            else if(@count == 36) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35], @spell_list1$[36]); }
            else if(@count == 37) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35], @spell_list1$[36], @spell_list1$[37]); }
            else if(@count == 38) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35], @spell_list1$[36], @spell_list1$[37], @spell_list1$[38]); }
            else if(@count == 39) { set @addspell, list1(@message$, @spell_list1$[1], @spell_list1$[2], @spell_list1$[3], @spell_list1$[4], @spell_list1$[5], @spell_list1$[6], @spell_list1$[7], @spell_list1$[8], @spell_list1$[9], @spell_list1$[10], @spell_list1$[11], @spell_list1$[12], @spell_list1$[13], @spell_list1$[14], @spell_list1$[15], @spell_list1$[16], @spell_list1$[17], @spell_list1$[18], @spell_list1$[19], @spell_list1$[20], @spell_list1$[21], @spell_list1$[22], @spell_list1$[23], @spell_list1$[24], @spell_list1$[25], @spell_list1$[26], @spell_list1$[27], @spell_list1$[28], @spell_list1$[29], @spell_list1$[30], @spell_list1$[31], @spell_list1$[32], @spell_list1$[33], @spell_list1$[34], @spell_list1$[35], @spell_list1$[36], @spell_list1$[37], @spell_list1$[38], @spell_list1$[39]); }

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
                    else { goto failed_deletearray; end; }
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
        set @list, list1("험난한 길을 걷는 수행자여,\\n무슨 일로 저를 찾으셨소?", "술사직업가지기");
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