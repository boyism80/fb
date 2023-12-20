{
    if($soul_doll == 4)
    {
COS004:
        set @pos, dialog(0, 0, 1, "또 보는구만. 전에 환상의 섬에서 봤었지? 난 조만간에 환상의 섬으로 또 놀러갈 거라네.");
        if(@pos == 0) end;
COS005:
        set @pos, dialog(0, 1, 1, "언제 가냐고? 마음 내키면 언제든지 갈 생각이야! 배표도 싸고 하니 여행하기 딱 좋더구만. 허허허...");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS004;

        if($soul_doll == 4) { set $soul_doll, 5; }
        end;
    }
COS001:
    set @pos, dialog(0, 0, 1, "환상의 섬에 가 본적이 있는가? 요즘 다들 한번 가 보자고 난리던데, 다녀온 사람들은 여행하기 딱 좋은 곳이라고 하더군.\\n\\n특히 전망대에서 내려다보는 풍경이 좋다고 하더구만!");
    if(@pos == 0) end;
COS002:
    set @pos, dialog(0, 1, 1, "그래서 지금 나도 가 보려고 하는 중일세. 자네도 생각 있으면 가 보시게나.");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS001;
COS003:
    set @pos, dialog(0, 1, 1, "길은 아직 나도 확실히는 모르겠고, 일단 성 남문으로 나가면서 사람들에게 물어보려고 하는데... 뭐, 혹시 남문에서 보게 되거든 아는체라도 해 보게나. 허허...");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS002;

    if($soul_doll == 0) { set $soul_doll, 1; }
}