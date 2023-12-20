{
    set @sd, mysd();
    set @id, myid();

    if($dotaeyeon == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "안녕하시오~ 도삭산코뿔소말이오. 그놈 이빨과 발톱, 뿔 정말 단단하게 생기지 않았소?");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "코뿔소의 이빨,발톱,뿔은 무기나 갑옷을 만드는데 정말 좋은 재료가 되지. 그중에서도 도삭산코뿔소는 내가 지금까지 본 것들중에서 제일이더군.\\n\\n그래서 요즘 도삭산코뿔소를 사냥하고 있는데 놈들이 워낙 사나워서 사냥이 매우 위험하다오.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "어떻소 도삭산코뿔소를 사냥해서 이빨,발톱,뿔을 내게 구해다 주지 않겠소? 물론 그냥 가져다 달라는것은 아니니 걱정 마시오. 내 좋은 물건 하나 주도록하지!");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, list2(@list, 0, 0, "내 제안이 어떻소?", "좋습니다. 맘에 드는군요.", "죄송하지만 전 갈길이 바빠서..");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $dotaeyeon, 1;
            addlegend @sd, 12, 7, 0, 1, "도태연의 부탁을 들어주자!";
            set @pos, dialog(0, 0, 1, "다시 한번 말하지만 도삭산코뿔소의 발톱,이빨,뿔 모두 구해다 줘야하오~\\n\\n하나라도 빠지면 내 선물을 줄 수 없으니 명심하게나. 그럼 몸조심하구려. 놈들이 정말 사납거든.");
            end;
        }
        else end;
    }
    if($dotaeyeon == 1)
    {
COS005:
        set @pos, dialog(0, 0, 1, "어디보자~ 코뿔소 사냥은 많이 했소? 발톱,이빨,뿔 다 가지고 왔는지 봅시다.");
        if(@pos == 0) end;
COS006:
        if(countitem(name2itemid("도삭산코뿔소발톱")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 발톱을 구하지 못한 모양이군..");
            end;
        }
        set @pos, dialog(0, 1, 1, "발톱은 있고~ 역시 튼튼해본인다니깐. 이빨은?");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS005;
COS007:
        if(countitem(name2itemid("도삭산코뿔소이빨")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 이빨을 구하지 못한 모양이군..");
            end;
        }
        set @pos, dialog(0, 1, 1, "이빨도 구했고, 뿔은 구하기 쉽지 않을텐데 가져왔소?");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS006;
COS008:
        if(countitem(name2itemid("도삭산코뿔소뿔")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 뿔을 구하지 못한 모양이군..");
            end;
        }
        set @pos, dialog(0, 1, 1, "오! 세가지 다 구해왔구려. 역시 내가 사람보는 눈은 정확하다니깐. 허허~\\n\\n그러면 내가 다 가져가겠소.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS007;
COS009:
        set $dotaeyeon, 2;
        delitem @sd, name2itemid("도삭산코뿔소발톱"), 1;
        delitem @sd, name2itemid("도삭산코뿔소이빨"), 1;
        delitem @sd, name2itemid("도삭산코뿔소뿔"), 1;
        message @sd, 3, "도삭산코뿔소발톱 주었습니다";
        message @sd, 3, "도삭산코뿔소이빨 주었습니다";
        message @sd, 3, "도삭산코뿔소뿔 주었습니다";
        additem @sd, name2itemid("도삭산황금봉"), 1;
        addlegend @sd, 12, 7, 0, 1, "도삭산 400층 퀘스트 완료";
        set @pos, dialog(0, 0, 1, "자 여기 내가 주는 선물이오. 도삭산의 비밀에 관련된 것 같은데.. 언젠가 쓸 일이 있을 것이오.");
        end;
    }
    else
    {
        set @pos, dialog(0, 0, 1, "안녕하신가? 덕분에 아무 훌륭한 무기와 방어구를 제작했다네. 나중에 한번 구경 오시게나.");
        end;
    }
}