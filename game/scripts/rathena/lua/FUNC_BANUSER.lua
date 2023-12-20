{
    set @sd, mysd();
    set @id, myid();

    set @pos, input1(@user$, "어느 유저를 제재하시겠습니까?");
    if(@pos == 0) { end; }

    set @target_sd, name2sd(@user$);
    if(@target_sd > 0)
    {
        set @pos, input1(@ban, "몇초간 제재하시겠습니까?");
        if(@pos == 0) { end; }
        else if(@ban <= 0) { set @pos, dialog(0,0,0, "제재는 1초이상만 가능합니다."); end; }

        set @ban_list, list1("정말 " + @user$ + "님을 " + @ban + "초간 제재하시겠습니까? 제재는 해제가 불가능합니다.", "예", "아뇨");
        if(@ban_list == 1)
        {
            set @nt, callfunc("Server_Nowtime");
            set #ban_timer, @nt + @ban;
            message @sd, 0, @user$ + "님을 " + @ban + "초간 제재합니다.";
            runscript @target_sd, "킥밴";
            end;
        }
        else { end; }
    }
    else
    {
        set @pos, dialog(0,0,0, @user$ + "님은 바람의나라에 없습니다.");
        end;
    }
    return;
}