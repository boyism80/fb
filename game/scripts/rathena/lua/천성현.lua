{
    image 0, 220, 0;
    set @sd, mysd();
    set @id, myid();
    if($thirdclass == 2) { goto POS000; }
    else if($thirdclass == 3) { goto POS00; }
    else if(get_joblevel(@sd) < 2 || $thirdclass) { set @pos, dialog(0, 0, 1, "안녕하십니까?"); end; }
POS00000:
    set @pos, list2(@ret, 0, 0, "안녕하세요. 어떻게 오셨나요?", "3차 승급을 원합니다");
    if(@pos == 0) { end; }
    if(@ret == 1)
    {
POS0000:
        set @pos, dialog(0, 0, 1, "안녕하세요? 저의 이름은 천성현이라고 합니다.");
        if(@pos == 0) { end; }
POS0001:
        set @pos, dialog(0, 1, 1, "현제 이 세계는 불, 물, 흙, 바람, 신성, 암흑의 조화로 이루어져 있습니다.");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS0000; }
POS0002:
        set @pos, dialog(0, 1, 1, "그런데 어느순간 몬스터들의 나타나서 침입이 잦아짐으로써 점점 한쪽으로 치우치는 중이랍니다.");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS0001; }
        set @pos, dialog(0, 0, 1, "그런데 이렇게 있다가는 아무래도 세계가 붕괴될것같기에 이렇게 " + get_name(@sd) + "님께 도움을 요청을 드리는 바입니다.");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS0002; }
        set @ret, list1("도와주시겠습니까?", "당연히 도와야죠!", "흠.. 나중에.....");
        if(@ret == 1)
        {
            set $thirdclass, 1;
            additem @sd, name2itemid("승급의서장"), 1;
            addlegend @sd, 6, 7, 0, 12, "3차 승급 퀘스트를 받다!";
            set @pos, dialog(0, 0, 0, "그렇다면 어서 길림성으로 가셔서 신수의 부름을 받드세요!!");
        }
    }
    end;
POS000:
    if(countitem(name2itemid("고대마법서'상")) > 0 && countitem(name2itemid("고대마법서'하")) > 0)
    {
POS001:
        set @pos, dialog(0, 0, 1, "오오.. 자네 의외로 빨리 해왔군 그래!!");
        if(@pos == 0) { end; }
        set @pos, dialog(0, 1, 1, "마지막으로 부탁하나 합세, 길림성에 잠들어있는 암흑왕를 잡고 증표인 암흑왕의봉인을 가져다 주게나!");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS001; }
        delitem @sd, name2itemid("고대마법서'상"), 1;
        delitem @sd, name2itemid("고대마법서'하"), 1;
        set $thirdclass, 3;
        addlegend @sd, 6, 7, 0, 12, "3차 승급 마지막 퀘스트를 받다!";
    }
    else { set @pos, dialog(0, 0, 1, "..."); end; }
    end;
POS00:
    if(countitem(name2itemid("암흑왕의봉인")) > 0)
    {
        set @pos, dialog(0, 0, 1, "고맙네! 나는 이걸로 다시 이세계의 조화를 마출수 있게되었으니... 나의 조그마한 보답을 받아주시게... 그럼 이만...");
        if(@pos == 0) { end; }
        set $thirdclass, 4;
        additem @sd, name2itemid("천성현의증표"), 1;
        delitem @sd, name2itemid("암흑왕의봉인"), 1;
        delitem @sd, name2itemid("승급의서장"), 1;
    }
    else { set @pos, dialog(0, 0, 1, "..."); end; }
    end;
}