{
    set @sd, mysd();
    set @id, myid();
    set @mapname$, get_mapname(@sd);

    spelleveryuncast @sd;

    if(@mapname$ == "청룡성남쪽" || @mapname$ == "청룡성입구" || @mapname$ == "청룡의성" || @mapname$ == "청룡의심장")
    {
        warp @sd, name2mapid("청룡성남쪽"), rand(24, 34), rand(36, 40);
        announce 5, "[알림] " + get_name(@sd) + "님이 죽어서 탈락하셨습니다.";
        pcdead @sd;
    }
    else if(@mapname$ == "주작성남쪽" || @mapname$ == "주작성입구" || @mapname$ == "주작의성" || @mapname$ == "주작의심장")
    {
        warp @sd, name2mapid("주작성남쪽"), rand(24, 34), rand(36, 40);
        announce 5, "[알림] " + get_name(@sd) + "님이 죽어서 탈락하셨습니다.";
        pcdead @sd;
    }
    else if(@mapname$ == "현무성남쪽" || @mapname$ == "현무성입구" || @mapname$ == "현무의성" || @mapname$ == "현무의심장")
    {
        warp @sd, name2mapid("현무성남쪽"), rand(24, 34), rand(36, 40);
        announce 5, "[알림] " + get_name(@sd) + "님이 죽어서 탈락하셨습니다.";
        pcdead @sd;
    }
    else if(@mapname$ == "백호성남쪽" || @mapname$ == "백호성입구" || @mapname$ == "백호의성" || @mapname$ == "백호의심장")
    {
        warp @sd, name2mapid("백호성남쪽"), rand(24, 34), rand(36, 40);
        announce 5, "[알림] " + get_name(@sd) + "님이 죽어서 탈락하셨습니다.";
        pcdead @sd;
    }
    else
    {
        if(get_job(@sd) >= 1 && get_job(@sd) <= 4)
        {
            if(get_level(@sd) < 99)
            {
                pcdead @sd;
            }
            else
            {
                if(ismapoption(@sd, 5))
                {
                    set @exp, get_exp(@sd);
                    if(@exp < 0) { set @exp1, 2147483647; }
                    else { set @exp1, @exp; }

                    set @exp2, (@exp1/100)*3;
                    set_exp @sd, get_exp(@sd)-@exp2;
                    updatestatus @sd, 0;

                    pcdead @sd;
                }
                else
                {
                    pcdead @sd;
                }
            }
        }
        else
        {
            effect @id, 22;
            set_hp @sd, get_maxhp(@sd);
            updatestatus @sd, 1;

            if(get_nation(@sd) == 1)
            {
                setarray @mapid[0], 29, 30, 31, 8016, 8017, 8018, 8043, 8044, 8045, 8070, 8071, 8072, 8097, 8098, 8099, 8124, 8125, 8126, 8151, 8152, 8153, 8178, 8179, 8180, 8205, 8206, 8207, 8232, 8233, 8234, 8259, 8260, 8261;
                setarray @x[0], 3, 4, 7, 8, 3, 4, 7, 8;
                setarray @y[0], 5, 5, 5, 5, 9, 9, 9, 9;
            }
            else
            {
                setarray @mapid[0], 15, 16, 17, 7716, 7717, 7718, 7743, 7744, 7745, 7770, 7771, 7772, 7797, 7798, 7799, 7824, 7825, 7826, 7851, 7852, 7853, 7878, 7879, 7880, 7905, 7906, 7907, 7932, 7933, 7934, 7959, 7960, 7961;
                setarray @x[0], 3, 4, 7, 8, 3, 4, 7, 8;
                setarray @y[0], 5, 5, 5, 5, 9, 9, 9, 9;
            }

            set @warp_mapid, @mapid[rand(0, getarraysize(@mapid)-1)];
            set @warp_x, @x[rand(0, getarraysize(@x)-1)];
            set @warp_y, @y[rand(0, getarraysize(@y)-1)];

            deletearray @mapid[0], getarraysize(@mapid);
            deletearray @x[0], getarraysize(@x);
            deletearray @y[0], getarraysize(@y);

            warp @sd, @warp_mapid, @warp_x, @warp_y;

            message @sd, 8, "죽으셨군요! 아직은 레벨이 낮으니 주막에서 살려드립니다. 직업을 가지신 후 죽으실 경우 유령이되니 조심하세요!";
        }
    }
    return;
}