{
    set @sd, mysd();
    set @id, myid();

    if($dragon_king == 10 && countitem(name2itemid("내통문서")) > 0)
    {
COS001:
        set @pos, list2(@list, 0, 0, "무슨 일인가?", "혹시 이 문서에 적힌", "내용을 알아볼 수 있는지요?");
        if(@pos == 0) end;
        else if(@list > 0 && @list <= 2)
        {
COS002:
            set @pos, dialog(0, 0, 1, "글쎄 한 번 보도록 하지\\n\\n...음......으음......");
            if(@pos == 0) end;
COS003:
            set @pos, dialog(0, 1, 1, "내가 배를 타고 여기 저기를 많이 돌아봐서 아네만 이건 서방 언어일세.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
COS004:
            set @pos, list2(@list_2, 0, 0, "우리 동방 언어와는 근본적으로 틀리지.", "그럼 이것을 알아보실 수 있는지요?");
            if(@pos == 0) end;
            else if(@list_2 != 1) end;
COS005:
            set @pos, list2(@list_3, 0, 0, "하하..물론 가능하지...헌데...", "......");
            if(@pos == 0) end;
            else if(@list_3 != 1) end;
COS006:
            set @pos, dialog(0, 0, 1, "허허..자네 내가 이것을 번역해 주는 대신 무엇인가 요구를 할 것 같아 잔뜩 긴장해 있구만,\\n하긴 무리도 아니지.. 요즘 세상에 공으로 되는 일이 거의 없으니 말일세.");
            if(@pos == 0) end;
COS007:
            set @pos, dialog(0, 1, 1, "하지만 난 틀려, 나는 한평생을 바다와 함께 살아온 바다 사나이라네.\\n나에게는 먹을 음식과 입을 옷만 있으면 그 이상은 필요 없다네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS006;
COS008:
            set @pos, dialog(0, 1, 1, "이건 내가 조건 없이 번역을 해 주겠네. 그 문서를 이리 주게.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS007;
COS009:
            set @pos, list2(@list_4, 0, 0, " ", "정말 고맙습니다.", "일본에 자주 왔다갔다 하는데,", "항상 말이 없어 매우 무뚝뚝하고", "무서우신 분인 줄 알았는데", "참 친절하신 분이시군요.");
            if(@pos == 0) end;
            else if(@list_4 > 0 && @list_4 <= 5)
            {
                delitem @sd, name2itemid("내통문서"), 1;
                message @sd, 3, "내통문서 주었습니다";
                additem @sd, name2itemid("번역된내통문서"), 1;
                addlegend @sd, 19, 7, 0, 1, "번역된내통문서를 얻다.";
                set @pos, dialog(0, 0, 0, "하하.. 앞으로 자주 인사하세나.");
                end;
            }
            else end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 0, "안녕하신가? 여행이란 참 즐거운 일이지.");
        end;
    }
}