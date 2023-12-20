{
    set @sd, mysd();
    set @id, myid();

    if(countitem(name2itemid(getarg(0))) < getarg(1))
    {
        set @pos, dialog(0, 0, 0, "아직 " + getarg(0) + " " + getarg(1) + "개를 모아오지 못하신 것 같군요?");
        end;
    }
    else
    {
COS001:
        set @pos, dialog(0, 0, 1, "정말 수고하셨습니다. 보상금 여기있습니다. 다음은 " + getarg(4) + "입니다. " + getarg(4) + "의뿔 " + getarg(1) + "개를 모아와 주십시오.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "갈수록 보상금이 높아지니 부탁드립니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set $oxygen_clear, getarg(3);
        set_money @sd, get_money(@sd)+getarg(2);
        updatestatus @sd, 2;
        delitem @sd, name2itemid(getarg(0)), getarg(1);
        message @sd, 3, getarg(0) + " 주었습니다";
    }
    return;
}