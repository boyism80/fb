{
    set @sd, mysd();
    image 1, 19, 0;
    
    if($nakrang7 == 1)
    {
        if(get_level(@sd) >= 5)
        {
            set @pos, dialog(0, 0, 1, "잘 하셨습니다!! 상으로\\n초심자의방패를 드릴게요!!");
            if(@pos == 0) end;

            delitem @sd, name2itemid("낙랑의두루마리7"), 1;
            message @sd, 3, "낙랑의두루마리7 주었습니다";
            additem @sd, name2itemid("낙랑의두루마리8"), 1;
            additem @sd, name2itemid("초심자의방패"), 1;
            set $nakrang7, 0;
            set @pos, dialog(0, 0, 1, "<보상>\\n'초심자의방패',\\n'낙랑의두루마리8'을 얻다!!!");
        }
        else { set @pos, dialog(0, 0, 1, "레벨 5를 만드는 것은 그다지\\n힘들지 않습니다. 왕초보\\n사냥터에서만 사냥해도 금방\\n만드실 수 있을 꺼에요."); }
    }
    else
    {
COS001:
        set @pos, dialog(1, 0, 1, "<임무>\\n 레벨 5 만들기\\n\\n<내용>\\n 잠시 퀴즈로 머리를 식혔으니, 다시 레벨업의\\n세계로!! 레벨 5를 만들어보세요!!\\n\\n<보상>\\n초심자의방패");
        if(@pos == 0) end;

        set @pos, dialog(1, 1, 1, "<힌트>\\n 레벨 5를 만들면 직업을 선택하실 수 있습니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set $nakrang7, 1;
    }
    
}