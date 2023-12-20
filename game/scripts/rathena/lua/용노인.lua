{
    set @sd, mysd();
    set @id, myid();

POS000:
    set @pos, dialog(0, 0, 1, "용왕의 방으로 이동하기 위해서는, 어금니 4개가 필요하네.");
    if(@pos == 0) end;
POS001:
    set @pos, dialog(0, 1, 1, "어금니의 이름들은 각각 알고 왔겠지?");
    if(@pos == -1) goto POS000;
    else if(@pos == 0) end;
POS002:
    set @pos, list2(@list, 0, 0, "지금 준비되어 있나?\\n참고로 그룹원이 있으면 같이 이동된다네.", "네", "아니요");
    if(@pos == 0) end;
    else if(@list == 1)
    {
        set @pos, list2(@list, 0, 0, "어디로 이동하겠는가?", "용왕용마의 방", "용왕용천의 방", "용왕용랑의 방", "용왕용겸의 방");
        if(@pos == 0) { end; }
        else if(@list == 1) { set @warp_id, 666; goto MAP; }
        else if(@list == 2) { set @warp_id, 667; goto MAP; }
        else if(@list == 3) { set @warp_id, 668; goto MAP; }
        else if(@list == 4) { set @warp_id, 669; goto MAP; }
    }
    else end;
    end;
MAP:
    if(countitem(name2itemid("묵룡의어금니")) < 1 || countitem(name2itemid("감룡의어금니")) < 1 || countitem(name2itemid("진룡의어금니")) < 1 ||  countitem(name2itemid("흑룡의어금니")) < 1)
    {
        set @pos, dialog(0, 0, 1, "아직 어금니가 준비되지 않았는데?");
        end;
    }
    else
    {
        delitem @sd, name2itemid("묵룡의어금니"), 1;
        delitem @sd, name2itemid("감룡의어금니"), 1;
        delitem @sd, name2itemid("흑룡의어금니"), 1;
        delitem @sd, name2itemid("진룡의어금니"), 1;
        message @sd, 3, "묵룡의어금니 주었습니다";
        message @sd, 3, "감룡의어금니 주었습니다";
        message @sd, 3, "흑룡의어금니 주었습니다";
        message @sd, 3, "진룡의어금니 주었습니다";

        if(@warp_id == 666) { mobspawn @warp_id, rand(5, 19), rand(5, 19), rand(0, 3), 1115; }
        else if(@warp_id == 667) { mobspawn @warp_id, rand(5, 19), rand(5, 19), rand(0, 3), 1116; }
        else if(@warp_id == 668) { mobspawn @warp_id, rand(5, 19), rand(5, 19), rand(0, 3), 1114; }
        else if(@warp_id == 669) { mobspawn @warp_id, rand(5, 19), rand(5, 19), rand(0, 3), 1113; }

        if(party(@sd) == 0)
        {
            warp @sd, @warp_id, rand(11, 14), rand(22, 24);
            message @sd, 0, "용왕의방으로 이동하였습니다.";
            message @sd, 0, "해당 맵에서 로그아웃시 자동귀환 됩니다.";
            end;
        }
        else
        {
            for(set @i, 0; @i < partymax(); set @i, @i+1)
            {
                set @target_sd, party_member(@sd, @i);
                if(@target_sd > 0)
                {
                    if(get_mapid(@sd) == get_mapid(@target_sd))
                    {
                        if(@target_sd != @sd)
                        {
                            warp @target_sd, @warp_id, rand(11, 14), rand(22, 24);
                            message @target_sd, 8, get_name(@sd) + "님에 의해 용왕의 방으로 이동하였습니다.";
                            message @target_sd, 0, "해당 맵에서 로그아웃시 자동귀환 됩니다.";
                        }
                    }
                }
            }
            set @pos, dialog(0, 0, 0, "용왕의방으로 그룹원과 함께 이동하였습니다.");
            warp @sd, @warp_id, rand(11, 14), rand(22, 24);
            message @sd, 0, "해당 맵에서 로그아웃시 자동귀환 됩니다.";
            end;
        }
    }
}