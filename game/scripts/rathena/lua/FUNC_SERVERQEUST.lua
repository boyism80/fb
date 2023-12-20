{
    set @sd, mysd();
    set @id, myid();
    set @last, lastkillmobid(@sd);

    if(@last == name2mobid("성문1") || @last == name2mobid("성문2") || @last == name2mobid("성문3") || @last == name2mobid("성문4") || @last == name2mobid("성문5") || @last == name2mobid("성문6") || @last == name2mobid("성문7") || @last == name2mobid("성문8"))
    {
        announce 5, "[알림] " + get_clanname(@sd) + " 문파의 " + get_name(@sd) + "님이 성문을 뚫고 진격중입니다.";
    }
    else if(@last == name2mobid("복홍"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellaether @target_sd, name2spellid("복홍잡기"), 3;
                }
            }
        }
        else
        {
            spellaether @sd, name2spellid("복홍잡기"), 3;
        }
    }
    else if(@last == name2mobid("복어장군"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellcast @target_sd, name2spellid("태자전음문서");
                }
            }
        }
        else
        {
            spellcast @sd, name2spellid("태자전음문서");
        }
    }
    else if(@last == name2mobid("게장군"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellaether @target_sd, name2spellid("게장군포박"), 3;
                }
            }
            spellaether @sd, name2spellid("게장군포박"), 3;
        }
        else
        {
            spellaether @sd, name2spellid("게장군포박"), 3;
        }
    }
    else if(@last == name2mobid("문어장군"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellaether @target_sd, name2spellid("문어장군포박"), 3;
                }
            }
        }
        else
        {
            spellaether @sd, name2spellid("문어장군포박"), 3;
        }
    }
    else if(@last == name2mobid("해마병사"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellcast @target_sd, name2spellid("해마병사잡기");
                }
            }
        }
        else
        {
            spellcast @sd, name2spellid("해마병사잡기");
        }
    }
    else if(@last == name2mobid("해마장군"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellcast @target_sd, name2spellid("해마장군잡기");
                }
            }
        }
        else
        {
            spellcast @sd, name2spellid("해마장군잡기");
        }
    }
    else if(@last == name2mobid("인어장군"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellaether @target_sd, name2spellid("인어장군잡기"), 3;
                }
            }
        }
        else
        {
            spellaether @sd, name2spellid("인어장군잡기"), 3;
        }
    }
    else if(@last == name2mobid("상어장군"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellaether @target_sd, name2spellid("상어장군잡기"), 3;
                }
            }
            spellaether @sd, name2spellid("상어장군잡기"), 3;
        }
        else
        {
            spellaether @sd, name2spellid("상어장군잡기"), 3;
        }
    }
    else if(@last == name2mobid("해파리장군"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellaether @target_sd, name2spellid("해파리장군잡기"), 3;
                }
            }
        }
        else
        {
            spellaether @sd, name2spellid("해파리장군잡기"), 3;
        }
    }
    else if(@last == name2mobid("해파리수하") || @last == name2mobid("해파리부하"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellcast @target_sd, name2spellid("전략문서");
                }
            }
        }
        else
        {
            spellcast @sd, name2spellid("전략문서");
        }
    }
    else if(@last == name2mobid("청의태자"))
    {
        if(party(@sd) != 0)
        {
            for(set @loop, 0; @loop < partymax(); set @loop, @loop+1)
            {
                set @target_sd, party_member(@sd, @loop);
                if(@target_sd > 0 && get_mapid(@sd) == get_mapid(@target_sd))
                {
                    spellaether @target_sd, name2spellid("청의태자포획"), 3;
                }
            }
        }
        else
        {
            spellaether @sd, name2spellid("청의태자포획"), 3;
        }
    }
    else if(@last == name2mobid("토깽이"))
    {
        if($rabbit_liver == 1)
        {
            spellaether @sd, name2spellid("토깽이포획"), 3;
        }
    }
    return;
}