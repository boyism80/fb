{
    set @sd, mysd();
    set @id, myid();

    if($jang_general == 1)
    {
        set @pos, list2(@list, 0, 0, "안녕하십니까. 허허, 아까부터 뭔가 찾으시는 눈치신데... 뭘 그리 찾으시는지요?", "황비연이라는 사람을 찾고 있습니다.", "아무 것도 아닙니다.");
        if(@pos == 1 && @list == 1)
        {
            set @pos, dialog(0, 0, 1, "황비연? ...음, 황비연이라면 요즘 탐관오리의 재물을 훔쳐 가난한 사람들에게 나눠 준다는 의적 아닙니까?");
            if(@pos == 0) end;

            set @pos, list2(@list_2, 0, 0, "참으로 훌륭한 사람이지요. 그런데 그는 왜 찾으십니까?", "그를 잡아서 현상금을 타려고 합니다.", "꼭 한 번 만나 보고 싶은 사람입니다.");
            if(@pos == 0) end;
            else if(@list_2 == 1)
            {
                set @pos, dialog(0, 0, 1, "황비연이 현상금이 높긴 하죠... 하지만 쉽게 잡기는 어려울 겁니다.");
                end;
            }
            else if(@list_2 == 2)
            {
COS001:
                set @pos, dialog(0, 0, 1, "그렇습니까! 하하. 아마 황비연에게 도움을 받으셨었던 모양이지요?\\n\\n실은 저도 그를 꼭 한 번 만나 보고 싶지만, 한 번도 본 적이 없답니다. 하하...");
                if(@pos == 0) end;
COS002:
                set @pos, dialog(0, 1, 1, "......");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS001;
COS003:
                set @pos, dialog(0, 1, 1, "......그를 꼭 만나고 싶으시다면, 제 부탁 하나만 들어주시겠습니까? 아시다시피 황비연은 신출귀몰하고, 사람들이 모두 그를 감싸주기 때문에 아무도 그의 행방을 모르지요.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS002;
COS004:
                set @pos, dialog(0, 1, 1, "하지만 저는 사실 황비연을 좀 알고 있습니다. 당신이 제 부탁을 들어주신다면 황비연과 만나 보실 수 있도록 자리를 만들어 보겠습니다.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS003;

                set @pos, list2(@list_3, 0, 0, "...자, 어떻습니까?", "알겠습니다, 그렇게 하죠.", "싫습니다. 어째 꺼림칙하군요.");
                if(@pos == 1 && @list_3 == 1)
                {
COS005:
                    set @pos, dialog(0, 0, 1, "감사합니다. 그럼...\\n\\n이 노비문서를 도삭산 100층 주막에 계시는 분께 전해 주십시오. 일이 끝난 다음 제게 오시면 약속대로 정보를 드리겠습니다.");
                    if(@pos == 0) end;

                    set @pos, dialog(0, 1, 1, "그럼 수고스러우시겠지만 잘 부탁드립니다.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS005;

                    set $jang_general, 2;
                    additem @sd, name2itemid("노비문서"), 1;
                    addlegend @sd, 39, 7, 0, 16, "도삭산 100층으로가 노비문서를 전해주자.";
                    end;
                }
                else end;
                end;
            }
            else end;
            end;
        }
        else end;
        end;
    }
    if($jang_general == 2)
    {
        set @pos, dialog(0, 0, 1, "아직 도삭산 100층에 계신분을 만나지 못하셨나 보군요.");
        end;
    }
    if($jang_general == 3)
    {
        if(countitem(name2itemid("보패")) == 0)
        {
            set @pos, dialog(0, 0, 1, "아직 도삭산 100층에 계신분을 만나지 못하셨나 보군요.");
            end;
        }
        else
        {
            set @pos, list2(@list, 0, 0, "문서는 전해 주고 오셨는지요?", "예, 확실히 전해 줬습니다.", "아니오, 아직...");
            if(@pos == 1 && @list == 1)
            {
                set @pos, dialog(0, 0, 1, "그렇군요. 감사합니다. 수고하셨습니다... 그런데 이 보패는 웬 것입니까?");
                if(@pos == 0) end;

                set @pos, list2(@list_2, 0, 0, "예? 도재영님이 이것을 제게 전해달라고 하셨다고요? 정말입니까?", "예, 당신을 자랑스러워하셨습니다. 황비연씨.");
                if(@pos == 1 && @list_2 == 1)
                {
                    set @pos, dialog(0, 0, 1, "...다 알고 계셨군요. 하하. 이거 들켜버렸네요. 죄송합니다. 예, 제가 황비연입니다.\\n\\n부탁도 들어 주셨고, 제 할 일도 다 끝났습니다. 많은 사람을 도왔고, 나으리께서도 저를 자랑스럽게 여겨 주신다니 여한이 없습니다.");
                    if(@pos == 0) end;

                    set @pos, list2(@list_3, 0, 0, "자, 이제 저를 체포하셔서 관아에 넘겨 주십시오.", "그럴 수는 없습니다.");
                    if(@pos == 1 && @list_3 == 1)
                    {
                        set @pos, list2(@list_4, 0, 0, "예? 무슨 말씀이신지...", "당신은 의적으로서 많은 사람을 도왔죠.");
                        if(@pos == 1 && @list_4 == 1)
                        {
                            set @pos, list2(@list_5, 0, 0, "......", "그런 사람을 체포할 수는 없습니다.");
                            if(@pos == 1 && @list_5 == 1)
                            {
                                set @pos, list2(@list_6, 0, 0, "음......하지만 의적이라고는 해도 도둑질은 나쁜 일입니다. 저는 죗값을 치러야 합니다.", "앞으로 좋은 일을 더 많이 하시면 되지 않겠습니까?");
                                if(@pos == 1 && @list_6 == 1)
                                {
COS006:
                                    set @pos, dialog(0, 0, 1, "아... 정말로 그럴까요? 부끄럽습니다...\\n\\n하지만 당신의 이야기를 들어 보니 그런 것 같기도 합니다. 그게 더 보람있는 길이란건 확실하겠지요...");
                                    if(@pos == 0) end;
COS007:
                                    set @pos, dialog(0, 1, 1, "그렇다면...\\n\\n이것을 가져가 주십시오. 제가 쓰던 머리띠입니다. 이것을 관아에 가져가시면 제가 죽었다고 생각할 것이고, 아마 제 현상금을 받으실 수 있을 겁니다.");
                                    if(@pos == 0) end;
                                    else if(@pos == -1) goto COS006;
COS008:
                                    set @pos, dialog(0, 1, 1, "보답이라고 하기엔 뭣하지만... 훔친 재물은 전부 가난한 사람들에게 나눠줬기 때문에 드릴 것이 없군요. 하하...");
                                    if(@pos == 0) end;
                                    else if(@pos == -1) goto COS007;

                                    set @pos, dialog(0, 1, 1, "저는 더 많은 사람들을 돕도록 하겠습니다. 물론 이제 도둑질은 하지 않겠습니다. 떳떳한 방법으로 사람들을 도울 겁니다. 정말 감사합니다.");
                                    if(@pos == 0) end;
                                    else if(@pos == -1) goto COS008;

                                    set $jang_general, 4;
                                    delitem @sd, name2itemid("보패"), 1;
                                    message @sd, 3, "보패 주었습니다";
                                    additem @sd, name2itemid("황비연의머리띠"), 1;
                                    addlegend @sd, 39, 7, 0, 16, "황비연의머리띠를 관아에 가져가자.";
                                    end;                                    
                                }
                                else end;
                                end;
                            }
                            else end;
                            end;                    
                        }
                        else end;
                        end;
                    }
                    else end;
                    end;
                }
                else end;
                end;
            }
            else end;
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "안녕하신가요?");
        end;
    }
}