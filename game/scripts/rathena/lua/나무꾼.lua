{
    set @sd, mysd();
    set @id, myid();

    if(get_level(@sd) < 30)
    {
        set @pos, dialog(0, 0, 1, "자네는 아직 나무를 하기엔 많이 부족해 보이는군.");
        end;
    }

    if($tree_man == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "나무를 하러 오셨소? 이 근처는 나무가 많지는 않지만 좋은 나무가 많아서 나무 하기에 제격이지.");
        if(@pos == 0) end;

        set @pos, dialog(0, 1, 1, "하지만 나무꾼치고는 옷차람이 특이하시구려. 어라, 게다가 도끼도 들고 오지 않은 것 같구려. 잠이 덜 깨셨소? 허허...");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set @pos, list2(@list, 0, 0, "괜찮다면 내 도끼를 주겠소이다. 나는 마침 나무를 다 하고 쉬는 중이라, 필요가 없다오. 집에 도끼는 잔뜩 있고 말이오.", "예, 빌려주세요.", "고맙지만 사양할께요.");
        if(@pos == 1 && @list == 1)
        {
            set $tree_man, 1;
            additem @sd, name2itemid("쇠도끼"), 1;
            addlegend @sd, 28, 7, 0, 5, "쇠도끼로 나무를 하자.";
            set @pos, dialog(0, 0, 1, "그럼 여기 도끼가 있으니 잘 쓰시오. 그럼 난 낮잠이나 자야겠구려. 나무 열심히 하시오...");
            end;
        }
    }
    if($tree_man == 1)
    {
        set @pos, list2(@list, 1, 0, "이미 한 번 쇠도끼를 빌려가지 않았소? 이제 별로 남은 수량이 없어서.. 다시 빌리려면 1만전을 내셔야 하오.", "다시 빌려주세요.", "필요없어요.");
        if(@pos == 1 && @list == 1)
        {
            if(get_money(@sd) < 10000)
            {
                set @pos, dialog(0, 0, 0, "돈이 부족한 것은 아니오?");
                end;
            }
            else
            {
                set_money @sd, get_money(@sd)-10000;
                updatestatus @sd, 2;

                additem @sd, name2itemid("쇠도끼"), 1;

                set @pos, dialog(0, 0, 1, "자, 여기있소. 또 잃어버리지 않게 조심하시오.");
                end;
            }
        }
    }
    else
    {
        set @pos, dialog(0, 0, 0, "zzZ...zzZ...");
        end;
    }
}