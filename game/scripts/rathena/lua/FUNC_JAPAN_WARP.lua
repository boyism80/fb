{
    set @sd, mysd();
    set @id, myid();
    image 0, 0, 0;
    if(get_level(@sd) >= 99)
    {
        set @pos, list2(@ret, 0, 0, "비밀통로로 가겠는가?", "아니오, 그냥 다음방으로 가겠습니다.", "네, 비밀통로로 보내주십시오.");
        if(@pos == 0) { end; }
        if(@ret == 1)
        {
            warp @sd, 2178, 4, 3;
        }
        else if(@ret == 2)
        {
            set @pos, list2(@ret, 0, 0, "어디로 가겠는가?", "백향의방으로 가겠습니다.", "문려의방으로 가겠습니다.", "닌자의방으로 가겠습니다.", "선녀의방으로 가겠습니다.",    "아니오, 잠시후에...");
            if(@pos == 0) { end; }
            if(@ret == 1)
            {
                if(get_money(@sd) < 100000) { set @pos, dialog(0, 0, 0, 100000-get_money(@sd) + "원이 부족합니다."); end; }
                set_money @sd, get_money(@sd)-100000;
                updatestatus @sd, 2;
                warp @sd, 2122, 5, 2;
            }
            else if(@ret == 2)
            {
                if(get_money(@sd) < 80000) { set @pos, dialog(0, 0, 0, 80000-get_money(@sd) + "원이 부족합니다."); end; }
                set_money @sd, get_money(@sd)-80000;
                updatestatus @sd, 2;
                warp @sd, 2198, 14, 38;
            }
            else if(@ret == 3)
            {
                if(get_money(@sd) < 60000) { set @pos, dialog(0, 0, 0, 60000-get_money(@sd) + "원이 부족합니다."); end; }
                set_money @sd, get_money(@sd)-60000;
                updatestatus @sd, 2;
                warp @sd, 2138, 3, 2;
            }
            else if(@ret == 4)
            {
                if(get_money(@sd) < 40000) { set @pos, dialog(0, 0, 0, 40000-get_money(@sd) + "원이 부족합니다."); end; }
                set_money @sd, get_money(@sd)-40000;
                updatestatus @sd, 2;
                warp @sd, 2147, 13, 2;
            }
        }
    }
    else
    {
        warp @sd, 2178, 4, 3;
    }
    return;
}