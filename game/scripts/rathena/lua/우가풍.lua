{
    set @sd, mysd();
    set @id, myid();

    if($woogapung == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "내 호롱불이..내 호롱불..");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "내 소중한 호롱불을 도삭산문비 녀석이 훔쳐갔다네. 그 호롱불은 어머니가 물려주신 유일한 물건이지, 어느새 그것없이는 잠을 이룰 수가 없게 되었다네.\\n\\n아늑한 호롱불빛이 따뜻한 어머니의 손길처럼 느껴져서...");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "신기한것은 그런 무리가 얼마전까진 없었는데 요즘들어 생겨나서는 말썽을 부린다는 점이지. 원래 도삭산은 살기좋은 곳이었는데.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, list2(@list, 0, 0, "부탁이네. 호롱불을 찾아다 줄 수 없겠나? 나에겐 정말 소중한 물건이라네.", "알겠습니다. 제가 찾아다드리죠.", "호롱불은 얼마든지 많은데...");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $woogapung, 1;
            addlegend @sd, 14, 7, 0, 1, "우가풍의 부탁을 들어주자!";
COS005:
            set @pos, dialog(0, 0, 1, "정말 고맙네. 그 마음만으로도. 문비는 무슈후슈들이 나오는 곳을 지나면 다른 녀석들과 함께 무리로 살고 있지.\\n\\n호롱불을 훔쳐간 녀석은 뭔가 좀 특이했네. 겉보기엔 분간이 잘 안되지만 문비 소굴을 다니다 보면 분명 만날수 있을거야.");
            if(@pos == 0) end;
COS006:
            set @pos, dialog(0, 1, 1, "꼭 그 못된 놈을 잡아서 호롱불을 찾아주게. 휴우.. 자네밖에 믿을 사람이 없네. 그럼..");
            if(@pos == -1) goto COS005;
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그렇지만.. 그게 잘 안된다네..");
            end;
        }
        else end;
    }
    if($woogapung == 1)
    {
        if(countitem(name2itemid("어머니의호롱불")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 찾으시지 못하신것 같군. 문비는 무슈후슈들이 나오는 곳을 지나면 다른 녀석들과 함께 무리로 살고 있지. 호롱불을 훔쳐간 녀석은 뭔가 좀 특이했네. 겉보기엔 분간이 잘 안되지만 문비 소굴을 다니다 보면 분명 만날수 있을거야.");
            end;
        }
        else
        {
            set $woogapung, 2;
            delitem @sd, name2itemid("어머니의호롱불"), 1;
            message @sd, 3, "어머니의호롱불 주었습니다";
            additem @sd, name2itemid("수정해골"), 1;
            addlegend @sd, 14, 7, 0, 1, "도삭산 600층 퀘스트 완료";
COS007:
            set @pos, dialog(0, 0, 1, "이..이것은... 맞네! 어머니의 호롱불이 맞아! 정말 어떻게 감사의 말을 해야할지. 감사히 받겠소.\\n\\n대단하구려. 그녀석을 잡다니. 잠깐잠깐 이럴때가 아니지. 내 꼭 주고 싶은것이 있었는데.");
            if(@pos == 0) end;
COS008:
            set @pos, dialog(0, 1, 1, "얼마전에 도삭산 여행을 다니다가 주운것인데 상당히 희귀한것이라네. 도삭산의 비밀에 관계된 것이 아닐까 하는데..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS007;
COS009:
            set @pos, dialog(0, 1, 1, "자네덕분에 오늘부터 잠을 편안히 잘 수 있겠군. 고마워~");
            if(@pos == -1) goto COS008;
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, get_name(@sd) + "씨 아닌가? 저번엔 정말 고마웠네. 덕분에 잠을 편안히 잘 수 있게 되었다네.");
        end;
    }
}