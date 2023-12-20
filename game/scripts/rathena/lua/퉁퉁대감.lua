{
    set @sd, mysd();
    set @id, myid();

    if($toongtoong == 0)
    {
        set @pos, list2(@list, 0, 0, "대륙 사람이로군. 듣기에 대륙에는 '호박'이라는 예쁜 게 있다더군. 그거 열 개만 구해다 줄 수 있을까?", "네, 구해드리지요.", "아니오, 바빠서..");
        if(@pos == 0) end;
        else if(@list != 1) end;

        set $toongtoong, 1;
        set @pos, dialog(0, 0, 1, "내 좋은걸로 보답할테니 어서 다녀오게. 잊지 말게. 호박 열 개 일세.");    
        end;
    }
    if($toongtoong == 1)
    {
        if(countitem(name2itemid("호박")) < 10)
        {
            set @pos, dialog(0, 0, 0, "아직 호박 열 개를 구하지 못했군.");
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "잘 가져왔군. 이리주게.");
            if(@pos == 0) end;

            delitem @sd, name2itemid("호박"), 10;
            message @sd, 3, "호박 주었습니다";
            additem @sd, name2itemid("선장의일기2"), 1;
            addlegend @sd, 32, 7, 0, 1, "진백랑의 부탁을 들어주자. [3/6]";
            set $toongtoong, 2;
            set @pos, dialog(0, 0, 1, "음, 예쁘긴 예쁘군. 약속대로 좋은걸 주지.");
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "호박.. 참 예쁘군..");
        end;
    }
}