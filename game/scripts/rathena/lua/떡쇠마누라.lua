{
    set @sd, mysd();
    set @id, myid();
    set @pos, dialog(0, 0, 1, "아이구 더워. 너무 더워서 뭔 일을 할 수가 없네...");
    if(@pos == 0) end;

    set @pos, list2(@list, 0, 0, "과일화채와 웨딩드레스1,2,3 중의 하나를 주실려우?", "예, 웨딩드레스1을 수선해주세요.", "예, 웨딩드레스2를 수선해주세요.", "예, 웨딩드레스3을 수선해주세요.", "아니오. 다음에 다시 오겠습니다.");
    if(@pos == 0) end;
    else if(@list == 1) { set @item$, "웨딩드레스1"; set @add$, "웨딩드레스5"; goto make; }
    else if(@list == 2) { set @item$, "웨딩드레스2"; set @add$, "웨딩드레스6"; goto make; }
    else if(@list == 3) { set @item$, "웨딩드레스3"; set @add$, "웨딩드레스7"; goto make; }
    else end;
    end;
make:
    if(countitem(name2itemid("과일화채")) == 0 || countitem(name2itemid(@item$)) == 0)
    {
        set @pos, dialog(0, 0, 0, "아이고, 더운데 재료도 없고 신경질나게 하지마!");
        end;
    }
    else
    {
        delitem @sd, name2itemid("과일화채"), 1;
        delitem @sd, name2itemid(@item$), 1;
        message @sd, 3, "과일화채 주었습니다";
        message @sd, 3, @item$ + " 주었습니다";
        additem @sd, name2itemid(@add$), 1;
        set @pos, dialog(0, 0, 1, "여기 수선이 끝났으니 가지고 가시우... 잘 입으시구랴...");
        end;
    }
}