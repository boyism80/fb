{
    set @sd, mysd();
    set @id, myid();

    if($handmagic <= 0 || $hangmagic >= 3)
    {
        set @pos, dialog(0, 0, 1, "나무아비타불..");
        end;
    }
    if($handmagic == 1)
    {
COS001:
        set @pos, dialog(0, 0, 1, "안녕하십니까. 이런 곳에서 이국의 행인을 만나게 되다니, 이것도 다 인연이겠지요.");
        if(@pos == 0) end;
COS002:
        set @pos, list2(@list, 0, 0, "저에게 무슨 볼일이라도 있으신건지요?", "소림사의 절기를 전수해주십시오.", "스님을 보니 반가워서 그만..");
        if(@pos == 0) end;
        else if(@list == 1)
        {
COS003:
            set @pos, dialog(0, 0, 1, "제가 소림사에서 온건 어떻게 아셨습니까. 저희 소림사의 절기들은 원래 경내에서만 수련하는 것으로, 전수받기 위해서는 입문을 하셔야 합니다.");
            if(@pos == 0) end;
COS004:
            set @pos, dialog(0, 1, 1, "그렇지만, 입문을 하기 위해선 소림사까지 가야하고, 저는 지금 고구려에 비단을 사러 가는 길이라 다시 돌아가려면 몇달은 걸릴 것입니다.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS003;
COS005:
            set @pos, dialog(0, 1, 1, "아.. 고구려의 무도가에게서 그런 부탁을 받으셨습니까. 하지만 말씀드린대로 절기의 전수는 불도에 입문한 분에게만 가능합니다.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS004;
COS006:
            set @pos, list2(@list_2, 0, 0, "불도에 입문하시겠습니까?", "네, 입문하겠습니다.", "아니오, 그건좀...");
            if(@pos == 0) end;
            else if(@list_2 == 1)
            {
COS007:
                set @pos, dialog(0, 0, 1, "진심이십니까? 그럼 제가 고구려에 다녀올때까지 기다려주시겠죠? 그 이후로 소림사로 돌아가서 몇년간 속세와 인연을 끊고 수련을 계속 하십시다.");
                if(@pos == 0) end;
COS008:
                set @pos, dialog(0, 1, 1, "네? 그런건지는 몰랐다구요? 이런이런.. 그런 마음으론 불도에 입문하시기 힘드시겠습니다.");
                if(@pos == -1) goto COS007;
                end;
            }
            else if(@list_2 == 2)
            {
COS009:
                set @pos, dialog(0, 0, 1, "쉽게 결정할 수 있는 문제가 아니지요.");
                if(@pos == 0) end;
COS010:
                set @pos, dialog(0, 1, 1, "아, 저에게 좋은 생각이 났는데, 고구려분이시니, 고구려 지리에 익숙하시겠죠? 국내성포목상에서 비단을 한필 사다주시면.. 제가 간단한 기술이나마 알려드릴 수 있을 것 같은데요..");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS009;
COS011:
                set @pos, list2(@list_3, 0, 0, "비단을 구해주실 수 있으시겠습니까?", "네, 비단을 구해드리겠습니다.", "지금은 고구려로 돌아가기가 좀 곤란해서...");
                if(@pos == 0) end;
                else if(@list_3 == 1)
                {
                    set $handmagic, 2;
                    addlegend @sd, 26, 7, 0, 1, "소림사승려에게 비단을 가져다주자.";
                    set @pos, dialog(0, 0, 1, "네 감사합니다. 이정도는 주지스님께서도 이해해 주시겠지요..\\n\\n그럼 무사히 다녀오시길.. 나무아미타불..");
                    end;
                }
                else if(@list_3 == 2)
                {
                    set @pos, dialog(0, 0, 1, "그러시군요.. 그거 아쉽게 됬네요.");
                    end;
                }
                else end;
            }
            else end;
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 1, "그런가요?");
            end;
        }
        else end;
    }
    if($handmagic == 2)
    {
        if(countitem(name2itemid("고구려비단")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 고구려비단을 못 사오신것 같군요.");
            end;
        }
        else
        {
COS012:
            set @pos, dialog(0, 0, 1, "비단을 가져오셨군요. 감사합니다.\\n\\n그럼 기술을 가르쳐드리겠습니다.");
            if(@pos == 0) end;
COS013:
            set @pos, dialog(0, 1, 1, "이것은 백열장이라고 하는 수도의 발전형 기술로, 강력한 파괴력을 가지고 있습니다.\\n\\n다만 지금은 기술을 알았을뿐, 수련을 하지 못하셨으니 실전에서 사용하시긴 힘들겁니다.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS012;
COS014:
            set @pos, dialog(0, 1, 1, "그럼 또 인연이 닿으면 만나겠지요. 살펴가십시오. 나무아미타불..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS013;

            set $handmagic, 3;
            addlegend @sd, 26, 7, 0, 1, "소림사의 비기를 배웠다.";
            delitem @sd, name2itemid("고구려비단"), 1;
            message @sd, 3, "고구려비단 주었습니다";
            if(checkspell(name2spellid("백열장")) == 52)
            {
                addspell name2spellid("백열장");
            }
            end;
        }
    }
}