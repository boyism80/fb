{
    set @sd, mysd();
    image 1, 19, 0;
    if($nakrang5 == 1)
    {
        if($count_squirrel >= 5 && $count_rabbit >= 5)
        {            
            set @pos, dialog(0, 0, 1, "잘 하셨습니다!! 상으로\\n초심자의머리띠를 드릴게요!!");
            if(@pos == 0) end;

            delitem @sd, name2itemid("낙랑의두루마리5"), 1;
            message @sd, 3, "낙랑의두루마리5 주었습니다";
            additem @sd, name2itemid("낙랑의두루마리6"), 1;
            additem @sd, name2itemid("초심자의머리띠"), 1;
            if(get_level(@sd) < 5)
            {
                set @pos, dialog(0, 0, 1, "<보상>\\n'초심자의귀마개'\\n'낙랑의두루마리6'을 얻다!!!\\n경험치 200 상승!!");
                if(@pos == 0) end;

                giveexp @sd, 200;
            }
            else { set @pos, dialog(0, 0, 1, "<보상>\\n'초심자의귀마개'\\n'낙랑의두루마리6'을 얻다!!!"); if(@pos == 0) end; }

            set $nakrang5, 0; 
            set $count_squirrel, 0;
            set $count_rabbit, 0;
        }
        else
        {
COS001:
            set @pos, dialog(0, 0, 1, "사냥한 토끼 : "+ $count_rabbit +" / 5\\n사냥한 다람쥐 : " + $count_squirrel + " / 5");
            if(@pos == 0) end;
COS002:
            set @pos, dialog(0, 1, 1, "몬스터 앞으로 가서\\n스페이스바를 연타하거나,\\n마우스로 몬스터를 클릭하면\\n공격할 수 있습니다.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS001;

            set @pos, dialog(0, 1, 1, "귀엽지만.. \\n눈물을 머금고\\n조금만 더 사냥해보세요..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
        }        
    }
    else
    {
COS003:
        set @pos, dialog(1, 0, 1, "<임무>\\n 토끼와 다람쥐 사냥\\n\\n<내용>\\n 이제, 사냥터에 왔으니 사냥을 해 봐야지요!!\\n주변에, '토끼'와 '다람쥐'가 보이실 것입니다.\\n일단 몬스터 앞에 서서 스페이스바를\\n연타하시면 마구 공격합니다. '토끼' 5마리와\\n'다람쥐' 5마리씩 사냥해보세요~\\n\\n<보상>\\n 초심자의귀마개, 경험치 200 (레벨5미만)");
        if(@pos == 0) end;
COS004:
        set @pos, dialog(1, 1, 1, "<힌트>\\n 마우스로 몬스터를 마구 클릭해도 공격합니다.\\n떨어진 아이템은 마우스로 클릭하거나, 아이템\\n위에 서서 ,키를 누르시면 주울 수 있습니다 ");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;

        set @pos, dialog(0, 1, 1, "팁\\n도토리나 토끼고기는\\n푸줏간에 파실 수 있습니다.\\n직업을 가질 때, 도토리가\\n10개가 필요하니 10개는\\n남겨두세요..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS004;

        set $nakrang5, 1;
        set $count_squirrel, 0;
        set $count_rabbit, 0;
    }
}