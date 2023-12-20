{
    set @sd, mysd();
    set @id, myid();

    if($jang_general == 0)
    {
        set @pos, dialog(0, 0, 1, "황궁을 견학하러 왔는가? 허허, 미안하네만 지금은 보시다시피 좀 어수선하다네.");
        if(@pos == 0) end;

        set @pos, list2(@list, 0, 0, "자네 혹시 황비연이라는 자를 아는가?", "잘 모르겠는데요. 그게 누굽니까?", "전 아무것도 모릅니다. 그럼 이만...");
        if(@pos == 0) end;
        else if(@list == 1)
        {
            set @pos, dialog(0, 0, 1, "음... 황비연이라는 자는 요즘 기승을 부리고 있는 신출귀몰한 도둑이지. 자칭 의적으로, 검은 머리띠를 두르고 나타나 탐관오리들의 재물을 훔쳐 가난한 사람들에게 나눠 준다는군.");
            if(@pos == 0) end;

            set @pos, list2(@list_2, 0, 0, "백성들도 황비연에게 도움을 많이 받고 있는 모양이야. 그에 대해 물으면 다들 모른다고 잡아떼더군. 허허, 이거야 원...", "의적이군요? 그럼 슬쩍 눈감아줘도 되지 않을까요?", "고생이 많으시네요. 저는 바쁜 일이 있어서 이만..");
            if(@pos == 1 && @list_2 == 1)
            {
COS001:
                set @pos, dialog(0, 0, 1, "...실은 나도 그렇게 하고 싶지만, 상부에서 그를 잡아들이라고 난리라서 어쩔 수 없다네. 현상금도 크게 걸려 있지.");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "사실 탐관오리들이 부정하게 모아온 재물이긴 하지만, 어찌 되었건 간에 도둑질은 나쁜 일이지 않은가.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS001;

                set @pos, list2(@list_3, 0, 0, "그래서 지금도 황비연을 잡으려고 수색중인데, 정말 신출귀몰하더군. 도저히 잡을 수가 없어...", "저도 협조하겠습니다. 황비연을 만나보고 싶군요.", "솔직히 잡히지 말았으면 좋겠군요. 그럼 전 이만..");
                if(@pos == 1 && @list_3 == 1)
                {
COS002:
                    set @pos, dialog(0, 0, 1, "음! 정말 그래 주겠는가? 고맙네. 자네는 황궁 사람이 아니니 어쩌면 황비연에게 쉽게 접글할 수 있을지도 모르지.");
                    if(@pos == 0) end;

                    set @pos, dialog(0, 1, 1, "황비연이 마지막으로 나타난 장소는 상해일세. 그 부근에 숨어 있을 것 같으니 잘 살펴 보게나. 그럼 수고해주게.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS002;

                    set $jang_general, 1;
                    addlegend @sd, 39, 7, 0, 2, "황비연을 만나러 상해로 가자.";
                    end;
                }
            }
        }
        else if(@list == 2)
        {
            set @pos, dialog(0, 0, 0, "허허.. 싱겁기는..");
            end;
        }
    }
    if($jang_general >= 1 && $jang_general <= 3)
    {
        set @pos, dialog(0, 0, 1, "한번 상해로 가보십시요. 황비연이 마지막으로 나타난 장소가 상해입니다.");
        end;
    }
    if($jang_general == 4)
    {
        set @pos, list2(@list, 0, 0, "황비연은 잡아왔는가?", "그는 죽었고, 그의 머리띠를 가져왔습니다.", "아니오, 아직...");
        if(@pos == 1 && @list == 1)
        {
            if(countitem(name2itemid("황비연의머리띠")) == 0)
            {
                set @pos, dialog(0, 0, 0, "머리띠가 없는것 같은데요?");
                end;
            }
            else
            {
COS003:
                set @pos, dialog(0, 0, 1, "오, 머리띠라... 어디 보세.\\n\\n음! 과연... 이것은 그가 항상 머리에 두르고 있다는 그 머리띠가 분명하군!");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "그가 정말로 죽었는지 어떤지는 자네만이 알고 있겠지만... 허허허.\\n\\n뭐, 이 머리띠면 상부에 황비연이 죽었다고 보고할 수 있겠지. 아무튼 수고했네. 여기 현상금 10만전을 가져가게.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS003;

                set $jang_general, 5;
                addlegend @sd, 39, 7, 0, 2, "의적 황비연 임무를 완벽히 수행하였다.";

                delitem @sd, name2itemid("황비연의머리띠"), 1;
                message @sd, 3, "황비연의머리띠 주었습니다";

                set_money @sd, get_money(@sd)+100000;
                updatestatus @sd, 2;
                end;
            }
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "안녕하신가? 자네 덕분에 난 진급을하여 편한안 삶을 살고 있지. 정말 고맙네.");
        end;
    }
}