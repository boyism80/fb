{
    set @sd, mysd();
    set @id, myid();

COS001:
    set @pos, dialog(0, 0, 1, "안녕, 보시다시피 나는 사냥꾼이야. 요즘 잡스러운 괴물들이 다시 들끓더니 외부인까지 들어오더만\\n\\n뭐 나쁜 일만은 아니야. 괴물들을 상대하려면 사람들이 많이 필요하거든. 이를테면 자네가 도와준다거나 할 수 있으니까.");
    if(@pos == 0) end;
COS002:
    set @pos, dialog(0, 1, 1, "아무튼 자네의 두 눈에 정의감이 흘러넘치는걸 보니 괴물들과 싸울 작정인가 보군. 그래, 그럴거야.\\n\\n그렇다면, 나도 가만히 있을수는 없지. 우리 가문에 대대로 전해져 내려오는 가문의 비밀 무기들을 만들어주지");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS001;
COS003:
    set @pos, dialog(0, 1, 1, "각각 거미독창과 저주의단검이라 불리는 무기들일세. 갖고 싶다면 재료만 좀 구해오면 되고... 또 돈도 조금만 주면 끝이지. 후후후후");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS002;
COS004:
    set @pos, list2(@list, 0, 0, "우리 가문 특유의 무기들이야, 골라보게!", "이 거미독창이란게 마음에 드네요", "이 저주의단검이란게 마음에 들어요", "에이, 별로 좋은건 없어보이는데요.");
    if(@pos == 0) end;
    else if(@list == 1)
    {
        set @pos, list2(@list_2, 0, 0, "탁월한 선택! 지금까지 이 창에 맞고 중독되지 않은 몬스터는 없었지!", "만들어주세요", "음, 다시 생각해볼래요");
        if(@pos == 0) end;
        else if(@list_2 == 1)
        {
COS005:
            set @pos, dialog(0, 0, 1, "거미독창을 만들려면 거미의이빨 25개와 독거미줄 50개가 필요해. 그리고 제작비와 수고비, 세금을 합해 5000전이 필요하지.");
            if(@pos == 0) end;
COS006:
            set @pos, list2(@list_3, 0, 0, "뛰어난 성능에 저렴한 가격! 한 번 믿어봐!", "어서 만들어 주세요", "나중에 다시 올게요...");
            if(@pos == 0) end;
            else if(@list_3 == 1)
            {
                if(countitem(name2itemid("거미의이빨")) < 25 || countitem(name2itemid("독거미줄")) < 50 || get_money(@sd) < 5000)
                {
                    set @pos, dialog(0, 0, 0, "재료가 조금 부족한 것 같은데? 다시 한번 살펴봐.");
                    end;
                }
                else
                {
                    set @pos, dialog(0, 0, 1, "좋아좋아, 여기있네. 조심해서 좋은 일에 잘 쓰시게나.");
                    if(@pos == 0) end;

                    delitem @sd, name2itemid("거미의이빨"), 25;
                    delitem @sd, name2itemid("독거미줄"), 50;
                    message @sd, 3, "거미의이빨 주었습니다";
                    message @sd, 3, "독거미줄 주었습니다";
                    additem @sd, name2itemid("거미독창"), 1;
                    end;
                }
            }
            else if(@list_3 == 2)
            {
                set @pos, dialog(0, 0, 0, "또 그세 마음이 바뀐거야? 그럼 나중에 꼭 오라구!");
                end;
            }
            else end;
        }
        else if(@list_2 == 2)
        {
            set @pos, dialog(0, 0, 0, "그래, 조금 더 생각해보고 오라구.");
            end;
        }
        else end;
    }
    else if(@list == 2)
    {
        set @pos, list2(@list_2, 0, 0, "탁월한 안목! 어떤 적이라해도 저주를 걸어버리는 검이지!", "만들어주세요", "음, 다시 생각해볼래요");
        if(@pos == 0) end;
        else if(@list_2 == 1)
        {
COS007:
            set @pos, dialog(0, 0, 1, "저주의단검을 만들려면 늑대의이빨 50개와 늑대의송곳니 10개가 필요해. 그리고 제작비와 수고비, 양도세를 합해 5000전이 필요하지!");
            if(@pos == 0) end;
COS008:
            set @pos, list2(@list_3, 0, 0, "엄청난 성능에 비해 너무나 싼 가격! 이 사람 거짓말 안해!", "좋아요! 만들어주세요!", "사냥꾼이 아니라 장사꾼 같은데요...그만 둘래요.");
            if(@pos == 0) end;
            else if(@list_3 == 1)
            {
                if(countitem(name2itemid("늑대의이빨")) < 50 || countitem(name2itemid("늑대의송곳니")) < 10 || get_money(@sd) < 5000)
                {
                    set @pos, dialog(0, 0, 0, "재료가 조금 부족한 것 같은데? 다시 한번 살펴봐.");
                    end;
                }
                else
                {
                    set @pos, dialog(0, 0, 1, "좋아좋아, 여기있네. 조심해서 좋은 일에 잘 쓰시게나.");
                    if(@pos == 0) end;

                    delitem @sd, name2itemid("늑대의이빨"), 50;
                    delitem @sd, name2itemid("늑대의송곳니"), 10;
                    message @sd, 3, "늑대의이빨 주었습니다";
                    message @sd, 3, "늑대의송곳니 주었습니다";
                    additem @sd, name2itemid("저주의단검"), 1;
                    end;
                }
            }
            else if(@list_3 == 2)
            {
                set @pos, dialog(0, 0, 0, "또 그세 마음이 바뀐거야? 그럼 나중에 꼭 오라구!");
                end;
            }
            else end;
        }
        else if(@list_2 == 2)
        {
            set @pos, dialog(0, 0, 0, "그래, 조금 더 생각해보고 오라구.");
            end;
        }
        else end;    
    }
    else if(@list == 3)
    {
        set @pos, dialog(0, 0, 0, "별로 좋은게 없어보인다니? 한번 사용은 해봤나? 한번 써본다면 그런 말은 나오지 않을걸?");
        end;
    }
    else end;

}