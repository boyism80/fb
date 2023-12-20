{
    set @sd, mysd();
    set @id, myid();

    if($myungsukboo == 1 || $myungsukboo == 3)
    {
        spellaether @sd, name2spellid("귀신퇴치부적"), 180;
    }
    if($geumgang_rock == 8)
    {
        spellaether @sd, name2spellid("표신궁고문서6"), 180;
    }
    if($donuhap == 1 || $donuhap == 2)
    {
        spellaether @sd, name2spellid("동충하초인삼"), 180;
    }
    if($dojaeyoung == 6)
    {
        spellaether @sd, name2spellid("만리향씨앗심기"), 180;
    }
    if($greatwall_repair == 7)
    {
        spellaether @sd, name2spellid("만리향씨앗"), 180;
    }
    if($hansangyak == 1)
    {
        spellaether @sd, name2spellid("적심토"), 180;
    }
    if($find_play_object == 1)
    {
        spellaether @sd, name2spellid("태자의장난감"), 180;
    }
    if($greatwall_repair == 1)
    {
        spellaether @sd, name2spellid("만리장성수리"), 180;
    }
    if($mountain_god_clothes == 1)
    {
        spellaether @sd, name2spellid("산신의보물지도"), 180;
    }
    if($totem_clothes == 1)
    {
        spellaether @sd, name2spellid("여신의이슬"), 180;
    }
    if($alcoholic_drink == 1)
    {
        spellaether @sd, name2spellid("청심초"), 180;
    }
    return;
}