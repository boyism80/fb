{
    set @sd, mysd();
    set @id, myid();

    if($myungsukboo != 2)
    {
        set @pos, dialog(0, 0, 1, "안녕하십니까? 저는 마타라고 합니다.");
        end;
    }
    else
    {
COS001:
        set @pos, dialog(0, 0, 1, "음... 부적이 더 필요하다고요? 알겠습니다. 마침 그 부적이 조금 남아 있으니 드리지요.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "그나저나 참 친절한 분이시군요. 생판 모르는 남을 위해 그 머나먼 일본과 이 중국을 왕복하다니. 탄복했습니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set $myungsukboo_check, 1;
        additem @sd, name2itemid("귀신퇴치부적"), 5;
        addlegend @sd, 29, 7, 0, 1, "귀신퇴치부적을 더 받다.";
    }
}