{
    set @sd, mysd();
    set @id, myid();

    if($dojaeyoung < 2)
    {
COS001:
        set @pos, dialog(0, 0, 1, "쿨럭..쿨럭..");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 0, "(안색이 별로 좋지 않다.)");
        if(@pos == -1) goto COS001;
        end;
    }
    if($dojaeyoung == 2)
    {
COS003:
        set @pos, dialog(0, 0, 1, "오.. " + get_name(@sd) + "씨 아닌가. 아들놈에게 약초를 구해셨다고 하더군요. 덕분에 내가 이렇게 건강을 되찾았소. 고맙구려.");
        if(@pos == 0) end;
COS004:
        set @pos, dialog(0, 1, 1, "그나저나 말이지, 내 아들놈이 약초를 구해준게 기특해서 내가 선물을 하나 주려고 하는데 말이야, 아아 물론 구하기는 " + get_name(@sd) + "씨가 구했지만서도, 아들놈도 고생을 했다고.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;
COS005:
        set @pos, dialog(0, 1, 1, "예전에 도삭산을 지나가는데 인어들이 방울을 가지고 있는게 보이던데, 인어의방울이라고 하면 집에 걸어두기만 해도 흉한 것들이 집에 접근을 못한다고 하거든");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS004;
COS006:
        set @pos, dialog(0, 1, 1, "내가 건강이 나빠진것도 도삭산의 흉한 기운 때문인듯 한데, 아들놈한테 방울 하나쯤 주고 싶구만.\\n\\n내 직접 가진 못하고, 어떻게든 구했으면 좋겠는데...");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS005;
COS007:
        set @pos, list2(@list, 0, 0, get_name(@sd) + "씨가 하나 구해주시지 않겠소?", "제가 구해드리겠습니다.", "저는 좀 바빠서..");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $dojaeyoung, 3;
            addlegend @sd, 10, 7, 0, 1, "도성연의 부탁을 들어주자!";
COS008:
            set @pos, dialog(0, 0, 1, "오오. 고맙소. 인어의방울..은 도삭산고양이인어가 가지고 있지 않을까? 고양이목에 방울달기라는 말도 있지 않소? 하하하. 농담이지만 말이지.");
            if(@pos == 0) end;
COS009:
            set @pos, dialog(0, 1, 1, "뫼롱씨는 내 생명의 은인이니 292층으로 가는 지름길로 보내주지.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS008;
COS010:
            set @pos, list2(@list_2, 0, 0, "지금 지름길로 292층으로 가시겠소?", "네, 보내주십시오.", "나중에 가겠습니다.");
            if(@pos == 0) end;
            else if(@list_2 == 1)
            {
                warp @sd, name2mapid("도삭산292층"), 16, 37;
                end;
            }
            else end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "허허.. 바쁘시다면 할 수 없군요.");
            end;
        }
        else end;
    }
    if($dojaeyoung == 3)
    {
        if(countitem(name2itemid("인어의방울")) == 0)
        {
            set @pos, list2(@list_3, 0, 0, "아직 인어의방울을 구하지 못하셨군. 지금 지름길로 292층으로 가시겠소?", "네, 보내주십시오.", "나중에 가겠습니다.");
            if(@pos == 0) end;
            else if(@list_3 == 1)
            {
                warp @sd, name2mapid("도삭산292층"), 16, 37;
                end;
            }
            else end;
        }
        else
        {
COS011:
            set @pos, dialog(0, 0, 1, "오.. 이것이 인어의방울이군. 고맙소고맙소.");
            if(@pos == 0) end;
COS012:
            set @pos, dialog(0, 1, 1, "그런데 이거 정말 미안한 얘긴데.. 내 딸이 이 얘길 듣더니, 왜 오빠한테만 선물을 주냐고 칭얼대는거야. 자기도 인어의거울을 가지고 싶다나?");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS011;
COS013:
            set @pos, dialog(0, 1, 1, "인어의거울은 보면볼수록 얼굴이 예뻐진다는 얘기가 있는데, 그걸 어디서 주워들었는지 원참...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS012;
COS014:
            set @pos, list2(@list_4, 0, 0, get_name(@sd) + "씨, 인어의거울도 좀 구해다 주실수 없겠소?", "네, 구하는 김에 거울도 구하지요.", "갑자기 바쁜 일이 생겨서...");
            if(@pos == 0) end;
            else if(@list_4 == 1)
            {
                set $dojaeyoung, 4;
                delitem @sd, name2itemid("인어의방울"), 1;
                message @sd, 3, "인어의방울 주었습니다";
COS015:
                set @pos, dialog(0, 0, 1, "정말 고맙소. 인어의거울은.. 도삭산이쁘니인어가 가지고 있으려나? 하하하");
                if(@pos == 0) end;
COS016:
                set @pos, list2(@list_5, 0, 0, "지금 지름길로 292층에 가시겠소?", "네, 보내주십시오.", "나중에 가겠습니다.");
                if(@pos == 0) end;
                else if(@list_5 == 1)
                {
                    warp @sd, name2mapid("도삭산292층"), 16, 37;
                    end;
                }
                else end;
            }
            else if(@list_4 == 2)
            {
                set @pos, dialog(0, 0, 0, "그런가? 흐음..딸이 하도 칭얼대서 말이지..");
                end;
            }
            else end;
        }
    }
    if($dojaeyoung == 4)
    {
        if(countitem(name2itemid("인어의거울")) == 0)
        {
            set @pos, list2(@list_6, 0, 0, "아직 인어의거울을 구하지 못하셨군. 지금 지름길로 292층으로 가시겠소?", "네, 보내주십시오.", "나중에 가겠습니다.");
            if(@pos == 0) end;
            else if(@list_6 == 1)
            {
                warp @sd, name2mapid("도삭산292층"), 16, 37;
                end;
            }
        }
        else
        {
COS017:
            set @pos, dialog(0, 0, 1, "오,, 이것이 인어의거울이군! 고맙소!! 정말 고맙소!");
            if(@pos == 0) end;
COS018:
            set @pos, dialog(0, 1, 1, "이걸 어떻게 사례하나.. 그렇지. 우리 집안에 내려오는 목걸이를 하나 드리지.\\n\\n'봉마의목걸이'라고 해서.. 마귀를 봉인하여 담았다고 하는 옥으로 만든것이란느데.. 정말인지는 모르겠소.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS017;
COS019:
            set $dojaeyoung, 5;
            delitem @sd, name2itemid("인어의거울"), 1;
            message @sd, 3, "인어의거울 주었습니다";
            additem @sd, name2itemid("봉마의목걸이"), 1;
            addlegend @sd, 10, 7, 0, 1, "도삭산 200층 퀘스트 완료";
            set @pos, dialog(0, 0, 1, "뭐 어쨌든 기념으로 드릴테니.. 잘 쓰시오..\\n\\n그럼 건강하시오!");
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, get_name(@sd) + "씨군, 요세 잘 지내나? 당신 덕분에 감사한일이 정말 많소.");
        end;
    }
}