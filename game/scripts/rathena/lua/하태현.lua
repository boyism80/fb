{
    set @sd, mysd();
    set @id, myid();

    if($hataehyun == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "이곳까지 오시느라 수고하셨네요. 끝까지 올라가실 생각인가요? 음.. 계속 올라가실거라면 부탁하나 드리고 싶은데..");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "조금더 올라가시다보면 도삭산선비족, 도삭산흉노족이 있답니다. 그녀석들은 포악하고 성격은 나쁘지만 여러가지 신기한것들을 가지고 있죠.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "그중에 가면종류가 신기한것이 많죠. 각종 나무가면과 또 철이나 다른 금속으로 만든 가면도 있다더군요.\\n\\n제가 그런 물건에 관심이 많아서 부탁드립니다만 나무가면 50개를 모아다 주실수 없으신가요? 50개면 종류별로 다 모을 수 있을것 같아서..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, list2(@list, 0, 0, "부탁드려요. 워낙 포악해서 저는 근처도 갈 용기가 없네요. 들어주실거죠?", "그런일이라면 맡겨만 주세요.", "가면이라. 전 그런것에 관심이 없어서요.");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $hataehyun, 1;
            addlegend @sd, 15, 7, 0, 1, "하태현의 부탁을 들어주자!";
            set @pos, dialog(0, 0, 1, "그럼 전 " + get_name(@sd) + "님만 기다리고 있을께요. 적어도 50개는 모아다 주셔야해요~ 그래야 종류별로. 부탁드릴께요.");
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "그러시군요.. 안타깝네요..");
            end;
        }
        else end;
    }
    if($hataehyun == 1)
    {
        if(countitem(name2itemid("나무가면")) < 50)
        {
            set @pos, dialog(0, 0, 0, "아직 나무가면이 별로 없네요.. 50개정도만 구해주세요~");
            end;
        }
        else
        {
COS005:
            set @pos, dialog(0, 0, 1, "우와. 역시 정말 멋진 가면들. 정말 감사해요. 제가 가져가도록 하죠.");
            if(@pos == 0) end;
COS006:
            set @pos, dialog(0, 1, 1, "고마움에 표시로 드리는 선물입니다. 투구인데요 님에게 맞는 투구를 고르세요..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS005;
COS007:
            set @pos, list2(@list_2, 0, 0, "어떤 투구를 드릴까요?", "진비투구를 주십시오.", "경비투구를 주십시오.", "법비모를 주십시오.", "격비모를 주십시오.", "어떤 투구들이 있는지 알려주십시오.");
            if(@pos == 0) end;
            else if(@list_2 > 0 && @list_2 <= 4)
            {
                set $hataehyun, 2;
                setarray @item$[1], "진비투구", "경비투구", "법비모", "격비모";
                setarray @jobs$[1], "전사", "도적", "주술사", "도사";
                setarray @image[1], 2103, 2105, 2108, 2227;
                image 1, @image[@list_2], 0;
                delitem @sd, name2itemid("나무가면"), 50;
                message @sd, 3, "나무가면 주었습니다";
                additem @sd, name2itemid(@item$[@list_2]), 1;
                addlegend @sd, 15, 7, 0, 1, "도삭산 700층 퀘스트 완료";
                set @pos, dialog(0, 0, 1, @item$[@list_2] + "는 " + @jobs$[@list_2] + "분들을 위한 투구입니다.");
                deletearray @item$[1], getarraysize(@item$);
                deletearray @jobs$[1], getarraysize(@jobs$);
                deletearray @image[1], getarraysize(@image);
                image 0, 541, 0;
                set @pos, dialog(0, 0, 1, "혹시 아실지 모르겠지만 도삭산선비족,도삭산흉노족의 가면을 이용하면 더 좋은 투구를 만드실 수 있을거에요.\\n\\n저도 자세한 방법은 모르지만요. 하여튼 너무나 감사해요. 다음에 꼭 다시 들러주세요~");
                end;
            }
            else if(@list_2 == 5)
            {
COS008:
                image 1, 2103, 0;
                set @pos, dialog(0, 0, 1, "진비투구는 전사를 위한 투구입니다.");
                if(@pos == 0) end;
COS009:
                image 1, 2105, 0;
                set @pos, dialog(0, 1, 1, "경비투구는 도적을 위한 투구입니다.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS008;
COS010:
                image 1, 2108, 0;
                set @pos, dialog(0, 1, 1, "법비모는 도적을 위한 투구입니다.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS009;
COS011:
                image 1, 2227, 0;
                set @pos, dialog(0, 1, 0, "격비모는 도적을 위한 투구입니다.");
                if(@pos == -1) goto COS010;
                end;
            }
            else end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "저번엔 정말 감사했습니다. 가면이 하나같이 모두 흥미롭군요.");
        end;
    }
}