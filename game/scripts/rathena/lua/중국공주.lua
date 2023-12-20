{
    set @sd, mysd();
    set @id, myid();

    if($princess_ring == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "제가 강에서 배를 타다가 반지를 빠뜨리고 말았어요. 그 반지는 옆나라의 제 정혼자인 왕자가 준 반지라 잃어버리면 그 나라와의 관계까지 위험해질수 있어요.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "찾으러 여러분이 들어갔지만 아직도 나오신 분이 없네요. 아무래도 평범한 강이 아닌거 같아요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, list2(@list, 0, 1, "제발 부탁드려요. 반지를 찾아주세요.", "걱정마세요. 제가 꼭 찾아드리죠.", "흠. 전 물을 싫어해서요. 죄송하네요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
        else if(@list == 1)
        {
            set @pos, dialog(0, 0, 1, "정말 고마워요. 그럼 믿고 기다리죠.");
            if(@pos == 0) end;

            set $princess_ring, 1;
            addlegend @sd, 3, 7, 0, 1, "중국공주의 잃어버린 반지를 되찾자.";
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그러시다면야.. 이를 어쩐담...");
            end;
        }
        else end;
    }
    if($princess_ring == 1)
    {
        if(countitem(name2itemid("공주의반지")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 제 반지가 없으신거같은데..");
            end;
        }
        
        set @pos, dialog(0, 0, 1, "정말 감사해요. 정말... 이 은혜는 잊지 못할꺼에요. 이건 약소하지만 제가 드리는 선물입니다.");
        if(@pos == 0) end;

        set $princess_ring, 2;
        delitem @sd, name2itemid("공주의반지"), 1;
        message @sd, 3, "공주의반지 주었습니다";
        additem @sd, name2itemid("청옥반지"), 1;
        addlegend @sd, 3, 7, 0, 1, "잃어버린 공주의 반지를 되찾아 주다.";
        end;
    }
    if($princess_ring == 2)
    {
        set @pos, dialog(0, 0, 1, "저번엔 정말로 감사했습니다.");
        end;
    }
}