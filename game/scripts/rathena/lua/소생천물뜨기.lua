{
    set @sd, mysd();
    set @id, myid();

    image 0, 0, 0;

    if(get_state(@sd) != 1)
    {
        if(countitem(name2itemid("물병")) > 0)
        {
            set @pos, dialog(0, 0, 1, "물병에 회복의 샘물을 가득 채웠습니다.");
            if(@pos == 0) end;

            delitem @sd, name2itemid("물병"), 1;
            message @sd, 3, "물병 주었습니다";
            additem @sd, name2itemid("회복의샘물"), 1;
            addlegend @sd, 2, 7, 0, 4, "소생천에서 회복의 샘물을 가득 채웠다.";
        }
    }
}