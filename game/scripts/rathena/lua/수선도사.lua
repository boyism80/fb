{
    set @sd, mysd();
    set @id, myid();

    if($repairascetic == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "난 천선도사와 죽마고우인 수선도사라네. 이 친구가 압록강의 절경을 혼자보기 너무 아깝다며 날 데려오지 뭔가.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "근데 와서 보니 걱정이 태산이라네. 이 친구는 아무것도 모르고 그저 아름답다고만 하지만 지금 압록강은 정상이 아니지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "속에서부터 썩어들어가고 있는데 이게 다 악어녀석들 때문이라네. 번식력이 뛰어나 갑자기 늘어가는 악어의 배설물 때문에...");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, dialog(0, 1, 1, "그래서 자네에게 부탁을 하나 하고 싶네만. 악어 녀석들의 피를 구해와 주는 것이지. 그것으로 강을 정화시킬 방법을 찾을수 있을걸세.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;
COS005:
        set @pos, list2(@list, 0, 1, "내 부탁을 들어주겠나?", "악어를 잡기만 하면 됩니까? 최선을 다하겠습니다.", "저에겐 무리겠군요. 죄송합니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS004;
        else if(@list == 1)
        {
COS006:
            set @pos, dialog(0, 0, 1, "그럼 일단 자네가 악어와 맞설 능력이 충분한지 살펴보겠네.");
            if(@pos == 0) end;
COS007:
            if(get_level(@sd) < 70)
            {
                set @pos, dialog(0, 0, 1, "자네는 아직 악어와 맞설 능력이 충분하지 않군.. 다음에 부탁하도록 하지.");
                end;
            }
            else
            {
                set $repairascetic, 1;
                addlegend @sd, 25, 7, 0, 8, "악어의피 3병을 구하자.";
COS008:
                set @pos, dialog(0, 0, 1, "좋네. 자네라면 믿고 부탁해도 되겠군. 악어들을 잡다보면 오염이 덜된 악어들이 있다네.");
                if(@pos == 0) end;
COS009:
                set @pos, dialog(0, 1, 1, "이녀석들의 순수한 피를모아 조사를 해보면 더러워진 압록강을 깨끗히 정화시킬수 있는 약을 만들수 있지.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS008;
COS010:
                set @pos, dialog(0, 1, 1, "다만 순수한 피여야만 하기 때문에 쉽지가 않을걸세. 포기하지 말고 나에게 악어의피 3병을 구해와주게.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS009;

                set @pos, dialog(0, 1, 1, "그럼 행운을 비네. 너무 늦장을 부리면 다시 깨끗해지기 힘들수도있으니 서둘러주게.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS010;
            }
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 1, "다시 한번 생각해보게. 머지않아 이 압록강이 푸른빛을 잃게 될지도 모르네. 정말 큰일이군.");
            end;
        }
    }
    if($repairascetic == 1)
    {
        if(countitem(name2itemid("악어의피")) < 3)
        {
COS011:
            set @pos, dialog(0, 0, 1, "아직 악어의피를 다 구하지 못했군. 쉽진 않을걸세. 요즘 변종된 악어가많아 순수한 피를 찾기 힘들게야.");
            if(@pos == 0) end;
COS012:
            set @pos, dialog(0, 1, 1, "조금만 더 힘을 내주게");
            if(@pos == -1) goto COS011;
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "오. 정말 다 구해왔군 그래. 이걸로 압록강을 정화시킬수 있겠어. 아차 이건 내가쓰던 머리띠인데 유용하게 쓰게나.");
            if(@pos == 0) end;

            set $repairascetic, 2;
            for(set @loop, 0; @loop < 3; set @loop, @loop+1)
            {
                delitem @sd, name2itemid("악어의피"), 1;
            }
            message @sd, 3, "악어의피 주었습니다";
            additem @sd, name2itemid("수선도사의머리띠"), 1;
            addlegend @sd, 25, 6, 0, 8, "수선도사의 부탁을 들어주었다.";
            end;
        }
    }
    if($repairascetic == 2)
    {
        set @pos, dialog(0, 0, 1, "저번엔 고마웠네. 덕분에 정화된 깨끗한 압록강을 다시볼수있어 기쁘네.");
        end;
    }
}