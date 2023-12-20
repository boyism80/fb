{
    set @sd, mysd();
    set @id, myid();

    if(get_name(@sd) == "연GM" || get_name(@sd) == "무휼GM" || get_name(@sd) == "세류GM" || get_name(@sd) == "유리GM" || get_name(@sd) == "해명GM" || get_name(@sd) == "배극GM" || get_name(@sd) == "하자GM" || get_name(@sd) == "호동GM" || get_name(@sd) == "주몽GM" || get_name(@sd) == "낙랑GM" || get_name(@sd) == "괴유GM")
    {
        set @pos, input2(@pass$, 0, 15, "비밀번호를 입력해주세요.", " ", " ");
        if(@pos == 0) { getoutofhere @sd; }
        else if(@pass$ == "gkgkgk")
        {
            message @sd, 0, get_name(@sd) + "님 안녕하세요?";
            set_gmlevel @sd, 2;
        }
        else { getoutofhere @sd; }
    }
    else
    {
        if(get_gmlevel(@sd) > 0)
        {
            set_gmlevel @sd, 0;
            getoutofhere @sd;
        }
    }
    return;
}