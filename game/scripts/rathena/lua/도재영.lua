{
    set @sd, mysd();
    set @id, myid();

    if($jang_general == 2 && countitem(name2itemid("노비문서")) > 0)
    {
        set @pos, list2(@list, 0, 0, "음... 뉘신지요? 제게 무슨 볼일이라도?", "상해에서 물건을 전해달라는 부탁을 받고 왔습니다.", "아무 것도 아닙니다.");
        if(@pos == 1 && @list == 1)
        {
            set @pos, list2(@list_2, 0, 0, "아니, 이것은... 얼마 전에 풀어 줬던 황비연의 노비문서군요. 생각납니다. 여기서 일하던 노비였는데, 참 의협심이 강한 청년이였죠. 그는 잘 있나요?", "예, 실은...");
            if(@pos == 1 && @list_2 == 1)
            {
POS001:
                set @pos, dialog(0, 0, 1, "예? 뭐라고요? 그게 사실입니까? 하하... 이거 참, 뭐라고 말해야 할지...\\n\\n황비연... 그가 의적이 되어 탐관오리를 혼내주고 사람들을 돕고 있단 말이지요?");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "뭐라고 해야 할까... 참 대견스럽군요. 그가 어느날 갑자기 어려운 사람들을 돕고 싶다며 풀어달라고 할 때에는 반신반의했었지요.");
                if(@pos == 0) end;
                else if(@pos == -1) goto POS001;

                set @pos, list2(@list_3, 0, 0, "지금 보니 그 때 황비연을 저희 집안 노비 신세에서 풀어준 것이 잘한 일이었다는 생각이 드는군요.", ".....");
                if(@pos == 1 && @list_3 == 1)
                {
POS002:
                    set @pos, dialog(0, 0, 1, "이 보패를 그에게 전해주십시오. 이 문서를 제게 전해달라고 부탁했다는 그 사람이 아마 황비연 본인일 겁니다.");
                    if(@pos == 0) end;

                    set @pos, dialog(0, 1, 1, "아마, 아무리 의적이라고 해도 도둑직을 한 것이 부끄러워서 제게 직접 오지 못한 것이겠죠. 하하...\\n\\n그에게 전해 주십시오. 우린 모두 너를 아주 자랑스럽게 여기고 있다고... 하하하...");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto POS002;

                    set $jang_general, 3;

                    delitem @sd, name2itemid("노비문서"), 1;
                    message @sd, 3, "노비문서 주었습니다";
                    additem @sd, name2itemid("보패"), 1;
                    addlegend @sd, 39, 7, 0, 2, "보패를 가지고 다시 상해주민에게 가보자.";
                    end;
                }            
            }
        }
    }
    else
    {
        if($dojaeyoung == 0)
        {
COS001:
            set @pos, dialog(0, 0, 1, "안녕하세요 " + get_name(@sd) + "님. 도삭산을 탐험하고 계시군요, 도삭산은 마법에 걸린 곳이라는 소문이있는데, 그 말처럼 이상한 여러가지 현상들이 일어나는 곳이랍니다.");
            if(@pos == 0) end;
COS002:
            set @pos, dialog(0, 1, 1, "아무래도 이런 마법이 걸린 곳에서 보통 사람이 사는것은 무리인지, 저희 아버지도 요즘 건강이 안좋으시죠.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS001;
COS003:
            set @pos, list2(@list, 0, 1, "의사의 말에 따르면 도삭산의 토끼들이 가지고 있는 약초를 달여서 지은 약을 드시면 차도가 있으실꺼라고 했는데...", "제가 구해드리겠습니다.", "그렇군요..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
            else if(@list == 1)
            {
                set $dojaeyoung, 1;
                addlegend @sd, 9, 7, 0, 1, "도재영의 부탁을 들어주자!";
COS004:
                set @pos, dialog(0, 0, 1, "아.. 정말 감사합니다. " + get_name(@sd) + "님은 마음도 넓으신 분이시군요.");
                if(@pos == 0) end;
COS005:
                set @pos, dialog(0, 1, 1, "도삭산의 토끼들은 151층에서 200층에서 나온다고 합니다. 의사의 말로는 약초잎사귀 5개와 약초가지 1개가 있어야 한다고 하네요.\\n\\n그럼 잘 부탁드리겠습니다! 감사합니다!");
                if(@pos == -1) goto COS0004;
                end;
            }
            else if(@list == 2)    
            {
                set @pos, dialog(0, 0, 0, "그렇죠..이 약초들을 어떻게 구하면 좋으련지..");
                end;
            }
            else end;
        }
        if($dojaeyoung == 1)
        {
            if(countitem(name2itemid("약초가지")) == 0 || countitem(name2itemid("약초잎사귀")) < 5)
            {
                set @pos, dialog(0, 0, 0, "아직 약초를 구를 구하시지 못하셨군요. 의사의 말로는 약초잎사귀 5개와 약초가지 1개가 있어야 한다고 합니다. 부탁드립니다.");
                end;
            }
            else
            {
                set @pos, dialog(0, 0, 1, "약초잎사귀와 약초가지를 구하셨군요! 감사합니다! 이것으로 저희 아버지가 건강을 되찾으시겠군요!");
                if(@pos == 0) end;

                set $dojaeyoung, 2;
                delitem @sd, name2itemid("약초가지"), 1;
                delitem @sd, name2itemid("약초잎사귀"), 5;
                message @sd, 3, "약초가지 주었습니다";
                message @sd, 3, "약초잎사귀 주었습니다";
                additem @sd, name2itemid("강철의구두"), 1;
                addlegend @sd, 9, 7, 0, 1, "도삭산 100층 퀘스트 완료";
                set @pos, dialog(0, 0, 1, "약소하지만 보답으로 강철의구두를 드리겠습니다. 도삭산을 돌아다니시면서 조금이라도 도움이 되길 바랍니다. 그럼..");
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 1, "저번에 약초를 구해주셔서 정말 감사드립니다. 덕분에 아버님의 건강이 많이 좋아지셨습니다.");
            end;
        }
    }
}