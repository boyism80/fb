{
    set @sd, mysd();
    set @id, myid();
    set @pos, dialog(0, 0, 1, "전 서버에 방송을 하시려구요?");
    if(@pos == 0) end;

    if(countitem(name2itemid("방송쿠폰")) == 0)
    {
        set @pos, dialog(0, 0, 0, "방송쿠폰을 가지고 계셔야 방송을 하실 수 있어요.");
        end;
    }

    set @pos, input2(@broadcast$, 0, 35, "방송하실 말을 적어주세요.", "방송할 말은", "입니다");
    if(@pos == 0) end;

    delitem @sd, name2itemid("방송쿠폰"), 1;
    message @sd, 3, "방송쿠폰 주었습니다";
    broadcast 5, "###[" + get_name(@sd) + "]>>" + @broadcast$;
}