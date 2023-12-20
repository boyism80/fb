{
    set @sd, mysd();
    set @id, myid();

    if($dotaeyeon < 2)
    {
        set @pos, dialog(0, 0, 0, "에구구... 손녀딸이 자꾸 나가 논다는데 위험에서 큰일이야..");
        end;
    }
    if($dotaeyeon == 2)
    {
COS001:
        set @pos, dialog(0, 0, 1, get_name(@sd) + "씨 아닌가, 400층의 아들이 코뿔소의 뿔등을 얻었다고 굉장히 좋아하더구만.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "그런데 이 근처에 갑자기 사나운 동물들이 늘어나고 있어. 내 손녀는 자꾸 나가 놀겠다고 조르는데..\\n\\n얼마전부터 살쾡이와 악어가 늘어나기 시작해서 이제는 아예 집으로 들어오기도 할 정도란 말이지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "어른들도 함부로 대들지 못하는터라 다들 집에서 꼼짝못하고 있다우\\n\\n원래 도삭산 동물들은 온순해서 사람에게 해를 끼치는건 생각도 못했는데 왜 갑자기 이러는 건지..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, list2(@list, 0, 0, "우리를 좀 도와주시게, 코뿔소를 그렇게 쉽게 잡았으니 살쾡이와 악어를 내쫓을 수 있지 않을까?", "그러죠. 제가 처리하겠습니다!!", "저보다 나은 적임자가 분명 있을겁니다.");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $dotaeyeon, 3;
            addlegend @sd, 13, 7, 0, 1, "황만령의 부탁을 들어주자!";
COS005:
            set @pos, dialog(0, 0, 1, "정말 고맙네.. 살쾡이와 악어는 가죽이 질겨서 쓸곳이 참 많단 말야. 살쾡이와 악어를 처치하고 올때 가죽도 좀 가져다 줄 수 있겠나?");
            if(@pos == 0) end;
COS006:
            set @pos, dialog(0, 1, 1, "도삭산 살쾡이가죽 300개와 도삭산악어비늘 200개를 구해다주면 내 보물을 드리도록 하지.\\n\\n살쾡이와 악어는 무리를 지어서 공격해오니 조심하게!");
            if(@pos == -1) goto COS005;
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그런가? 그럼 좀 더 기다려봐야겠군..");
            end;
        }
        else end;
    }
    if($dotaeyeon == 3)
    {
        if(countitem(name2itemid("도삭산살쾡이가죽")) < 300 || countitem(name2itemid("도삭산악어비늘")) < 200)
        {
            set @pos, dialog(0, 0, 0, "아직 다 구하지 못한 모양이군.. 도삭산 살쾡이가죽 300개와 도삭산악어비늘 200개를 구해다주면 내 보물을 드리도록 하지.");
            end;
        }
        else
        {
COS007:
            set @pos, dialog(0, 0, 1, "오~ 정말 대단하이. 이렇게나 많이. 다친곳은 없나? 그러면 다 가져가겠네.");
            if(@pos == 0) end;
COS008:
            set $dotaeyeon, 4;
            delitem @sd, name2itemid("도삭산살쾡이가죽"), 300;
            delitem @sd, name2itemid("도삭산악어비늘"), 200;
            message @sd, 3, "도삭산살쾡이가죽 주었습니다";
            message @sd, 3, "도삭산악어비늘 주었습니다";
            additem @sd, name2itemid("수정의귀걸이"), 1;
            addlegend @sd, 13, 7, 0, 1, "도삭산 500층 퀘스트 완료";
            set @pos, dialog(0, 0, 1, "자 여기 내가 주는 선물이라네. 도와줘서 고맙네, 항상 좋은 일만 있게나!");
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "저번에 날 도와줘서 고마우이. 가죽은 아주 유용하게 사용하고 있다네.");
        end;
    }
}