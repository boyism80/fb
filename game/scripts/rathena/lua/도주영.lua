{
    set @sd, mysd();
    set @id, myid();

    if($dojaeyoung < 5)
    {
        set @pos, dialog(0, 0, 0, "더 예뻐질 방법이 없을까?");
        end;
    }
    if($dojaeyoung == 5)
    {
COS001:
        set @pos, dialog(0, 0, 1, "앗! " + get_name(@sd) + "님 이시군요! 저희 아버지께서 저에게 인어의거울을 주셨는데, 잘 받았습니다.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "이곳 도삭산은 외로운 곳이랍니다. 사람들도 그리 많이 오가지 않고.. 그래서 저는 400층까지의 길을 만리향 꽃으로 가득찬 길로 만들겠다는 생각을 하게 되었죠.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "하지만 얼마전에 정성들여 모으던 만리향 씨앗들을 해파리 괴물들에게 빼앗기고 말았어요.\\n\\n그래서 말인데 제 부탁을 좀 들어주세요. 해파리 괴물들에게 제 만리향 씨앗을 빼앗아 400층까지 가는길 곳곳에 심어 주시면 됩니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, dialog(0, 1, 1, "걸어다니시다 보면 씨앗을 심기에 좋은 장소가 보이실꺼에요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;
COS005:
        set @pos, list2(@list, 0, 0, "제 부탁 들어주실거죠??", "그런 부탁이라면 기꺼이~!", "죄송하지만 저에겐 그럴만한 시간이 없습니다.");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set $dojaeyoung, 6;
            spellaether @sd, name2spellid("만리향씨앗"), 180;
            addlegend @sd, 11, 7, 0, 1, "도주영의 부탁을 들어주자!";
            set @pos, dialog(0, 0, 1, "씨앗은 해파리 괴물들을 처치하면 얻을 수 있을거에요. 부디 제 부탁을 들어주시길. 그럼 몸조심하세요....");
            end;
        }
        else end;
    }
    if($dojaeyoung == 6) 
    {
        if($ssiat_count >= 1 && $ssiat_count <= 9)
        {
            set @pos, dialog(0, 0, 1, "정말 감사해요. 덕분에 도삭산이 많이 보기 좋아졌어요.");
            end;
        }
        if($ssiat_count >= 10 && $ssiat_count <= 49)
        {
            if($ssiat_item_1 == 0)
            {
                set $ssiat_item_1, 1;
                additem @sd, name2itemid("동인각"), 1;
                addlegend @sd, 11, 7, 0, 1, "도삭산 300층 퀘스트 완료";
                set @pos, dialog(0, 0, 1, "흑..정말 감사드려요. 대신 제가 아끼는 물건을 하나 드리죠. 자..");
            }
        }
        if($ssiat_count == 50 && $ssiat_count <= 99)
        {
            if($ssiat_item_2 == 0)
            {
                set $ssiat_item_2, 1;
                additem @sd, name2itemid("청옥의목걸이"), 1;
                addlegend @sd, 11, 7, 0, 1, "도삭산 300층 퀘스트 완료";
                set @pos, dialog(0, 0, 1, "흑..정말 감사드려요. 대신 제가 아끼는 물건을 하나 드리죠. 자..");
            }
        }
        if($ssiat_count == 100 && $ssiat_count <= 199)
        {
            if($ssiat_item_3 == 0)
            {
                set $ssiat_item_3, 1;
                additem @sd, name2itemid("은인각"), 1;
                addlegend @sd, 11, 7, 0, 1, "도삭산 300층 퀘스트 완료";
                set @pos, dialog(0, 0, 1, "흑..정말 감사드려요. 대신 제가 아끼는 물건을 하나 드리죠. 자..");
            }
        }
        if($ssiat_count == 200 && $ssiat_count <= 999)
        {
            if($ssiat_item_4 == 0)
            {
                set $ssiat_item_4, 1;
                additem @sd, name2itemid("봉마의귀걸이"), 1;
                addlegend @sd, 11, 7, 0, 1, "도삭산 300층 퀘스트 완료";
                set @pos, dialog(0, 0, 1, "흑..정말 감사드려요. 대신 제가 아끼는 물건을 하나 드리죠. 자..");
            }
        }
        if($ssiat_count == 1000)
        {
            if($ssiat_item_5 == 0)
            {
                set $ssiat_item_5, 1;
                set $dojaeyoung, 7;
                additem @sd, name2itemid("적막의목걸이"), 1;
                addlegend @sd, 11, 7, 0, 1, "도삭산 300층 퀘스트 완료";
                set @pos, dialog(0, 0, 1, "흑..정말 감사드려요. 대신 제가 아끼는 물건을 하나 드리죠. 자..");
            }
        }
        set @pos, dialog(0, 0, 0, "아직 많이 심지 못하셨군요..");
        end;
    }
    if($dojaeyoung == 7)
    {
        set @pos, dialog(0, 0, 1, "저번엔 정말 감사했어요. 덕분에 도삭산이 많이 보기가 좋아졌어요.");
        end;
    }
}