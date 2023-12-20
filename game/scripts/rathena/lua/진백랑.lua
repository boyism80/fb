{
    set @sd, mysd();
    set @id, myid();

    if($lighthouse_weapon == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "대륙의 기운이 느껴지는군. 자네라면 내 억울함을 풀어줄 수 있을지도 모르지.");
        if(@pos == 0) end;
COS002:
        set @pos, list2(@list, 0, 0, "난 오래전 이 섬에서 죽은 중국의 선장이다. 날 좀 도와주겠나?", "네, 도와드리지요.", "아니오.. 싫습니다.");
        if(@pos == 0) end;
        else if(@list != 1) end;

COS003:
        set @pos, dialog(0, 0, 1, "나는 원래 서복이란 자를 쫓아 이곳에 당도했다. 결국 서복은 없었지만 너무나 아름다운 이 섬에서 얼마간 머물기로 마음먹었지. 그 결과 나는 엄청난 비밀을 알게 되었고, 그 대가로 이렇게 원혼이 되었다.");
        if(@pos == 0) end;
COS004:
        set @pos, dialog(0, 1, 1, "죽었다는 사실은 억울하지 않아. 다만, 내가 이 섬에 대해 기록한 일지가 세상에 알려지지 않은 것이 슬플뿐이야. 이미 섬 이곳저곳으로 흩어져 버렸지만.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;
COS005:
        set @pos, dialog(0, 1, 1, "전부 삼만장 정도 되는데, 아홉장만 모아도 굉장히 충실한 내용이야. 아홉장과 겉표지를 모아서 내게 가져다주면 좋겠군.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS004;

        set $lighthouse_weapon, 1;
        addlegend @sd, 32, 7, 0, 1, "진백랑의 부탁을 들어주자. [1/6]";
        end;
    }
    if($lighthouse_weapon == 1)
    {
        if(countitem(name2itemid("선장의일기1")) == 0 || countitem(name2itemid("선장의일기2")) == 0 ||
        countitem(name2itemid("선장의일기3")) == 0 || countitem(name2itemid("선장의일기4")) == 0 ||
        countitem(name2itemid("선장의일기5")) == 0 || countitem(name2itemid("선장의일기6")) == 0 ||
        countitem(name2itemid("선장의일기7")) == 0 || countitem(name2itemid("선장의일기8")) == 0 ||
        countitem(name2itemid("선장의일기9")) == 0 || countitem(name2itemid("선장의일기겉표지")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 일기 아홉장과 겉표지를 구하지 못했나보군..");
            end;
        }
        else
        {
COS006:
            set @pos, dialog(0, 0, 1, "일기장을 정리해오겠네 좀 기다려주게나");
            if(@pos == 0) end;
COS007:
            set @pos, dialog(0, 1, 1, "수고했네. 여기 일기장이 있어. 이 섬을 여행하는데 있어 좋은 도움이 되었으면 더 바랄 나위가 없을 것 같네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS006;
COS008:
            set @pos, dialog(0, 1, 1, "그리고 이건, 내가 생전에 쓰던 검이네. 등대와 같은 빛을 낸다하여 등대빛의 검이라 불리우지. 괜찮다면 꼭 좋은 일에 써주길 바라네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS007;

            set $lighthouse_weapon, 2;
            setarray @item$[0], "선장의일기1", "선장의일기2", "선장의일기3", "선장의일기4", "선장의일기5", "선장의일기6", "선장의일기7", "선장의일기8", "선장의일기9", "선장의일기겉표지";
            for(set @loop, 0; @loop < 10; set @loop, @loop+1)
            {
                delitem @sd, name2itemid(@item$[@loop]), 1;
            }
            for(set @loop, 0; @loop < 10; set @loop, @loop+1)
            {
                message @sd, 3, @item$[@loop] + " 주었습니다";
            }
            additem @sd, name2itemid("선장의일기장"), 1;
            additem @sd, name2itemid("등대빛의검"), 1;
            addlegend @sd, 32, 6, 0, 1, "등대빛의검을 받다!";
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "......");
        end;
    }
}