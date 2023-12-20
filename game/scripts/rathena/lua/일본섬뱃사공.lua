{
    set @sd, mysd();
    set @id, myid();

    if(gettime(1, 0) >= 40 && gettime(1, 0) <= 59)
    {
        set @pos, dialog(0, 0, 1, "고구려입니다. 안녕히 가십시오.");
        if(@pos == 0) end;

        warp @sd, name2mapid("고구려-일본선착장"), 54, 14;
        end;
    }
    else if(gettime(1, 0) >= 0 && gettime(1, 0) <= 19)
    {
        set @pos, dialog(0, 0, 1, "지금 항해중입니다. 곧 도착할 예정이오니 잠시만 더 기다려 주시기 바랍니다.");
        end;
    }
    else if(gettime(1, 0) >= 20 && gettime(1, 0) <= 40)
    {
        set @pos, dialog(0, 0, 1, "일본입니다. 안녕히 가십시오.");
        if(@pos == 0) end;

        warp @sd, name2mapid("동쪽해안가"), 1, 44;
        end;
    }
    else
    {
        set @pos, dialog(0, 0, 1, "지금 항해중입니다. 곧 도착할 예정이오니 잠시만 더 기다려 주시기 바랍니다.");
        end;
    }
}