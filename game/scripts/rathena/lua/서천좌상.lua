{
    set @sd, mysd();
    set @id, myid();

    if($greatwall_repair == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "마침 잘 만났네. 자네 만리장성을 돌아보았나? 요즘 만리장성의 곳곳이 무너져내려 걱정이 보통이 아니라네.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "만리장성을 수리하는데는 고구려의 벽돌만큼 좋은게 없지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, list2(@list, 0, 0, "자네가 만리장성의 무너져내린곳을 고쳐줄 수 있겠나?", "만리장성이!! 제가 꼭 고치겠습니다!!", "죄송하지만 저에겐 그럴만한 시간이 없습니다.");
        if(@pos == 0) end;
        else if(@list != 1) end;

        set $greatwall_repair, 1;
        spellaether @sd, name2spellid("만리장성수리"), 180;
        addlegend @sd, 21, 7, 0, 1, "만리장성 수리를 부탁받다.";
        set @pos, dialog(0, 0, 1, "수리에는 꼭 고구려의 벽돌을 써야만 하네. 벽돌은 국내성의 대장간에서 판다고 하더군. 만리장성을 돌아다니다 보면 수리해야 할곳을 찾을수 있을걸세. 10군데 이상 고친다면 적절한 보상을 해주겠네.");
        end;
    }
    if($greatwall_repair == 1)
    {
        if($greatwall_repair_count < 10)
        {
            set @pos, dialog(0, 0, 0, "조금 더 수리해야 할 것 같은데?");
            end;
        }
        else
        {
            if($greatwall_repair_count >= 10 && $greatwall_repair_count <= 49)
            {
                setarray @item$[0], "국내성비서", "장안성비서";
                setarray @value[0], 5, 5;
            }
            else if($greatwall_repair_count >= 50 && $greatwall_repair_count <= 99)
            {
                setarray @item$[0], "토깽이변신시약";
                setarray @value[0], 1;
            }
            else if($greatwall_repair_count >= 100 && $greatwall_repair_count <= 999)
            {
                setarray @item$[0], "청의태자변신시약";
                setarray @value[0], 1;
            }
            else if($greatwall_repair_count >= 1000 && $greatwall_repair_count <= 9999)
            {
                setarray @item$[0], "회호박결정";
                setarray @value[0], 1;
            }
            else if($greatwall_repair_count >= 10000)
            {
                setarray @item$[0], "천풍선";
                setarray @value[0], 1;
            }

            for(set @loop, 0; @loop < getarraysize(@item$); set @loop, @loop+1)
            {
                additem @sd, name2itemid(@item$[@loop]), @value[@loop];
            }

            deletearray @item$[0], getarraysize(@item$);
            deletearray @value[0], getarraysize(@value);

            set $greatwall_repair, 2;
            addlegend @sd, 21, 7, 0, 1, "만리장성 수리에 성공하다.";
            set @pos, dialog(0, 0, 1, "오~ 수고했네!! 자 여기 받게나. 시간이 있다면 더 수고해 주길 바라겠네. 수고하는 만큼 댓가는 지불할테니...");
            end;
        }
    }
    if($greatwall_repair == 2)
    {
        set @pos, dialog(0, 0, 1, "저번엔 정말 고마웠네. 당신과 같이 만리장성의 수리에 힘써주는 사람이 많아 만리장성이 점차 옛 모습을 되찾고 있네.");
        end;
    }
}