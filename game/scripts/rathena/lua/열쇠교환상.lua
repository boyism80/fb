{
    set @sd, mysd();
    set @id, myid();
COS001:
    set @pos, list2(@list, 0, 0, "아함~ 졸려.... 무슨일로 왔는가? 난 바쁜사람이라네...", "보물열쇠를 가지고 왔어요...", "하하.. 그냥 인사차...");
    if(@pos == 0) end;
    else if(@list == 1)
    {
        set @pos, dialog(0, 0, 1, "오호 그런가? 보물열쇠를 가지고 오면 내가 바꿔주긴 하는데...");
        if(@pos == 0) end;

        set @pos, list2(@key, 0, 0, "그래... 무슨 열쇠를 가지고 왔나?", "보물열쇠1 을 가지고 왔습니다.", "보물열쇠2 를 가지고 왔습니다.", "보물열쇠3 을 가지고 왔습니다.", "보물열쇠4 를 가지고 왔습니다.");
        if(@pos == 0) end;
        else if(@key == 1) { set @key$, "보물열쇠1"; set @item$, "현철장갑"; set @clothes$, "장갑"; set @msg1$, "을"; set @msg2$, "을"; set @msg3$, "이"; }
        else if(@key == 2) { set @key$, "보물열쇠2"; set @item$, "속삭임팔찌"; set @clothes$, "팔찌"; set @msg1$, "를"; set @msg2$, "를"; set @msg3$, "가"; }
        else if(@key == 3) { set @key$, "보물열쇠3"; set @item$, "진마반지"; set @clothes$, "반지"; set @msg1$, "을"; set @msg2$, "를"; set @msg3$, "이"; }
        else if(@key == 4) { set @key$, "보물열쇠4"; set @item$, "선류"; set @clothes$, "선류"; set @msg1$, "를"; set @msg2$, "를"; set @msg3$, "가"; }
        else end;

        if(countitem(name2itemid(@key$)) == 0)
        {
            set @pos, dialog(0, 0, 0, @key$ + @msg3$ + " 없지 않은가?!");
            end;
        }
        else
        {
            delitem @sd, name2itemid(@key$), 1;
            message @sd, 3, @key$ + " 주었습니다";
            additem @sd, name2itemid(@item$), 1;
            set @pos, dialog(0, 0, 1, "오호 " + @key$ + " " + @msg1$ + " 정말로 가지고 왔구먼, 내 좋은 " + @clothes$ + "" + @msg2$ + " 선물로 줌세나");
            end;
        }
    }
    else if(@list == 2)
    {
        set @pos, dialog(0, 0, 0, "허허.. 싱겁기는....");
        end;
    }
    else end;
}