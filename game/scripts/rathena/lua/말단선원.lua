{
    set @sd, mysd();
    set @id, myid();

    if($goose_egg == 0)
    {
        set @pos, list2(@list, 1, 0, "아이고~ 이거 큰일이네! 어떡하지? 엉엉 울고싶다 정말.", "무슨 일이세요?", "(그냥 지나친다)");
        if(@pos == 0) end;
        else if(@list != 1) end;
COS001:
        set @pos, dialog(1, 0, 1, "이 근방은 우리 대양선단의 주요 식량 채취 장소 인데요, 해물, 해초, 과일 등이 풍부한 반면 항상 사나운 두꺼비들과 일본게들이 득시글거려 매번 이렇게 고생이랍니다.");
        if(@pos == 0) end;
COS002:
        set @pos, list2(@list_2, 1, 1, "아 글쎄.. 이번엔 제가 애써 모아놓은 기러기알들을 그놈들이 다 훔쳐갔지 뭡니까요! 흑흑. 그걸 어떻게 되찾는다.", "흠. 그거라면.. 제가 그놈들을 해치우고 되찾아 드리지요.", "어쩔 수 없죠 뭐..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        else if(@list_2 != 1) end;
COS003:
        set $goose_egg, 1;
        set @pos, dialog(0, 0, 0, "헛! 정말입니까? 꼭 좀 부탁드립니다.\\n알을 찾아오시면 개당 200전씩 사례하겠습니다.");
        end;
    }
    else
    {
        set @pos, list2(@list_3, 0, 0, "기러기알은 찾아오셨나요?", "네", "아니오");
        if(@pos == 0) end;
        else if(@list_3 != 1) end;

        if(countitem(name2itemid("기러기알")) == 0)
        {
            set @pos, dialog(0, 0, 0, "기러기알 없습니다.");
            end;
        }
        else
        {
            set @count, countitem(name2itemid("기러기알"));
            delitem @sd, name2itemid("기러기알"), @count;
            message @sd, 3, "기러기알 주었습니다";
            set_money @sd, get_money(@sd)+(200*@count);
            set @pos, dialog(0, 0, 1, "기러기알 " + @count + "개에 대한 대가로 " + (200*@count) + "전을 드렸습니다.");
            end;
        }
    }
}