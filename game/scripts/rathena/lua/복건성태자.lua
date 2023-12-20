{
    set @sd, mysd();
    set @id, myid();

    if($sell_doll == 0)
    {
        if(countitem(name2itemid("청자다람쥐인형")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아흠.. 어디 재미있는 장난감이 없나..");
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "앗, 귀한 장난감을 가지고 계시네요. 그건... 청자다람쥐인형이군요.");
            if(@pos == 0) end;

            set @list, list1("그걸 제게 파시지 않겠어요?", "그래, 팔께!", "아니, 됐어.");
            if(@list == 1)
            {
COS001:
                set @pos, dialog(0, 0, 1, "고맙습니다! 음... 이건 꽤 귀한 장난감이니까 10만전에 살게요!");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "다른 재미있는 장난감이 있으면 제게 가져와 주세요. 제가 다 사드릴게요.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS001;

                set $sell_doll, 1;
                delitem @sd, name2itemid("청자다람쥐인형"), 1;
                message @sd, 3, "청자다람쥐인형 주었습니다";
                set_money @sd, get_money(@sd)+100000;
                updatestatus @sd, 2;
            }
        }
    }
    else
    {
        if($find_play_object == 0)
        {
            set @pos, dialog(0, 0, 1, "아... 대체 어디 있지? 어디서 잃어버렸더라? 뒤적뒤적...");
            if(@pos == 0) end;

            set @pos, list2(@list, 0, 0, "어, 누구세요? 제게 하실 말씀이라도 있나요?", "난 그냥 지나가는 사람인데. 뭘 그리 찾고 있니?", "아니, 아무것도 아니야.");
            if(@pos == 1 && @list == 1)
            {
                set @pos, dialog(0, 0, 1, "아, 잃어버린 장난감을 찾고 있어요. 여기 복건성에 계시는 관리가 제 아버님이시죠.");
                if(@pos == 0) end;

                set @pos, list2(@list_2, 0, 0, "귀한 장난감들인데, 이번에 또 잃어버리면 아버님이 더 이상 사주시지 않겠대요. 이를 어쩌지...", "장난감은 내가 찾아 줄 테니 여기서 기다리렴.", "그래, 열심히 찾으렴");
                if(@pos == 1 && @list_2 == 1)
                {
COS002:
                    set @pos, dialog(0, 0, 1, "어, 정말요? 고맙습니다! 찾아주시면 저도 뭔가 꼭 보답을 할게요.\\n\\n잃어버린 장난감은 상아주사위, 오색폭죽, 청옥팽이예요.");
                    if(@pos == 0) end;
COS003:
                    set @pos, dialog(0, 1, 1, "상아주사위는 여기 복건성 어딘가에서 잃어버렸고......\\n\\n오색폭죽은 상해 어딘가에서 잃어버렸고......\\n\\n청옥팽이는 강서성에서 잃어버렸죠.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS002;

                    set @pos, dialog(0, 1, 1, "그럼 저는 여기서 기다릴께요. 장난감을 찾으시거든 제게 말씀해 주세요.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS003;

                    set $find_play_object, 1;
                    spellaether @sd, name2spellid("태자의장난감"), 180;
                    addlegend @sd, 37, 7, 0, 1, "복건성태자의 잃어버린 장난감을 찾아주자.";
                    end;
                }
            }
        }
        else
        {
            if(countitem(name2itemid("상아주사위")) == 0 || countitem(name2itemid("오색폭죽")) == 0 || countitem(name2itemid("청옥팽이")) == 0)
            {
                set @pos, dialog(0, 0, 0, "상아주사위는 복건성에서, 오색폭죽은 상해어딘가에서, 청옥팽이는 강서성에서 잃어버렸어요.");
                end;
            }
            else
            {
COS004:
                set @pos, dialog(0, 0, 1, "우와! 상아주사위, 오색폭죽, 청옥팽이를 모두 찾아 주셨군요! 대단해요! 정말 고맙습니다.");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "찾느라 힘드셨죠? 이건 팔과탕이라는 요리인데, 맛도 좋고 약으로도 쓰이는 우리 전통요리랍니다. 한 번 드셔보세요.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS004;

                delitem @sd, name2itemid("상아주사위"), 1;
                delitem @sd, name2itemid("오색폭죽"), 1;
                delitem @sd, name2itemid("청옥팽이"), 1;
                message @sd, 3, "상아주사위 주었습니다";
                message @sd, 3, "오색폭죽 주었습니다";
                message @sd, 3, "청옥팽이 주었습니다";
                additem @sd, name2itemid("팔과탕"), 10;
                set $find_play_object, 0;
                dellegend @sd, 37;
            }
        }
    }
}