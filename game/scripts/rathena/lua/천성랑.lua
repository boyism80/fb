{
    image 0, 50, 0;
    set @sd, mysd();
    set @id, myid();
    set @joblv, get_joblevel(@sd);

    if($fourclass == 7) { goto POS00000; }
    else { set @pos, dialog(1, 0, 1, "..."); end; }
POS00000:
    if(get_job(@sd) == 1) { set @jobname$, "검성"; }
    else if(get_job(@sd) == 2) { set @jobname$, "태성"; }
    else if(get_job(@sd) == 3) { set @jobname$, "현자"; }
    else if(get_job(@sd) == 4) { set @jobname$, "진선"; }

    set @pos, list2(@ret, 1, 0, "당신은 지금껏 수많은 수련을 거쳐 인간으로서 오르기\\n힘든 경지에 도달하셨고, 더욱이 세상을 종말로부터\\n보호하는 업적을 세우셨으니 " + @jobname$ + "으로 불리우시기에\\n부족함이 없습니다.\\n지금 바로 승급을 원하시나요?", "예", "아뇨");
    if(@pos == 0) { end; }
    else if(@ret == 1)
    {
        setarray @hp[0], 1400000, 1400000, 650000, 600000;
        setarray @mp[0], 0, 0, 900000, 750000;

        if(get_level(@sd) >= 99 && get_purehp(@sd) >= @hp[get_job(@sd)-1] && get_puremp(@sd) >= @mp[get_job(@sd)-1])
        {
            if(countitem(name2itemid("반고의심장")) == 0)
            {
                set @pos, dialog(0, 0, 0, "반고의심장이 없습니다.");
            }
            else
            {
                delitem @sd, name2itemid("반고의심장"), 1;
                message @sd, 3, "반고의심장 주었습니다";

                set_joblevel @sd, @joblv+1;

                spellaether @sd, name2spellid("승급보상"), 1;

                broadcast 4, "(( [" + get_name(@sd) + "]님이 " + get_jobname(@sd) + "으로 승급하였습니다. 축하합니다! ))";

                set $fourclass, 8;
            }
            deletearray @hp[0], getarraysize(@hp);
            deletearray @mp[0], getarraysize(@mp);
        }
        else
        {
            set @pos, dialog(0, 0, 0, "좀더 수련을 쌓고오세요.");
            end;
        }
    }
}