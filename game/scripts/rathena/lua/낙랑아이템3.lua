{
    set @sd, mysd();
    image 1, 19, 0;
    if($nakrang3 == 1)
    {    
        if(get_equipitem(@sd, 0) == name2itemid("초심자의목도"))
        {
            if(get_gender(@sd) == 0) { set @pos, dialog(0, 0, 1, "잘 하셨습니다!! 상으로\\n초심자의남자갑주를 드릴게요!!"); if(@pos == 0) end; }
            else { set @pos, dialog(0, 0, 1, "잘 하셨습니다!! 상으로\\n초심자의여자갑주를 드릴게요!!"); if(@pos == 0) end; }

            delitem @sd, name2itemid("낙랑의두루마리3"), 1;
            message @sd, 3, "낙랑의두루마리3 주었습니다";
            additem @sd, name2itemid("낙랑의두루마리4"), 1;
        
            if(get_level(@sd) < 5) { giveexp @sd, 50; }

            if(get_gender(@sd) == 0) 
            {
                if(get_level(@sd) < 5) 
                {
                    set @pos, dialog(0, 0, 1, "<보상>\\n'초심자의남자갑주',\\n'낙랑의두루마리4'을 얻다!!!\\n경험치 50 상승!!");
                }
                else 
                {                    
                    set @pos, dialog(0, 0, 1, "<보상>\\n'초심자의남자갑주',\\n'낙랑의두루마리4'을 얻다!!!");
                }
                additem @sd, name2itemid("초심자의남자갑주"), 1; 
            }
            else
            { 
                if(get_level(@sd) < 5) 
                {                    
                    set @pos, dialog(0, 0, 1, "<보상>\\n'초심자의여자갑주',\\n'낙랑의두루마리4'을 얻다!!!\\n경험치 50 상승!!");
                }
                else
                {
                    set @pos, dialog(0, 0, 1, "<보상>\\n'초심자의여자갑주',\\n'낙랑의두루마리4'을 얻다!!!");
                }
                additem @sd, name2itemid("초심자의여자갑주"), 1; 
                
            }
            set $nakrang3, 0;
        }
        else { set @pos, dialog(1, 0, 1, "실패하셨군요!! 좀 더 연습한 후 다시\\n시도해보세요.."); }
    }
    else
    {
COS001:
        if(get_gender(@sd) == 0) { set @pos, dialog(1, 0, 1, "<임무>\\n 목도착용\\n\\n<내용> \\n도톨을 사용하는 방법은 잘 익히셨지요? 이번엔\\n목도를 착용해 봅시다! 도톨과 같은 방법으로\\n사용하시면 착용이 된답니다!! 자.. 시간은 5초\\n드립니다!!\\n\\n<보상>\\n 초심자의남자갑주, 경험치 50 (레벨5미만)"); if(@pos == 0) end; }
        else { set @pos, dialog(1, 0, 1, "<임무>\\n 목도착용\\n\\n<내용> \\n도톨을 사용하는 방법은 잘 익히셨지요? 이번엔\\n목도를 착용해 봅시다! 도톨과 같은 방법으로\\n사용하시면 착용이 된답니다!! 자.. 시간은 5초\\n드립니다!!\\n\\n<보상>\\n 초심자의여자갑주, 경험치 50 (레벨5미만)"); if(@pos == 0) end; }

        set @pos, dialog(1, 1, 1, "<힌트>\\n 목도를 착용하기 위해서는 오른쪽의\\n'소지품(단축키i)'을 클릭하신 다음 제가 드린\\n목도를 더블클릭하면 됩니다. 단축키 'u(use)'를\\n누른 다음, 목도가 있는 슬롯의 알파벳을\\n누르셔도 됩니다. 'c'슬롯에 있다면, 'u'를\\n누르신 후 'c'를 누르시면 됩니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set $nakrang3, 1;
    }
}