{
    set @sd, mysd();
    set @id, myid();

    if($clear_shiled == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "요즘 용궁을 드나드는 외분인들이 부쩍 늘었어요.\\n\\n그래서 용궁의 물이 점차 더러워 지고 있는 것 같아요.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "용궁의 물을 다시 정화시키는 방법을 알고 싶은데 그 방법을 마침 랑구륜이 알고 있다고 하더라구요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, list2(@list, 0, 1, "가서 좀 알아봐 주실 수 있나요?", "물론입니다.", "별로..내키지가 않아서...");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
        else if(@list == 1)
        {
            set $clear_shiled, 1;
            addlegend @sd, 23, 7, 0, 1, "용궁정화 퀘스트를 받다.";
            set @pos, dialog(0, 0, 1, "고마워요.\\n\\n한시라도 빨리 정화하는 방법을 알아다 주세요.");
            end;
        }
        else end;
    }
    if($clear_shiled >= 1 && $clear_shiled <= 2)
    {
        set @pos, dialog(0, 0, 1, "물을 정화시키는 방법은 랑구륜에게 가보시면 될거에요.");
        end;
    }
    if($clear_shiled == 3)
    {
        if(countitem(name2itemid("정화비서")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 물을 정화시키는 법을 알아오지 못하셨군요.");
            end;
        }
        else
        {
COS004:
            set @pos, dialog(0, 0, 1, "아 비법을 알아오셨네요.\\n\\n어디 한번 볼까요.");
            if(@pos == 0) end;
COS005:
            set @pos, dialog(0, 1, 1, "음........으으으으음..........\\n\\n아. 이렇게 간단한 것을....");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS004;
COS006:
            set @pos, dialog(0, 1, 1, "하지만 이 방법을 실행하기 위해서는 숯의정화가 필요하네요. 가서 숯의정화 3조각만 가져다 주세요.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS005;
COS007:
            set @pos, dialog(0, 1, 1, "숯의정화를 만들기 위해서는 불의 힘이 깃든 물건이 필요하겠네요.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS006;

            set $clear_shiled, 4;
            delitem @sd, name2itemid("정화비서"), 1;
            message @sd, 3, "정화비서 주었습니다";
            addlegend @sd, 23, 7, 0, 1, "우선녀의 부탁을 들어주자.";
            end;
        }
    }
    if($clear_shiled == 4)
    {
        if(countitem(name2itemid("숯의정화")) < 3)
        {
            set @pos, dialog(0, 0, 0, "물을 정화하기 위해 숯의정화 3조각만 구해주세요.");
            end;
        }
        else
        {
            set $clear_shiled, 5;
            delitem @sd, name2itemid("숯의정화"), 1;
            delitem @sd, name2itemid("숯의정화"), 1;
            delitem @sd, name2itemid("숯의정화"), 1;
            message @sd, 3, "숯의정화 주었습니다";
            additem @sd, name2itemid("정화의방패"), 1;
            addlegend @sd, 23, 7, 0, 1, "용궁정화에 성공하였다.";
COS008:
            set @pos, dialog(0, 0, 1, "아. 이제야 용궁이 다시 깨끗함을 찾을 수 있겠네요.");
            if(@pos == 0) end;
COS009:
            set @pos, dialog(0, 1, 0, "이건 감사의 뜻으로 드리는 것이니까 받아두세요.");
            if(@pos == -1) goto COS008;
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, get_name(@sd) + "덕분에 용궁의 물이 깨끗해졌어요.");
        end;
    }
}