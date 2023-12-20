{
    image 0, getarg(2), getarg(3);
    set @sd, mysd();
    set @id, myid();
    set @totem$, getarg(0);
    set @list, list1("안녕하세요. 어떻게 오셨나요?", @totem$ + "성 입장");
    if(@list == 1)
    {
        if(clan_exist(@sd) != 0)
        {
            if(get_state(@sd) != 1)
            {
                if(!strcmp(get_clanname(@sd), getarg(1)))
                {
                    if(#clan_start == 0)
                    {
                        warp @sd, name2mapid(@totem$ + "의성"), rand(11, 17), rand(4, 11);
                    }
                    else
                    {
                        warp @sd, name2mapid(@totem$ + "성입구"), rand(49, 57), rand(145, 148);
                    }
                }
                else
                {
                    if(#clan_mapname$ == @totem$ + "성")
                    {
                        warp @sd, name2mapid(@totem$ + "성입구"), rand(49, 57), rand(145, 148);
                    }
                    else
                    {
                        set @pos, dialog(0, 0, 0, "현재 " + @totem$ + "성 공성이 진행중이지 않습니다.");
                    }
                }
            }
            else
            {
                set @pos, dialog(0, 0, 0, "유령은 참가할 수 없습니다.");
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 0, "가입된 문파가 없습니다.");
            end;
        }
    }
    return;
}