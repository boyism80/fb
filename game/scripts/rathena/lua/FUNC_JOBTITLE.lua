{
    set @sd, mysd();
    set @id, myid();
COS00000:
    set @pos, dialog(0, 0, 1, "네 정성이 갸륵하니... 그래,\\n무슨 칭호를 받고 싶으냐?");
    if(@pos == 0) { end; }
    set @pos, input2(@title$, 1, 20, "", "받고싶은 칭호는", "입니다");
    if(@pos == 0) { end; }
    else if(@pos == -1) { goto COS00000; }
    set @pos, list2(@ret, 0, 0, "그 칭호로 바꾸려면 금전 5000전을\\n바쳐야 하느니라. 네 소원을 이루겠\\n느냐?", "예", "아니오");
    if(@pos == 0) end;
    else if(@ret == 1)
    {
        if(get_money(@sd) < 5000)
        {
            set @pos, dialog(0, 0, 1, "예끼 이놈! 감히 돈도 없으면서 날 찾아와?");
            end;
        }
        set_money @sd, get_money(@sd)-5000;
        updatestatus @sd, 2;
        set_title @sd, @title$;
COS00001:
        set @pos, dialog(0, 0, 1, "그럼, 너의 소원을 들어주겠노\\n라");
        if(@pos == 0) { end; }
        set @pos, dialog(0, 1, 1, "칭호를 받았으니, 자신감을 가\\n지고 다니게나...");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto COS00001; }
    }
}