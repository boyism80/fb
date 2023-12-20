{
    set @sd, mysd();
    set @id, myid();
    if(get_level(@sd) < 56)
    {
        set @pos, dialog(0, 0, 1, "자네는 무기를 업그레이드 할 만큼 숙련되지 못했군.");
        end;
    }
    switch(get_job(@sd))
    {
    case 1:
        set @message$, "현철중검 업그레이드";
        break;
    case 2:
        set @message$, "야월도 업그레이드";
        break;
    case 3:
        set @message$, "영혼마령봉 업그레이드";
        break;
    case 4:
        set @message$, "해골죽장 업그레이드";
        break;
    }
    set @pos, list2(@list, 0, 0, "안녕하세요. 어떻게 오셨나요?", @message$);
    if(@pos == 0) end;
    else if(@list == 1)
    {
        if(get_job(@sd) == 1) goto POS001;
        else if(get_job(@sd) == 2) goto TOS001;
        else if(get_job(@sd) == 3) goto DOS001;
        else if(get_job(@sd) == 4) goto COS001;
    POS001:
        if(countitem(name2itemid("현철중검")) == 0)
        {
        POS002:
            set @pos, dialog(0,0,1, "자네.. 전사로군? 전사의 무기라면 역시 현철중검이지! 현철중검이라는 이름은 '검은색의 철로 만든 무거운 검' 이라는 뜻이라네. 커다란 검신에서 나오는 파괴력으로 무엇이든 부숴버리는 강력한 검이지.");
            if(@pos == 0) end;
        POS003:
            set @pos, dialog(0,1,1, "내가 수년간 연구하여 현철중검을 더 강력한 무기로 만드는 방법을 알아냈지. 후후후..");
            if(@pos == 0) end;
            else if(@pos == -1) goto POS002;
        POS004:
            set @pos, dialog(0,1,1, "뭐.. 자네는 현철중검을 쓰지 않고 있으니 관계없는 얘기겠군.");
            if(@pos == -1) goto POS003;
            else end;
        }
        else
        {
        POS005:
            set @pos, dialog(0,0,1, "현철중검을 더 강력하게 하는 법을 알고 있나?");
            if(@pos == 0) end;
        POS006:
            set @pos, dialog(0,1,1, "현철중검은 그 무게와 크기로 무엇이든 부숴버리는 검이지. 다만 현철중검은 날카롭지 않기 때문에 베는 맛이 없어.");
            if(@pos == 0) end;
            else if(@pos == -1) goto POS005;
        POS007:
            set @pos, dialog(0,1,1, "그 이유는 현철은 매우 단단한 물질이라 날카롭게 만들수가 없었다는건데, 의외로 그 해결책이 가까운데 있었다네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto POS006;
        POS008:
            set @pos, dialog(0,1,1, "그건 바로 도적들이 사용하는 날카로운 무기인 '야월도'를 이용하여 현철중검을 깎아서 날카롭게 만드는 것이라네!");
            if(@pos == 0) end;
            else if(@pos == -1) goto POS009;
        POS009:
            set @pos, dialog(0,1,1, "다만 현철중검을 한번 깎으면 야월도는 바로 망가져 버려서 다시는 쓸 수 없게 된다네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto POS008;
        POS010:
            if(countitem(name2itemid("야월도")) == 0)
            {
                set @pos, dialog(0,0,1, "야월도를 가지고 오면 자네의 현철중검을 더 강하게 바꿔주겠네.");
                end;
            }
            else
            {
            POS011:
                set @pos, dialog(0,0,1, "오, 자네는 야월도를 가지고 있군. 자 그럼 현철중검을 놓고 이걸 야월도로 치는 걸세. 이렇게 말이지. 흠!");
                if(@pos == 0) end;

                delitem @sd, name2itemid("현철중검"), 1; message @sd, 3, "현철중검 주었습니다.";
                delitem @sd, name2itemid("야월도"), 1; message @sd, 3, "야월도 주었습니다.";
                additem @sd, name2itemid("흑철중검"), 1;
                sound @id, 70;
            POS012:
                set @pos, dialog(0,0,1, "자 이렇게 흑철중검이 완성되었다네. 어때? 현철중검보다 더 날카롭고 강력해 보이지 않는가? 하하하.");
                if(@pos == 0) end;
            POS013:
                set @pos, dialog(0,1,1, "그럼 앞으로 이 검과 함께 더욱더 정진하여 강인한 전사가 되게나.");
                if(@pos == 0) end;
                else if(@pos == -1) goto POS012;
            }
        }
        end;
    TOS001:
        if(countitem(name2itemid("야월도")) <= 0)
        {
        TOS002:
            set @pos, dialog(0,0,1, "도적은 역시 비밀스러운 데에 매력이 있지. 달빛에 빛나는 야월도를 가지고 밤길을 달려가는 도적의 모습을 상상만 해도 멋지지 않은가!");
            if(@pos == 0) end;
        TOS003:
            set @pos, dialog(0,1,1, "그런데 바로 그 색깔, 흰색이라는 것이 야월도의 멋이기도 하지만 약점이기도 하지. 숨어서 다녀야 하는데 밝은 색깔의 무기를 가지고 다닌다는게 좀 이상하지 않은가?");
            if(@pos == 0) end;
            else if(@pos == -1) goto TOS002;
        TOS004:
            set @pos, dialog(0,1,1, "내 생각엔 역시 야월도는 검은색이어야 할 것 같단 말이지. 자네는 야월도를 쓰고 있지 않으니 별로 관심은 없을지 모르겠지만 말일세.");
            if(@pos == -1) goto TOS003;
            else end;
        }
        else
        {
        TOS005:
            set @pos, dialog(0,0,1, "도적들이 많이 쓰는 야월도가 흰색이라는 것이 좀 이상하지 않은가? 내 생각엔 역시 야월도는 검은색이어야 할 것 같단 말이지.");
            if(@pos == 0) end;
        TOS006:
            set @pos, dialog(0,1,1, "사실은 내가 야월도를 검은 색의 무기로 바꾸는 법을 생각해 냈는데.. 그건 바로 전사들이 사용하는 검은색의 무기인 '현철중검'을 이용하는 것이지!");
            if(@pos == 0) end;
            else if(@pos == -1) goto TOS005;
        TOS007:
            set @pos, dialog(0,1,1, "야월도와 현철중검을 같이 담금질을 하면 현철중검의 검은철이 야월도로 옮겨오게 되어 흑월도가 된다네. 색깔만 검어지는 것이 아니라, 현철의 단단함까지 가지게 된다네! 그 대신 여기에 사용한 현철중검은 다시 쓸 수 없게 되지만 말이지..");
            if(@pos == 0) end;
            else if(@pos == -1) goto TOS006;
        TOS008:
            if(countitem(name2itemid("야월도")) == 0)
            {
                set @pos, dialog(0,0,1, "현철중검을 가지고 오면 자네의 야월도를 바꿔주겠네.");
                end;
            }
            else
            {
            TOS009:
                set @pos, dialog(0,0,1, "오, 자네는 현철중검을 가지고 있군, 자 그럼 야월도를 현철중검과 함께 이렇게 담금질을 하는거지! 흠!");
                if(@pos == 0) end;

                delitem @sd, name2itemid("야월도"), 1; message @sd, 3, "야월도 주었습니다.";
                delitem @sd, name2itemid("현철중검"), 1; message @sd, 3, "현철중검 주었습니다.";
                additem @sd, name2itemid("흑월도"), 1;
                sound @id, 70;
            TOS010:
                set @pos, dialog(0,0,1, "자 이렇게 흑월도가 완성되었다네. 어때? 빛을 받아도 검은색이라 비밀스럽게 움직일때 더 편리하겠지?");
                if(@pos == 0) end;
            TOS011:
                set @pos, dialog(0,1,1, "그럼 앞으로 이 칼과 함께 더욱더 멋진 도적이 되게나.");
                if(@pos == 0) end;
                else if(@pos == -1) goto TOS010;
            }
        }
        end;
    DOS001:
        if(countitem(name2itemid("영혼마령봉")) == 0)
        {
        DOS002:
            set @pos, dialog(0,0,1, "주술사들이 사용하는 영혼마령봉에는 영혼이 깃들어 있다고 하지. 봉에 깃들어 있는 영혼의 힘으로 신비스러운 마법을 쓸 수 있도록 도와주는게야. 그렇다면 더 강력한 영혼을 깃들게 하면 더 강력한 봉이 되지 않을까?");
            if(@pos == 0) end;
        DOS003:
            set @pos, dialog(0,1,1, "자네는 영혼마령봉을 쓰고 있지 않으니 별로 관심 없을지 모르지만 말일세.");
            if(@pos == -1) goto DOS002;
            end;
        }
        else
        {
        DOS004:
            set @pos, dialog(0,0,1, "영혼마령봉은 깃들어 있는 영혼의 마력을 빌어 신비스러운 마법을 사용하는 것이지. 그럼 여기에 더 강력한 영혼을 깃들게 하면, 마력이 더 강해질 것 같지 않은가?");
            if(@pos == 0) end;
        DOS005:
            set @pos, dialog(0,1,1, "도사들이 사용하는 해골죽장의 원혼을 영혼마령봉에 깃들게 해보았더니 훨씬 더 강한 봉이 만들어지더구만! 해골죽장에 깃든 원혼은 강한 원한을 가지고 있어서 영혼마령봉의 영혼보다 더 큰 마력을 가지기 때문일세.");
            if(@pos == 0) end;
            else if(@pos == -1) goto DOS004;
        DOS006:
            set @pos, dialog(0,1,1, "영혼마령봉과 해골죽장으로 동시에 서로에게 마법을 사용하여 영혼을 바꿀 수 있다네. 그렇지만 영혼마령봉의 영혼은 해골속에 머무르지 않는 것 같더군.");
            if(@pos == 0) end;
            else if(@pos == -1) goto DOS005;
        DOS007:
            if(countitem(name2itemid("해골죽장")) == 0)
            {
                set @pos, dialog(0,0,1, "해골죽장을 가지고 오면 자네의 영혼마령봉을 더 강하게 바꿔주겠네.");
                end;
            }
            else
            {
            DOS008:
                set @pos, dialog(0,0,1, "오, 자네는 해골죽장을 가지고 있군. 자 그럼 영혼마령봉과 해골죽장을 가지고 동시에 서로에게 마법을  사용하는 것이지! 핫!");
                if(@pos == 0) end;

                delitem @sd, name2itemid("영혼마령봉"), 1; message @sd, 3, "영혼마령봉 주었습니다.";
                delitem @sd, name2itemid("해골죽장"), 1; message @sd, 3, "해골죽장 주었습니다.";
                additem @sd, name2itemid("원혼마령봉"), 1;
                sound @id, 70;
            DOS009:
                set @pos, dialog(0,0,1, "자 이렇게 원혼마령봉이 완성되었다네, 원한을 가진 영혼이라 색깔이 빨갛게 보이는 것이라네.");
                if(@pos == 0) end;
            DOS010:
                set @pos, dialog(0,1,1, "그럼 조심해서 다루도록 해.");
                if(@pos == 0) end;
                else if(@pos == -1) goto DOS009;
            }
        }
        end;
    COS001:
        set @pos, dialog(0,0,1, "도사들이 사용하는 해골죽장의 해골 속에는 원혼이 들어있다네. 자네들 도사들은 이 해골속의 원혼을 달래며 그 마력을 받아 마법을 쓰는게야.");
        if(@pos == 0) end;

        if(countitem(name2itemid("해골죽장")) == 0)
        {
            set @pos, dialog(0,1,1, "하지만 다른 사람들을 위해서 애쓰는 도사가 가진 무기에 원혼이 담겨 있다는 것이 좀 이상하지 않은가? 자네는 해골죽장을 쓰고 있지 않으니 별로 관심은 없을지 모르지만 말일세.");
            if(@pos == -1) goto COS001;
            else end;
        }
        else
        {
        COS002:
            set @pos, dialog(0,0,1, "그럼 주술사들의 영혼마령봉에 깃든 순수한 영혼을 해골죽장으로 옮겨보는 건 어떨까? 여러가지로 시험해보았네만, 해골에는 순수한 영혼이 잘 머무르지 않으려 하더군.");
            if(@pos == 0) end;
        COS003:
            set @pos, dialog(0,1,1, "그래서 해골을 떼어내고 영혼이 깃들수 있는 봉의 촉을 만들어서 달아보았더니, 예상대로 도사들이 더 강한 마력을 발휘할 수 있게 되더군. 해골죽장속의 원혼은 성불하도록 해주고, 순수한 영혼을 죽장에 깃들게 하면 된다네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
        COS004:
            if(countitem(name2itemid("영혼마령봉")) == 0)
            {
                set @pos, dialog(0,0,1, "영혼마령봉을 가지고 오면 자네의 해골죽장을 더 강하게 바꿔주겠네.");
                end;
            }
            else
            {
        COS005:
                set @pos, dialog(0,0,1, "오, 자네는 영혼마령봉을 가지고 있군. 자 그럼 해골죽장의 원혼을 성불하게 해주고, 영혼마령봉의 영혼이 깃들게 해보세나. 핫!");
                if(@pos == 0) end;    

                delitem @sd, name2itemid("해골죽장"), 1; message @sd, 3, "해골죽장 주었습니다.";
                delitem @sd, name2itemid("영혼마령봉"), 1; message @sd, 3, "영혼마령봉 주었습니다.";
                additem @sd, name2itemid("영혼죽장"), 1;
                sound @id, 70;
        COS006:
                set @pos, dialog(0,0,1, "자 이렇게 영혼죽장이 완성되었다네. 영혼마령봉과 비슷한 색을 띄고 있는 것이, 바로 순수한 영혼이 깃들었다는 증거이지.");
                if(@pos == 0) end;
        COS007:
                set @pos, dialog(0,1,1, "그럼 조심해서 다루도록 해.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS006;        
            }
        }
        end;
    }
    else end;
}