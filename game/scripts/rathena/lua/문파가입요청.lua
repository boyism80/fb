{
    image 0, 220, 0;
    set @sd, mysd();
    set @id, myid();
    set @name$, #clanname$;
    
    set @join, list1(@name$ + "문파에서 문파가입 요청이 들어왔습니다. 가입하시겠습니까? ", "네, 가입하겠습니다.", "아니오, 안할래요.");
    if(@join == 1)
    {
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
    }
    end;
}