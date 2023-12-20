{
    set @sd, mysd();
    set @id, myid();
    set @mapid, get_mapid(@sd);
    set @mapname$, get_mapname(@sd);
    if(@mapname$ == "소호의방" || @mapname$ == "전욱의방" || @mapname$ == "제곡의방" || @mapname$ == "요제의방" || @mapname$ == "순제의방" || @mapname$ == "반고의방" || @mapname$ == "용왕용마의침소" || @mapname$ == "용왕용천의침소" || @mapname$ == "용왕용랑의침소" || @mapname$ == "용왕용겸의침소" || @mapid >= 2200 && @mapid <= 2230 || @mapid >= 2240 && @mapid <= 2243)
    {
        set_mp @sd, get_mp(@sd)+30;
        spellcast @sd, name2spellid("귀환");
        message @sd, 0, "[알림] " + @mapname$ + "에서 로그아웃 하실경우 자동귀환 됩니다."; 
    }
    else if(@mapname$ == "청룡성남쪽" || @mapname$ == "청룡성입구" || @mapname$ == "청룡의성" || @mapname$ == "청룡의심장")
    {
        warp @sd, name2mapid("청룡성남쪽"), rand(24, 34), rand(36, 40);
        message @sd, 0, "[알림] " + @mapname$ + "에서 로그아웃 하실경우 청룡성남쪽으로 귀환됩니다.";
    }
    else if(@mapname$ == "주작성남쪽" || @mapname$ == "주작성입구" || @mapname$ == "주작의성" || @mapname$ == "주작의심장")
    {
        warp @sd, name2mapid("주작성남쪽"), rand(24, 34), rand(36, 40);
        message @sd, 0, "[알림] " + @mapname$ + "에서 로그아웃 하실경우 주작성남쪽으로 귀환됩니다.";
    }
    else if(@mapname$ == "현무성남쪽" || @mapname$ == "현무성입구" || @mapname$ == "현무의성" || @mapname$ == "현무의심장")
    {
        warp @sd, name2mapid("현무성남쪽"), rand(24, 34), rand(36, 40);
        message @sd, 0, "[알림] " + @mapname$ + "에서 로그아웃 하실경우 현무성남쪽으로 귀환됩니다.";
    }
    else if(@mapname$ == "백호성남쪽" || @mapname$ == "백호성입구" || @mapname$ == "백호의성" || @mapname$ == "백호의심장")
    {
        warp @sd, name2mapid("백호성남쪽"), rand(24, 34), rand(36, 40);
        message @sd, 0, "[알림] " + @mapname$ + "에서 로그아웃 하실경우 백호성남쪽으로 귀환됩니다.";
    }
    else
    {
        set @nt, callfunc("Server_Nowtime");
        if($logout_time+172800 < @nt)
        {
            if(get_level(@sd) >= 5)
            {
                set_mp @sd, get_mp(@sd)+30;
                spellcast @sd, name2spellid("귀환");
            }
        }
    }
    return;
}