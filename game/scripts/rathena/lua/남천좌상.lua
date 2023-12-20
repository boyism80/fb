{
    set @sd, mysd();
    set @id, myid();
    set @count, 500;

    if($hoo_kill == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "요즘 백성들이 후를 두려워해서 집밖으로 다니는것 조차 두려워하고 있다네. 일정한 곳에 머물지 않고 땅을 파고 어디든지 돌아다니기 때문에 더욱 두려운 존재야.");
        if(@pos == 0) end;

        set @pos, dialog(0, 1, 1, "이놈들을 퇴치하지 않으면 머지않아 우리나라가 쑥밭이 될 지경이니 이거 정말 큰일이네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set @pos, list2(@list, 0, 0, "자네가 후를 퇴치하여 주지 않겠나? 제발 부탁하네.", "그러죠. 제가 처리하겠습니다!!", "저보다 나은 적임자가 분명 있을겁니다.");
        if(@pos == 1 && @list == 1)
        {
COS002:
            set @pos, dialog(0, 0, 1, "고맙네. 후는 번식력도 강해서 한번에 " + @count + "마리 이상 잡지 않으면 금방 다시 번식을 한다네. 후는 여러종류가 있지만, 어떤 종류든 후를 잡으면 뼈를 남긴다네.");
            if(@pos == 0) end;

            set @pos, dialog(0, 1, 1, "자네가 후를 " + @count + "마리 이상 잡았다는 증거로, " + @count + "마리의 뼈를 가지고 오면 내 포상을 하지. '후의뼈' " + @count + "개라네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;

            set $hoo_kill, 1;
            addlegend @sd, 38, 7, 0, 1, "후 말살 퀘스트를 받다.";
            end;
        }
    }
    if($hoo_kill == 1)
    {
        if(countitem(name2itemid("후의뼈")) < @count)
        {
            set @pos, dialog(0, 0, 0, "후의뼈가 너무 적은것은 아닌가? " + @count + "마리 이상을 잡고 후의뼈 " + @count + "개를 가져와주게.");
            end;
        }
        else
        {
COS003:
            set @pos, dialog(0, 0, 1, @count + "마리를 잡는데 성공했군! 하지만 아직 일이 끝난게 아닐세. 미리 얘기를 하지 않았지만 후라는 녀석은 마계천신이라는 왕이 있어서 그 왕이 사라지지 않는 이상 시간이 지나면 더욱 강하게 되살아나지.");
            if(@pos == 0) end;

            set @pos, dialog(0, 1, 1, "그래서 한가지 부탁을 더 해야겠네. 마계천신을 잡아 그의 뼈를 가져와주게. 정말 마지막 부탁이네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS003;

            set @pos, list2(@list, 0, 0, "마계천신의뼈를 가져와줄 수 있겠나?", "물론이죠! 다시는 되살아나지 못하도록 해야합니다.", "그런것까지 제가 할 필요는 없는것 같군요.");
            if(@pos == 1 && @list == 1)
            {
                set $hoo_kill, 2;

                delitem @sd, name2itemid("후의뼈"), @count;
                message @sd, 3, "후의뼈 주었습니다";

                addlegend @sd, 38, 7, 0, 1, "마계천신의뼈를 구하자.";
COS004:
                set @pos, dialog(0, 0, 1, "오, 고맙네. 그럼 자네가 가지고온 후의뼈는 가져가도록 하겠네.");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "그럼 마저 부탁하지. 행운을 비네.");
                if(@pos == -1) goto COS004;
                end;
            }
        }
    }
    if($hoo_kill == 2)
    {
        if(countitem(name2itemid("마계천신의뼈")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 마계천신의뼈를 구하지 못한 모양이로군..");
            end;
        }
        else
        {
COS005:
            set @pos, dialog(0, 0, 1, "오오, 정말로 마계천신의뼈를 가지고 왔군. 자넨 정말 대단한 인물이군..");
            if(@pos == 0) end;
COS006:
            set @pos, dialog(0, 1, 1, "정말 진심으로 고맙네. 자넨 정말 큰일을 해냈어! 허허...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS005;
COS007:
            set @pos, dialog(0, 1, 1, "이건 고마움의 표시니 가져가도록 하게. 고구려에 있다는 숨겨진 마을에서 만들었다는 '세시투구'를 흉내내어 만들어 본 것이라네. 물론 세시투구만큼의 성능이 나오진 않지만 말이야.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS006;

            set @pos, dialog(0, 1, 1, "이 투구를 쓰고 있으면 자네의 무기와 갑옷의 내구력이 오랫동안 유지될걸세. 그럼 잘 가시게..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS007;

            set $hoo_kill, 3;

            delitem @sd, name2itemid("마계천신의뼈"), 1;
            message @sd, 3, "마계천신의뼈 주었습니다";
            additem @sd, name2itemid("강철투구"), 1;
            addlegend @sd, 38, 6, 0, 1, "후 말살 임무를 완수하였다.";
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, get_name(@sd) + "씨 아니오? 저번엔 정말 감사했소. 덕분에 백성들의 고민을 한결 덜어주었다네.");
        end;
    }
}