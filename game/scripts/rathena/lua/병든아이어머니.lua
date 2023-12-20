{
    set @sd, mysd();
    set @id, myid();

    if(get_level(@sd) < 56)
    {
        set @pos, dialog(0, 0, 0, "(옆에 있는 아이를 보며 크게 한숨을 내쉰다.)");
        end;
    }

    if($sick_children == 0)
    {
        set @pos, list2(@list, 0, 0, "아... 가난이 원수지, 병든 네게 보약 한 첩 지어줄 형편도 못 되는구나! 이럴 때 네 아버지가 있었다면... 에휴...", "저... 무슨 일이라도 있나요?", "아픈 사람이라도 있나? 뭐, 나랑은 상관없지...");
        if(@pos == 1 && @list == 1)
        {
COS001:
            set @pos, dialog(0, 0, 1, "아... 들으셨나요? .....제 옆에 있는 아이는 저희 아들인데, 얼마 전에 병을 앓고 나더니 기력이 쇠했는지 기운이 없고 말도 못해요.");
            if(@pos == 0) end;
COS002:
            set @pos, dialog(0, 1, 1, "아이 아빠는 오래 전에 돈을 벌겠다고 ㅇ ㅓ디론가 떠나서는 소식이 없고, 저 혼자 남의 집 삯바느질로 먹고 사는데, 이 애에게 보약 한 첩 지어 먹일 형편도 못 되네요.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS001;

            set @pos, list2(@list_2, 0, 0, "이대로 가다간 혹시 이 애가 아예 말을 못하게 되는 건 아닐까 겁이 덜컥 나서, 저도 모르게 그만 소리를 내어 한탄하고 말았군요...", "그런 가엾은 일이... 제가 도와드리겠습니다!", "그래요? 뭐, 저랑은 상관없는 일이니까 전 이만..");
            if(@pos == 1 && @list_2 == 1)
            {
                set $sick_children, 1;
                addlegend @sd, 36, 7, 0, 1, "아픈 아이를 위해 보약을 구하자.";
COS003:
                set @pos, dialog(0, 0, 1, "예? 정말이세요? 이렇게 고마울 데가...\\n\\n아시다시피 보약은 워낙 비싸서, 저희 형편으로는 지을 수가 없답니다.");
                if(@pos == 0) end;
COS004:
                set @pos, dialog(0, 1, 1, "그런데... 재료만 가져다 주면 어떤 보약이든 공짜로 지어주는 사람이 장안성 푸줏간에 있다고 해요.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS003;
COS005:
                set @pos, dialog(0, 1, 1, "그 사람이라면 저희 아들의 증세만 듣고도 보약을 지어줄 것 같은데... 정말 죄송하지만 한번 그분께 여쭤 봐 주세요. 부탁드립니다...");
                if(@pos == -1) goto COS004;
                end;
            }
        }
        else end;
        end;
    }
    if($sick_children == 1)
    {
        if(countitem(name2itemid("삼전신보탕")) == 0)
        {
            set @pos, dialog(0, 0, 0, "보약은 장안성 푸줏간에 가보시면 될거예요.");
            end;
        }
        else
        {
            set @pos, list2(@list, 0, 0, "아아...! 보약을 정말로 구해다 주셨군요! 이 은혜를 어찌 갚아야 할지...", "사례는 필요없습니다. 자, 이 약을 받으세요.", "아무리 가난해도... 수고비 정도는 주시겠죠?");
            if(@pos == 0) end;
            if(@list == 1)
            {
                set $sick_children, 2;

                delitem @sd, name2itemid("삼전신보탕"), 1;
                message @sd, 3, "삼전신보탕 주었습니다";
                additem @sd, name2itemid("청자다람쥐인형"), 1;
                addlegend @sd, 36, 7, 0, 1, "아픈 아이의 병을 치료해 주었다.";
COS006:
                set @pos, dialog(0, 0, 1, "감사합니다... 정말 감사합니다... 이 은혜 언젠가는 꼭 갚겠어요.......");
                if(@pos == 0) end;
COS007:
                set @pos, dialog(0, 1, 1, "......아, 이건 저희 애가 가지고 놀던 장남간인데, 보잘것 없어 보여도 아이 아빠가 예전에 귀한 장난감이라면서 가져온 것이랍니다.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS006;

                set @pos, dialog(0, 1, 1, "오래된 장난감이니 누군가 장난감을 수집하는 사람이라도 있다면, 비싸게 팔 수 있을 거예요.");
                if(@pos == -1) goto COS007;
                end;
            }
            else if(@list == 2)
            {
                set @pos, dialog(0, 0, 0, "저... 그.. 그게...");
                end;
            }
            else end;
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "저번엔 정말 감사했습니다. 아이가 덕분에 건강을 많이 회복했습니다.");
        end;
    }
}