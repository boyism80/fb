{
    set @sd, mysd();
    set @id, myid();

    if($dosirak == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "곧 우리 아버님이 환갑이 되시지. 아~ 이 못난 아들을 기르시느라 정말 고생 많이 하셨지. 이젠 내가 호강시켜 드려야 할 텐데... 오래오래 사셨으면 좋겠어.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "환갑잔치에 쓸 요리가 필요한데, 변변한 요리사가 없어서 말이야. 환갑잔치는 평생 한번 있는 경사인데, 그런 날에 맛없는 요리를 드셔서야 안 되지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, list2(@list, 0, 1, "아버님께 맛있는 것을 많이 드시게 하고 싶은데, 요리사를 구할 수가 없으니 참 큰일이야..", "저도 요리를 만들 줄 아는데, 도와드릴까요?", "찬찬에게 맡겨보시지 그러세요?");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
        else if(@list == 1)
        {
            set $dosirak, 1;
COS004:
            set @pos, dialog(0, 0, 1, "자네가 도와주겠다고? 허허! 그거 듣던 중 반가운 소리구만! 실은 자네에게 이런 이야기를 한 것도 자네를 유심히 보니 요리 깨나 해 본 사람 같아서였지. 허허허..");
            if(@pos == 0) end;
COS005:
            set @pos, dialog(0, 1, 1, "그래, 어떤 요리든 좋네. 맛있는 요리라면 뭐든지 좋아. 요리를 가져오면 요리가 얼마나 맛있는가에 따라 사례를 하도록 하겠네.\\n\\n자자, 이러고 있지 말고 서둘러 주게. 난 여기서 기다리고 있겠네.");
            if(@pos == -1) goto COS004;
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그렇다면 좋겠지만 찬찬은 너무 멀리 있어서 말이지...");
            end;    
        }
        else end;
    }
    if($dosirak == 1)
    {
        set @pos, list2(@list_2, 0, 0, "오, 기다리고 있었네, 요리는 만들어 왔는가?", "예, 만들어 왔습니다.", "아니오, 아직..");
        if(@pos == 0) end;
        else if(@list_2 == 1)
        {
COS006:
            set @pos, dialog(0, 0, 1, "그런가! 수고했네. 그럼 어떤 요리를 가져왔는지 꺼내 보게나.");
            if(@pos == 0) end;
COS007:
            set @pos, input2(@food_name$, 1, 15, "요리 이름인 뭔가?", "제가 만든 요리는", "입니다");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS006;

            if(@food_name$ != "")
            {
                setarray @food$[0], "바나나다발", "영양반찬", "영양음료", "영양통구이", "초보도시락", "기분전환반찬", "기분전환음료", "기분전환통구이", "활력충전통구이", "활력충전반찬", "활력충전음료", "궁극의도시락", "생장촉진통구이", "생장촉진반찬", "생장촉진음료" , "전설의도시락", "생명보존통구이", "생명보존반찬", "생명보존음료", "환상의도시락", "1차도시락", "2차도시락" , "3차도시락", "4차도시락", "비장의도시락";
                setarray @item$[0], " ", " ", " ", " ", "북천황변신시약", " ", " ", " ", " ", " ", " ", "양첨목봉", "청의태자변신시약" , " ", " ", "흑형도", "토깽이변신시약", " ", " ", "천풍선", "암흑왕변신시약", "진명삼지창", "칠교칠선", "금마반지", "이가닌자의검"; 
                setarray @value[0], 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 1, 1, 1, 1, 2, 1;
                setarray @money[0], 500, 5000, 5000, 0, 0, 0, 30000, 30000, 50000, 0, 50000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;
                for(set @loop, 0; @loop < 25; set @loop, @loop+1)
                {
                    if(@food_name$ == @food$[@loop])
                    {
                        if(countitem(name2itemid(@food_name$)) > 0)
                        {
                            delitem @sd, name2itemid(@food_name$), 1;
                            message @sd, 3, @food_name$ + " 주었습니다";
                            if(@item$[@loop] != " ")
                            {
                                if(name2itemid(@item$[@loop]) > 0)
                                {
                                    additem @sd, name2itemid(@item$[@loop]), @value[@loop];
                                }
                            }
                            if(@money[@loop] > 0)
                            {
                                set_money @sd, get_money(@sd)+@money[@loop];
                                updatestatus @sd, 2;
                            }
                            set @pos, dialog(0, 0, 1, "정말 고맙네! 수고했네. 앞으로도 계속 부탁하네.");
                            goto success;
                            end;
                        }
                    }
                }
                set @pos, dialog(0, 0, 0, @food_name$ + "!! 그거 좋지... 하지만 자네는 그 요리를 가지고 있지 않은데? 어디 놔 두고 온 것은 아닌가? 쯧쯧...");
                goto success;
                end;
            }
        }
        else if(@list_2 == 2)
        {
            set @pos, dialog(0, 0, 0, "흠.. 곧 환갑잔치가 시작하니 조금만 서둘러 주게.");
            end;
        }
        else end;
    }
    end;
success:
    deletearray @food$[0], getarraysize(@food$);
    deletearray @item$[0], getarraysize(@item$);
    deletearray @value[0], getarraysize(@value);
    deletearray @money[0], getarraysize(@money);
}