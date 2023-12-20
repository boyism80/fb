{
    set @sd, mysd();
    set @id, myid();

    if($myungsukboo == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "혹시 자네도 이 이글거리는 기운이 보이는가? 집 전체에 귀기가 서려서 잠을 잘 수가 없군. 집값이 싸서 샀더니만 이런 귀신 붙은 집일 줄 몰랐어.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "지나가던 스님께 여쭤 봤더니 부적을 방 구석구석 바닥에 묻으면 귀신을 퇴치할 수 있다고 하더군.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, list2(@list, 0, 1, "그래서 조금 전까지는 부적을 묻고 있는 중이였는데, 나 혼자서는 안되겠더구만. 보다시피 나 혼자 살기에는 집이 좀 넓어서 말이야.", "제가 좀 도와 드리죠.", "열심히 붙이세요. 전 나가봐야겠네요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
        else if(@list == 1)
        {
COS004:
            set @pos, dialog(0, 0, 1, "그래 주겠는가? 고맙네.");
            if(@pos == 0) end;
COS005:
            set @pos, dialog(0, 1, 1, "이 부적을 가지고 집 구석구석을 살피다가... 허술해 보이는 부분이 있으면 바닥에 묻어주게나. 잘 부탁하네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS004;

            set $myungsukboo, 1;
            set $myungsukboo_count, 0;

            spellaether @sd, name2spellid("귀신퇴치부적"), 180;
            additem @sd, name2itemid("귀신퇴치부적"), 5;
            addlegend @sd, 29, 7, 0, 6, "안도의 부탁을 들어주자. [1/2]";
            end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "아이고. 이 많은 부적을 언제 다 묻는담...");
            end;
        }
        else end;
    }
    if($myungsukboo == 1)
    {
        if($myungsukboo_count < 5)
        {
            set @pos, dialog(0, 0, 0, "아직 부적을 다 묻지 못한 것 같군.");
            end;
        }
        else
        {
COS006:
            set @pos, dialog(0, 0, 1, "부적이 다 떨어졌다고? 큰일이군! 부적이 더 있어야겠는데...");
            if(@pos == 0) end;
COS007:
            set @pos, dialog(0, 1, 1, "그런데 이 부적은 예전에 중국에 갔을 때 귀신 쫓는 데에 능하다는 고승에게 받은 것이라 더 받으려면 중국까지 가야 할 판국이야. 이거 야단났군...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS006;
COS008:
            set @pos, list2(@list_2, 0, 1, "이를 어쩐다...", "제가 그 고승을 만나고 오겠습니다.", "아무래도 너무 힘들어서 안되겠네요. 전 이만...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS007;
            else if(@list_2 == 1)
            {
COS009:
                set @pos, dialog(0, 0, 1, "정말 고맙네! 자넨 혹시 보살인가? 정말 친절하구만. 일이 끝나고 나면 내 꼭 사례하겠네.");
                if(@pos == 0) end;
COS010:
                set @pos, dialog(0, 1, 1, "그렇지. 그 스님을 찾는다면 이름이라도 가르쳐 줘야겠군. 뭐라고 했더라? ...아, 그렇지.\\n\\n중국에 있다는 그 시님의 법명은 '마타'라고 했다네. 미안하지만 부적을 얻어 와 주게나. 부탁하네.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS009;

                set $myungsukboo, 2;
                end;
            }
            else if(@list_2 == 2)
            {
                set @pos, dialog(0, 0, 0, "이를 어쩐다...");
                end;
            }
            else end;
        }
    }
    if($myungsukboo == 2)
    {
        if($myungsukboo_check == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 마타님을 만나지 못한 것 같군");
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "정말 수고했네. 그 먼 길을 이리도 빨리 다녀왔는가? 대단하구만! 자, 이러고 있을 게 아니라 어서 부적을 붙여 주게나.");
            if(@pos == 0) end;

            set $myungsukboo, 3;
            set $myungsukboo_count, 0;

            addlegend @sd, 29, 7, 0, 6, "안도의 부탁을 들어주자. [2/2]";

            spellaether @sd, name2spellid("귀신퇴치부적"), 180;
            end;
        }
    }
    if($myungsukboo == 3)
    {
        if($myungsukboo_count < 5)
        {
            set @pos, dialog(0, 0, 0, "아직 부적을 모두 묻지 못한 모양이군?");
            end;
        }
        else
        {
COS011:
            set @pos, dialog(0, 0, 1, "다 끝났군, 이젠 좀 살만하구만!");
            if(@pos == 0) end;
COS012:
            set @pos, dialog(0, 1, 1, "다 자네 덕분일세, 약속대로 사례를 해야겠는데... 뭐가 좋을까... 그렇지. 이걸 가져가게.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS011;
COS013:
            set @pos, dialog(0, 1, 1, "이건 내가 예전에 귀신을 쫓아 보려고 샀던 부적이야. 비슷하게 생겨서 효과가 있으려니 하고 샀더니만 귀신 쫓는 데에는 효과가 없고 그저 머리가 좀 맑아지더구만.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS012;
COS014:
            set @pos, dialog(0, 1, 1, "나처럼 머리 나쁜 사람한테는 별 쓸모가 없겠지만 자네한테는 쓸모가 있을 것 같으니 가져가주게나.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS013;

            set $myungsukboo, 4;

            additem @sd, name2itemid("명석부"), 1;
            addlegend @sd, 29, 7, 0, 6, "안도의 부탁을 들어주었다.";
            end;    
        }
    }
    if($myungsukboo == 4)
    {
        set @pos, dialog(0, 0, 1, "자네 덕분에 집에 귀기가 많이 사라졌어. 고맙네.");
        end;
    }
}