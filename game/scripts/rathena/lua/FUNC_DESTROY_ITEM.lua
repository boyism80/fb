{
    set @sd, mysd();
    set @id, myid();

    set @pos, dialog(0, 0, 1, "아이템을 파괴하기 위해선 파괴할 아이템을 지금 가지고 계셔야 하며, 금전 1000전이 소비됩니다.");
    if(@pos == 0) end;

    input1 @txt$, "어떤 아이템을 파괴하시렵니까?";
    if(countitem(name2itemid(@txt$)) > 0)
    {
        if(get_money(@sd) < 1000) { set @pos, dialog(0, 0, 0, "금전이 부족합니다."); end; }
        else
        {
            delitem @sd, name2itemid(@txt$), 1;
            set_money @sd, get_money(@sd)-1000;
            updatestatus @sd, 2;
            set @pos, dialog(0, 0, 1, "파괴 되었습니다.");
            end;
        }
    }
    else { set @pos, dialog(0, 0, 1, "아이템이 존재하지 않습니다."); end; }
    return;
}