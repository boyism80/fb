{
    set @sd, mysd();
    set @id, myid();

    image 0, 0, 0;

    if(countitem(name2itemid("봉래산전설")) > 0)
    {
        warp @sd, name2mapid("난파선선장의방"), 1, rand(10, 13);
    }
    else
    {
        set @pos, dialog(0, 0, 0, "불길한 기운에 가로막혀 들어갈 수 없다.");
        set_x @id, get_x(@id)-1;
        updatexy @sd;
        updatelookpc @sd, 0;
        updateallpc @sd;
    }
}