{
    set @sd, mysd();
    set @id, myid();

    if($mountain_god_clothes == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "안녕하세요~ 도삭산 800층까지 오시다니.. 수고 많으셨겠습니다.\\n\\n800층 까지도 물론 힘들게 오셨겠지만, 801층 부터는 완전히 다른 세계가 펼쳐진답니다.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "가보시면 아시리라 생각합니다만, 원래부터 도삭산에 살고 있던 '산신'이라는 인간도 동물도 아닌 생물이 살고 있지요.\\n\\n신선수행을 하던 인간이 산신이 되었다는 얘기가 있는데, 사실인지는 알 수 없지요..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "이들 산신은 인간이었을때 가지고 있던 소지품을 어떤곳에 묻어서 버린다고 하는데, 신선수행을 하던 사람들이었으니 귀중품들이 많이 있을겁니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, dialog(0, 1, 1, "도삭산 801층 이후에는 책장들이 있는 곳이 있는데, 그중 어딘가 한군데에서 제가 보물지도를 본적이 있습니다. 아마 산신들의 귀중품을 숨겨놓은 곳을 표시해둔 것 같은데, 그때 도망치느라 가져오질 못했습니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;
COS005:
        set @pos, list2(@list, 0, 0, "한번 구하러 가보시지요?", "네, 찾아가봐야 겠군요.", "갈길이 바빠서..");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $mountain_god_clothes, 1;
            spellaether @sd, name2spellid("산신의보물지도"), 180;
            addlegend @sd, 41, 7, 0, 25, "산신의보물지도를 찾자";
            set @pos, dialog(0, 0, 1, "그럼 보물을 찾으시면 저에게도 꼭 보여주셔야 됩니다!");
            end;
        }
        else end;
    }
    if($mountain_god_clothes == 1)
    {
        if(countitem(name2itemid("산신의비단")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 보물을 찾기 못하신 것 같군요. 801층 이후에는 책장들이 있는 곳이 있는데, 그중 어딘가 한군데에서 제가 보물지도를 본적이 있습니다. 아마 산신들의 귀중품을 숨겨놓은 곳을 표시해둔 것 같은데...");
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "오오! 이것은 산신의비단이 아닙니까! 이걸 저에게 맡기시면 제가 멋진 옷을 만들어드리겠습니다!");
            if(@pos == 0) end;

            set @pos, list2(@list, 0, 0, "저에게 맡기시는 것이 어떠실지?", "네 그럼 부탁합니다.", "안됩니다 어떻게 구한건데.");
            if(@pos == 1 && @list == 1)
            {
COS006:
                set @pos, dialog(0, 0, 1, "크.. 이런 옷감을 내 손으로 만지게 되다니.. 영광입니다 영광..");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "그런데 이 옷감.. 너무 튼튼해서 보통 바늘로는 꿰멜수가 없겠는데요.. 아무래도 산신들이 이 옷감을 다룰때 쓰는 바늘이 있을 것 같은데..\\n\\n산신들이 사용하는 바늘이 있어야 옷을 만들 수 있겠습니다..");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS006;

                delitem @sd, name2itemid("산신의비단"), 1;
                message @sd, 3, "산신의비단 주었습니다";

                set $mountain_god_clothes, 2;
                addlegend @sd, 41, 7, 0, 25, "산신의바늘을 구해 마석주에게 가져다주자.";
                end;
            }
            else end;
            end;
        }
    }
    if($mountain_god_clothes == 2)
    {
        if(countitem(name2itemid("산신의바늘")) == 0)
        {
            set @pos, dialog(0, 0, 0, "산신의바늘을 아직 구하지 못하신거 같은데요?");
            end;
        }
        else
        {
COS007:
            set @pos, dialog(0, 0, 1, "이것은... 바늘이 아니라 새털처럼 보이는데.. 어디 한번..");
            if(@pos == 0) end;
COS008:
            set @pos, dialog(0, 1, 1, "아니! 이것은 마법이 걸린 깃털인가 봅니다. 어째서 이런 튼튼한 옷감이 이런 부드러운 것에 구멍이 나는 것인지...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS007;

            set @pos, dialog(0, 1, 1, "자.. 그럼 어디한번 옷을 만들어볼까요.. 아. 그런데 어떤 옷을 만들어드리지요?");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS008;

            set @pos, list2(@list, 0, 0, "어떤 옷을 만들어드릴까요?", "산신의천수를 만들어주십시오", "산신의정화를 만들어주십시오", "산신의인풍을 만들어주십시오", "산신의비설을 만들어주십시오", "산신의수령을 만들어주십시오", "산신의정령을 만들어주십시오", "산신의인령을 만들어주십시오", "산신의비령을 만들어주십시오");
            if(@pos == 1 && @list > 0)
            {
                setarray @item$[0], "산신의천수", "산신의정화", "산신의인풍", "산신의비설", "산신의수령", "산신의정령", "산신의인령", "산신의비령";
                setarray @information$[0], "는", "는", "은", "은", "은", "은", "은", "은";
                setarray @image[0], 2171, 2167, 2173, 2169, 2172, 2168, 2174, 2170;
                setarray @job$[0], "전사", "도적", "주술사", "도사", "전사", "도적", "주술사", "도사";
                setarray @gender$[0], "남자", "남자", "남자", "남자", "여자", "여자", "여자", "여자", "여자";

                set @val, @list-1;
                set @add$, @item$[@val];
                set @image_2, @image[@val];
                set @jobname$, @job$[@val];
                set @information_2$, @information$[@val];
                set @gender_2$, @gender$[@val];

                deletearray @item$[0], getarraysize(@item$);
                deletearray @image[0], getarraysize(@image);
                deletearray @job$[0], getarraysize(@job$);
                deletearray @information$[0], getarraysize(@information$);
                deletearray @gender$[0], getarraysize(@gender$);
COS009:
                image 1, @image_2, 0;
                set @pos, dialog(0, 0, 1, @add$ + @information_2$ + " " + @gender_2$ + " " + @jobname$ + "들을 위한 옷입니다.");
                if(@pos == 0) end;
COS010:
                image 0, 548, 0;
                set @pos, dialog(0, 1, 1, "자.. 그럼.. ...... ...... ..........");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS009;

                delitem @sd, name2itemid("산신의바늘"), 1;
                message @sd, 3, "산신의바늘 주었습니다";
                additem @sd, name2itemid(@add$), 1;
                set $mountain_god_clothes, 3;
                addlegend @sd, 41, 6, 0, 25, "산신의옷을 만들다!";
                set @pos, dialog(0, 0, 1, "여기 있습니다! 멋진 옷을 만들게 되어 정말 기쁘군요!");
                end;
            }
            else end;
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "제가 만들어 드린 산신의옷은 잘 사용하고 계신가요?");
        end;
    }
}