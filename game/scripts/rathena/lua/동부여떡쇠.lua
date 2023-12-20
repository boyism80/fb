{//염인백화검
    image 0, 7, 3;
    set @sd, mysd();
    set @id, myid();

    switch(get_job(@sd))
    {
    case 1:
        set @menu$, "현철중검 업그레이드";
        break;
    case 2:
        set @menu$, "야월도 업그레이드";
        break;
    case 3:
        set @menu$, "영혼마령봉 업그레이드";
        break;
    case 4:
        set @menu$, "해골죽장 업그레이드";
        break;
    }

    if(get_level(@sd) <= 50) { set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "물건 고치기", "모두 고치기", "무기 별칭 명명", "아이템 파괴", "염인백화검 만들기", "뢰진도 만들기"); }
    else { set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "물건 고치기", "모두 고치기", "무기 별칭 명명", "아이템 파괴", "염인백화검 만들기", "뢰진도만들기", @menu$); }
    if(@ret == 1)
    {
        set @ret, list1("무엇을 사시게요?", "기타");
        if(@ret == 1) { callshop 0, 22; }
    }
    else if(@ret == 2) { callshop 1, 23; } //팔수있는 번호
    else if(@ret == 3) { callrepair(); }
    else if(@ret == 4) { callrepairall(); }
    else if(@ret == 5) { callnaming(); }
    else if(@ret == 6) { callfunc "DESTROY_ITEM"; }
    else if(@ret == 7)
    {
COS0001:
        set @pos, dialog(0, 0, 1, "거기..자네 혹시 인형술사가 가지고 있는 백화검에 대하여 들어보았는가?");
        if(@pos == 0) end;
COS0002:
        set @pos, dialog(0, 1, 1, "자네도 써봤다면 알겠지만 불길이 그다지 뜨겁질 않아 좋지 못한 무기지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS00001;
COS0003:
        set @pos, dialog(0, 1, 1, "고심 끝에 백화검의 불을 검날로 승화시키는 기술을 개발하였다네. 그게바로 염인백화검이지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS0002;
COS0004:
        set @pos, list2(@quest, 0, 1, "염인백화검을 만들고 싶은가? 염인백화검을 만들기 위해선 백화검 3개가 필요하네", "예, 만들어 보고 싶습니다.", "아니오, 그딴거 필요없습니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS0003;
        else if(@quest == 1)
        {
            if(countitem(name2itemid("백화검")) < 3)
            {
                set @pos, dialog(0, 0, 0, "인형술사에게서 백화검을 구할수 있다네 백화검 3개가 필요해.");
                end;
            }
            else
            {
                if(rand(1, 10) <= 6)
                {
                    for(set @loop, 0; @loop < 3; set @loop, @loop+1)
                    {
                        delitem @sd, name2itemid("백화검"), 1;
                    }    
                    additem @sd, name2itemid("염인백화검"), 1;
                    set @pos, dialog(0, 0, 1, "자. 여기 염인백화검있네. 가저 가게나..");
                    end;
                }
                else
                {
                    delitem @sd, name2itemid("백화검"), 1;
                    set @pos, dialog(0, 0, 0, "이런.. 만드는데 실패하였군..");
                    end;
                }
            }
        }
        else
        {
            set @pos, dialog(0, 0, 0, "마음이 변한다면 다시오게나..");
            end;
        }
    }
    else if(@ret == 8)
    {
        if(countitem(name2itemid("벼락나뭇가지")) == 0)
        {
            set @pos, dialog(0, 0, 0, "뢰진도를 만들기 위한 재료가 없는데?");
            end;
        }
        else
        {
COS001:
            set @pos, dialog(0, 0, 1, "오옷, 이것은 벼락을 맞은 나뭇가지가 아닌가?\\n\\n이리 줘보게, 내 한번 깎아서 무기를 만들어볼테니..");
            if(@pos == 0) end;
COS002:
            set @pos, dialog(0, 1, 1, "싫다면 지금이라도 그만두게...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS001;
COS003:
            set @pos, dialog(0, 1, 1, "...\\n\\n......\\n\\n.........");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
COS004:
            delitem @sd, name2itemid("벼락나뭇가지"), 1;
            message @sd, 3, "벼락나뭇가지 주었습니다.";
            if(rand(1, 10) <= 9)
            {
                
                additem @sd, name2itemid("뢰진도"), 1;
                set @pos, dialog(0, 0, 1, "아주 훌륭한 무기가 완성되었다네! 한번 휘둘러 보게나!");
                end;
            }
            else
            {
                additem @sd, name2itemid("천둥부적"), 1;
                set @pos, dialog(0, 0, 1, "아쉽게 실패했군. 하지만 쓸만한 보조류를 만들었으니 요긴하게 사용하게나.");
                end;
            }
        }
    }
    else if(@ret == 9)
    {
        callfunc "WEAPON_UPGRADE";
    }
    else end;
}