{
    set @sd, mysd();
    set @id, myid();

    if($oxygen_clear == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "외부에서 오신분이십니까? 남경 지역을 지나실때 조심하시기 바랍니다.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "현재 산소라는 요괴 때문에 백성들이 피해가 이만저만이 아니라하여 소탕작전을 위해 출동하였습니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "하지만 워낙 산소의 종류와 수가 많고 그에 비해 병사의 수가 터무니없이 적어 어려움이 많습니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, list2(@list, 0, 1, "산소를 소탕하는데 힘을 빌려주시지 않겠습니까? 도와주시면 보상금을 드리도록 하겠습니다.", "좋습니다! 제 힘이 도움이 된다면 기꺼이..", "죄송하지만 갈길이 멀답니다. 이만.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;
        else if(@list == 1)
        {
            set @pos, dialog(0, 0, 1, "우선 산소들을 상대하실 수 있는지 봐야겠습니다.");
            if(@pos == 0) end;

            if(get_level(@sd) < 50)
            {
                set @pos, dialog(0, 0, 0, "아직 산소들을 상대하시기엔 무리인 것 같습니다. 마음만은 고맙게 받겠습니다.");
                end;
            }
            else
            {
COS005:
                set @pos, dialog(0, 0, 1, "좋습니다! 이정도라면 충분합니다. 산소들은 종류마다 뿔을 가지고 있습니다. 이 뿔은 증표로 모아오시면 그에 따른 보상금을 드리겠습니다.");
                if(@pos == 0) end;
COS006:
                set @pos, dialog(0, 1, 1, "우선 약한 산소부터 시작하도록 하죠. 연청산소가 가장 약합니다. 연청산소의 뿔을 50개 모아와 주시기 바랍니다.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS005;

                set $oxygen_clear, 1;
                addlegend @sd, 4, 7, 0, 1, "연청산소의 뿔을 50개 모아가자.";
                end;
            }
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그러시군요.. 가시는 길 조심히 살펴서 가세요.");
            end;
        }
        else end;
    }
    if($oxygen_clear == 1)
    {
        callfunc "OXYGENCLEAR", "연청산소의뿔", 50, 3000, 2, "백산소";
        addlegend @sd, 4, 7, 0, 1, "백산소의 뿔을 50개 모아가자.";
    }
    if($oxygen_clear == 2)
    {
        callfunc "OXYGENCLEAR", "백산소의뿔", 50, 5000, 3, "황산소";
        addlegend @sd, 4, 7, 0, 1, "황산소의 뿔을 50개 모아가자.";
    }
    if($oxygen_clear == 3)
    {
        callfunc "OXYGENCLEAR", "황산소의뿔", 30, 10000, 4, "청산소괴";
        addlegend @sd, 4, 7, 0, 1, "청산소괴의 뿔을 50개 모아가자.";
    }
    if($oxygen_clear == 4)
    {
        callfunc "OXYGENCLEAR", "청산소괴의뿔", 30, 15000, 5, "적산소괴";
        addlegend @sd, 4, 7, 0, 1, "적산소괴의 뿔을 30개 모아가자.";
    }
    if($oxygen_clear == 5)
    {
        callfunc "OXYGENCLEAR", "적산소괴의뿔", 30, 20000, 6, "녹산소괴";
        addlegend @sd, 4, 7, 0, 1, "녹산소괴의 뿔을 30개 모아가자.";
    }
    if($oxygen_clear == 6)
    {
        if(countitem(name2itemid("녹산소괴의뿔")) < 30)
        {
            set @pos, dialog(0, 0, 0, "아직 녹산소괴의뿔 30개를 모아오지 못하신 것 같군요?");
            end;
        }
COS007:
        set @pos, dialog(0, 0, 1, "정말 감사드립니다. 여기까지 도와주셔서 저희에겐 너무나 큰 도움이 되었답니다.");
        if(@pos == 0) end;
COS008:
        set @pos, dialog(0, 1, 1, "상당히 강하신분 같은데 한가지 더 부탁드려도 될까요. 산소들의 우두머리인 산소괴왕은 저희가 건드릴 수가 없답니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS007;
COS009:
        set @pos, list2(@list_2, 0, 1, "산소괴왕을 처리해주시겠습니까?", "알겠습니다. 제가 처리해보도록 하죠.", "죄송합니다. 그건 저에게 무리일듯하군요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS008;
        else if(@list_2 == 1)
        {
            delitem @sd, name2itemid("녹산소괴의뿔"), 30;
            message @sd, 3, "녹산소괴의뿔 주었습니다";
            set_money @sd, get_money(@sd)+30000;
            updatestatus @sd, 2;
            set $oxygen_clear, 7;
            addlegend @sd, 4, 7, 0, 1, "산소괴왕의 뿔을 구하자.";
            set @pos, dialog(0, 0, 1, "산소괴왕은 다른 산소들과는 차원이 다릅니다. 더구나 잘 나타나지 않아 찾기가 더 힘들답니다.\\n\\n산소괴왕의뿔을 가져오시면 귀한 물건을 드리도록 하죠. 부디 조심하시기 바랍니다.");
            end;
        }
        else if(@list_2 == 2)
        {
            set @pos, dialog(0, 0, 0, "그러시다면야 할 수 없죠.");
            end;
        }
        else end;
    }
    if($oxygen_clear == 7)
    {
        if(countitem(name2itemid("산소괴왕의뿔")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 산소괴왕의뿔을 구하시지 못하셨군요. 부디 조심하시길 바랍니다.");
            end;
        }
COS010:
        set @pos, dialog(0, 0, 1, "정말 대단하군요. 설마 정말 산소괴왕을 잡아오시리라고는. 이로써 큰 걱정을 덜게 되었군요.");
        if(@pos == 0) end;
COS011:
        set @pos, dialog(0, 1, 1, "정말 감사드립니다. 그 보답으로 이 보검을 드리죠. 황제에게 특별히 하사받은것인데 저에겐 과분한 물건인듯 하군요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS010;
COS012:
        set @pos, dialog(0, 1, 1, "부디 좋은 곳에 써주시기 발바니다. 그럼 안녕히.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS011;

        set $oxygen_clear, 0;
        delitem @sd, name2itemid("산소괴왕의뿔"), 1;
        message @sd, 3, "산소괴왕의뿔 주었습니다";
        additem @sd, name2itemid("흑영패도"), 1;
        addlegend @sd, 4, 7, 0, 1, "산소소탕 작전을 성공적으로 수행하였다.";
    }
}