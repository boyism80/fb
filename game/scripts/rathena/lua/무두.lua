{
    set @sd, mysd();
    set @id, myid();
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "초혼술방법");
    if(@ret == 1)
    {
        if($shark_weapon < 3)
        {
            set @pos, dialog(0, 0, 0, "자네는 아직 알 때가 아니군.");
            end;
        }
        else
        {
            if($shark_weapon == 3)
            {
COS001:
                set @pos, dialog(0, 0, 1, "상어장군을 저승에서 불러내고 싶다고?\\n\\n매우 어려운 일을 부탁하는구만.");
                if(@pos == 0) end;
COS002:
                set @pos, dialog(0, 1, 1, "사어장군을 소환하기 위해서는 용궁의정기가 필요하다네.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS001;
COS003:
                set @pos, dialog(0, 1, 1, "용궁의정기를 만드는 것은 내가 모르니 다른 사람한테 가서 물어보시게.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS002;

                set $shark_weapon, 4;
                addlegend @sd, 24, 7, 0, 11, "초혼술 방법을 알아보자.";
                end;
            }
            if($shark_weapon >= 4 && $shark_weapon <= 9)
            {
                set @pos, dialog(0, 0, 0, "아직 용궁의정기를 다 모으지 못했나 보군..");
                end;
            }
            if($shark_weapon == 10)
            {
                if(countitem(name2itemid("용궁의정기1")) == 0 || countitem(name2itemid("용궁의정기2")) == 0 || countitem(name2itemid("용궁의정기3")) == 0 || countitem(name2itemid("용궁의정기4")) == 0 || countitem(name2itemid("용궁의정기5")) == 0)
                {
                    set @pos, dialog(0, 0, 0, "아직 용궁의정기를 다 모으지 못했나 보군..");
                    end;
                }
                else
                {
COS004:
                    set @pos, dialog(0, 0, 1, "용궁의정기를 가져왔군.");
                    if(@pos == 0) end;
COS005:
                    set @pos, dialog(0, 1, 1, "그럼 어디보자...아수라 마차라 바하라 미다라...하아아압!!!");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS004;

                    set $shark_weapon, 11;
                    addlegend @sd, 24, 7, 0, 11, "무기제조법을 얻다.";
                    delitem @sd, name2itemid("용궁의정기1"), 1;
                    delitem @sd, name2itemid("용궁의정기2"), 1;
                    delitem @sd, name2itemid("용궁의정기3"), 1;
                    delitem @sd, name2itemid("용궁의정기4"), 1;
                    delitem @sd, name2itemid("용궁의정기5"), 1;
                    message @sd, 3, "용궁의정기1 주었습니다";
                    message @sd, 3, "용궁의정기2 주었습니다";
                    message @sd, 3, "용궁의정기3 주었습니다";
                    message @sd, 3, "용궁의정기4 주었습니다";
                    message @sd, 3, "용궁의정기5 주었습니다";
                    additem @sd, name2itemid("무기제조법"), 1;

                    set @pos ,dialog(0, 0, 1, "상어장군이 하는 말을 적었으니 이 두루마리를 가져가게.");
                    end;
                }
            }
            else
            {
                set @pos, dialog(0, 0, 1, "......");
                end;
            }
        }
    }
    else end;
}