{
    set @sd, mysd();
    set @id, myid();
    set @last, lastkillmobid(@sd);

    if(@last == name2mobid("자호"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "자호";
                }
            }
        }
        else
        {
            runscript @sd, "자호";
        }
    }
    else if(@last == name2mobid("친자호"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "친자호";
                }
            }
        }
        else
        {
            runscript @sd, "친자호";
        }
    }
    else if(@last == name2mobid("구자호"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "구자호";
                }
            }
        }
        else
        {
            runscript @sd, "구자호";
        }
    }
    else if(@last == name2mobid("거미"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "거미";
                }
            }
        }
        else
        {
            runscript @sd, "거미";
        }
    }
    else if(@last == name2mobid("사마귀"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "사마귀";
                }
            }
        }
        else
        {
            runscript @sd, "사마귀";
        }
    }
    else if(@last == name2mobid("거미랑"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "거미랑";
                }
            }
        }
        else
        {
            runscript @sd, "거미랑";
        }
    }
    else if(@last == name2mobid("사마귀랑"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "사마귀랑";
                }
            }
        }
        else
        {
            runscript @sd, "사마귀랑";
        }
    }
    else if(@last == name2mobid("사마귀"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "사마귀";
                }
            }
        }
        else
        {
            runscript @sd, "사마귀";
        }
    }
    else if(@last == name2mobid("서현거미"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "서현거미";
                }
            }
        }
        else
        {
            runscript @sd, "서현거미";
        }
    }
    else if(@last == name2mobid("사마귀"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "사마귀";
                }
            }
        }
        else
        {
            runscript @sd, "사마귀";
        }
    }
    else if(@last == name2mobid("가재"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "가재";
                }
            }
        }
        else
        {
            runscript @sd, "가재";
        }
    }
    else if(@last == name2mobid("전갈"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "전갈";
                }
            }
        }
        else
        {
            runscript @sd, "전갈";
        }
    }
    else if(@last == name2mobid("가재장"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "가재장";
                }
            }
        }
        else
        {
            runscript @sd, "가재장";
        }
    }
    else if(@last == name2mobid("전갈장"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "전갈장";
                }
            }
        }
        else
        {
            runscript @sd, "전갈장";
        }
    }
    else if(@last == name2mobid("사마귀"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "사마귀";
                }
            }
        }
        else
        {
            runscript @sd, "사마귀";
        }
    }
    else if(@last == name2mobid("서현가재"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "서현가재";
                }
            }
        }
        else
        {
            runscript @sd, "서현가재";
        }
    }
    else if(@last == name2mobid("유령"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "유령";
                }
            }
        }
        else
        {
            runscript @sd, "유령";
        }
    }
    else if(@last == name2mobid("초급유령"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "초급유령";
                }
            }
        }
        else
        {
            runscript @sd, "초급유령";
        }
    }
    else if(@last == name2mobid("중급유령"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "중급유령";
                }
            }
        }
        else
        {
            runscript @sd, "중급유령";
        }
    }
    else if(@last == name2mobid("사마귀"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "사마귀";
                }
            }
        }
        else
        {
            runscript @sd, "사마귀";
        }
    }
    else if(@last == name2mobid("고급유령"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "고급유령";
                }
            }
        }
        else
        {
            runscript @sd, "고급유령";
        }
    }
    else if(@last == name2mobid("자령"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "자령";
                }
            }
        }
        else
        {
            runscript @sd, "자령";
        }
    }
    else if(@last == name2mobid("주령"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "주령";
                }
            }
        }
        else
        {
            runscript @sd, "주령";
        }
    }
    else if(@last == name2mobid("지령"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "지령";
                }
            }
        }
        else
        {
            runscript @sd, "지령";
        }
    }
    else if(@last == name2mobid("해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "해골";
                }
            }
        }
        else
        {
            runscript @sd, "해골";
        }
    }
    else if(@last == name2mobid("칼든해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "칼든해골";
                }
            }
        }
        else
        {
            runscript @sd, "칼든해골";
        }
    }
    else if(@last == name2mobid("날쌘해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "날쌘해골";
                }
            }
        }
        else
        {
            runscript @sd, "날쌘해골";
        }
    }
    else if(@last == name2mobid("쾌도해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "쾌도해골";
                }
            }
        }
        else
        {
            runscript @sd, "쾌도해골";
        }
    }
    else if(@last == name2mobid("흑해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "흑해골";
                }
            }
        }
        else
        {
            runscript @sd, "흑해골";
        }
    }
    else if(@last == name2mobid("자해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "자해골";
                }
            }
        }
        else
        {
            runscript @sd, "자해골";
        }
    }
    else if(@last == name2mobid("주해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "주해골";
                }
            }
        }
        else
        {
            runscript @sd, "주해골";
        }
    }
    else if(@last == name2mobid("지해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "지해골";
                }
            }
        }
        else
        {
            runscript @sd, "지해골";
        }
    }
    else if(@last == name2mobid("쾌도해골"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "쾌도해골";
                }
            }
        }
        else
        {
            runscript @sd, "쾌도해골";
        }
    }
    else if(@last == name2mobid("처녀귀신"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "처녀귀신";
                }
            }
        }
        else
        {
            runscript @sd, "처녀귀신";
        }
    }
    else if(@last == name2mobid("달걀귀신"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "달걀귀신";
                }
            }
        }
        else
        {
            runscript @sd, "달걀귀신";
        }
    }
    else if(@last == name2mobid("불귀신"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "불귀신";
                }
            }
        }
        else
        {
            runscript @sd, "불귀신";
        }
    }
    else if(@last == name2mobid("독충"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "독충";
                }
            }
        }
        else
        {
            runscript @sd, "독충";
        }
    }
    else if(@last == name2mobid("몽달귀신"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "몽달귀신";
                }
            }
        }
        else
        {
            runscript @sd, "몽달귀신";
        }
    }
    else if(@last == name2mobid("빗자루귀신"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "빗자루귀신";
                }
            }
        }
        else
        {
            runscript @sd, "빗자루귀신";
        }
    }
    else if(@last == name2mobid("선비검객"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "선비검객";
                }
            }
        }
        else
        {
            runscript @sd, "선비검객";
        }
    }
    else if(@last == name2mobid("선비평민"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "선비평민";
                }
            }
        }
        else
        {
            runscript @sd, "선비평민";
        }
    }
    else if(@last == name2mobid("흉노평민"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "흉노평민";
                }
            }
        }
        else
        {
            runscript @sd, "흉노평민";
        }
    }
    else if(@last == name2mobid("흉노전사"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "흉노전사";
                }
            }
        }
        else
        {
            runscript @sd, "흉노전사";
        }
    }
    else if(@last == name2mobid("흉노무사"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "흉노무사";
                }
            }
        }
        else
        {
            runscript @sd, "흉노무사";
        }
    }
    else if(@last == name2mobid("녹살쾡이"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "녹살쾡이";
                }
            }
        }
        else
        {
            runscript @sd, "녹살쾡이";
        }
    }
    else if(@last == name2mobid("표황살쾡이"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "표황살쾡이";
                }
            }
        }
        else
        {
            runscript @sd, "표황살쾡이";
        }
    }
    else if(@last == name2mobid("외눈황천구"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "외눈황천구";
                }
            }
        }
        else
        {
            runscript @sd, "외눈황천구";
        }
    }
    else if(@last == name2mobid("외눈자천구"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "외눈자천구";
                }
            }
        }
        else
        {
            runscript @sd, "외눈자천구";
        }
    }
    else if(@last == name2mobid("흑혈후"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "흑혈후";
                }
            }
        }
        else
        {
            runscript @sd, "흑혈후";
        }
    }
    else if(@last == name2mobid("녹혈후"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "녹혈후";
                }
            }
        }
        else
        {
            runscript @sd, "녹혈후";
        }
    }
    else if(@last == name2mobid("흑선문후"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "흑선문후";
                }
            }
        }
        else
        {
            runscript @sd, "흑선문후";
        }
    }
    else if(@last == name2mobid("흡혈강시"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "흡혈강시";
                }
            }
        }
        else
        {
            runscript @sd, "흡혈강시";
        }
    }
    else if(@last == name2mobid("아기강시"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "아기강시";
                }
            }
        }
        else
        {
            runscript @sd, "아기강시";
        }
    }
    else if(@last == name2mobid("흑발강시"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    runscript @target_sd, "흑발강시";
                }
            }
        }
        else
        {
            runscript @sd, "흑발강시";
        }
    }
    return;
}