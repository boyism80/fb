{
    image 0, 220, 0;
    set @sd, mysd();
    set @id, myid();
    set @name$, #clanname$;
    
    set @err, set_clan(@sd, @name$, get_name(@sd));
    if(@err == 0)
    {
        set @pos, dialog(0, 0, 0, "정상적으로 가입되었습니다.");
        end;
    }
    else
    {
        set @pos, dialog(0, 0, 0, "오류로 인해 가입되지 않았습니다..");
        end;
    }
    end;
}