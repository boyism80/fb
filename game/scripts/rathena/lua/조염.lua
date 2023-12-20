{
    set @sd, mysd();
    set @id, myid();

    if($bong_book == 0)
    {
        set @pos, list2(@list, 0, 0, "내가 책 한권 드릴테니 한번 읽어나 보시게나.", "예, 주세요.", "아니오, 책이라면 질색이라서..");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $boong_book, 1;
            additem @sd, name2itemid("봉래산전설"), 1;
            set @pos, dialog(0, 0, 1, "여기있네. 그럼 재밌게 읽게나.");
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그러면 쓰나.. 책을 읽어야 마음에 양식이 쌓이는 것이네.");
            end;
        }
        else end;
    }
    else
    {
        if(countitem(name2itemid("봉래산전설")) == 0)
        {
            set @pos, dialog(0, 0, 1, "뭐야? 책을 잃어버려? 이런 고약한.. 내 저번엔 다시 줬지만, 이번엔 그냥은 못주겠네.");
            if(@pos == 0) end;

            set @pos, list2(@book, 0, 0, "1000전을 내면 다시 한권 주지.", "봉래산전설을 다시 읽고싶어요. 1000전에 주세요.", "돈이 없어서..");
            if(@pos == 0) end;
            else if(@book != 1) end;

            if(get_money(@sd) < 1000)
            {
                set @pos, dialog(0, 0, 0, "돈이 부족하지 않나?");
                end;
            }
            else
            {
                additem @sd, name2itemid("봉래산전설"), 1;
                set_money @sd, get_money(@sd)-1000;
                updatestatus @sd, 2;
                set @pos, dialog(0, 0, 1, "다시 잘 읽어봐!");
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 1, "어때? 봉래산전설은 재미있나?");
            end;
        }
    }
}