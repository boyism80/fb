{
    set @sd, mysd();
    set @id, myid();

    if(last_say(@sd, "검정깃발") && get_mapname(@sd) == "대방성대장간")
    {
        image 0, 487, 0;

        if(get_money(@sd) < 5000)
        {
            set @pos, dialog(0, 0, 0, "금전이 " + (5000-get_money(@sd)) + "전 부족합니다.");
        }
        else
        {
            set_money @sd, get_money(@sd)-5000;
            updatestatus @sd, 2;

            additem @sd, name2itemid("검정깃발"), 1;

            set @pos, dialog(0, 0, 1, "자, 받게나 여기있네.");
        }
    }
    else if(last_say(@sd, "참가"))
    {
        if(get_x(@id) >= 19 && get_x(@id) <= 35 && get_y(@id) >= 25 && get_y(@id) <= 40)
        {
            if(get_mapname(@sd) == "현무성남쪽")
            {
                callfunc "KONGSUNG", "현무", #turtle_castle$, 1, 3;
            }
            else if(get_mapname(@sd) == "주작성남쪽")
            {
                callfunc "KONGSUNG", "주작", #bird_castle$, 1, 31;
            }
            else if(get_mapname(@sd) == "청룡성남쪽")
            {
                callfunc "KONGSUNG", "청룡", #dragon_castle$, 1, 2;
            }
            else if(get_mapname(@sd) == "백호성남쪽")
            {
                callfunc "KONGSUNG", "백호", #tiger_castle$, 1, 11;
            }
        }
    }
    else if(last_say(@sd, "/레전드") && get_gmlevel(@sd) == 2)
    {
        message @sd, 0, "주의사항 : 레전드 테스트시 레전드 번호는 50000번으로 합니다.";
        set @pos, input1(@a, "모양");
        set @pos, input1(@b, "글씨색");
        addlegend @sd, 50000, @a, 0, @b, "좆같은테스트";
        message @sd, 0, "모양 : " + @a + ", 글씨색 : " + @b;
    }
    else if(last_say(@sd, "/버그해결"))
    {
        callfunc "BUGCLEAR";
    }
    else if(last_say(@sd, "/몹지우기1") && get_gmlevel(@sd) == 2)
    {
        set @target_id, obj_getfront();
        if(@target_id > 0 && istype(@target_id) == 2)
        {
            deletemob @target_id, mapid();
        }
    }
    else if(last_say(@sd, "ㅇㄴㅁ멀티닷!굞멀튀"))
    {
        set_level @sd, 5;
        set_job @sd, rand(1, 4);
        addspell name2spellid("비영사천문");
    }
    else if(last_say(@sd, "/?"))
    {
        message @sd, 3, "/버그해결";
    }
    return;
}