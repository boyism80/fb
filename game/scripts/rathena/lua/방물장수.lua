{
    set @sd, mysd();
    set @id, myid();

    if($anfqud == 0)
    {
        if(countitem(name2itemid("물병보관증")) == 0)
        {
            set @pos, dialog(0, 0, 1, "물병이 필요하신가요? 물병의 가격은 15만전입니다.");
            if(@pos == 0) end;
            set @list, list1("물병을 구입하시겠습니까?", "예", "아니오");
            if(@list == 1)
            {
                set @pos, dialog(0, 0, 1, "물병을 빚는데는 시간이 걸립니다. 여기 보관증을 드릴테니 며칠후 오세요.");
                additem @sd, name2itemid("물병보관증"), 1;
                end;
            }
            else end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "물병을 찾으러 오셨군요.");
            if(@pos == 0) end;

            if(get_money(@sd) < 150000)
            {
                set @pos, dialog(0, 0, 0, "15만전을 구해오세요.");
                end;
            }
            else
            {
                set @pos, dialog(0, 0, 1, "여기있습니다.");
                set $anfqud, 1;
                set_money @sd, get_money(@sd)-150000;
                updatestatus @sd, 2;
                delitem @sd, name2itemid("물병보관증"), 1;
                additem @sd, name2itemid("물병"), 1;
            }
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "이미 물병을 한번 받으셨군요.");
        end;
    }
}