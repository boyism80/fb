{
    set @sd, mysd();
    set @id, myid();

    set @pos, list2(@list, 0, 0, "무엇을 도와드릴까요?", "출항시간에 대해서 알려주세요.", "배를 태워주세요.");
    if(@pos == 0) end;
    else if(@list == 1)
    {
COS001:
        set @pos, dialog(0, 0, 1, "고구려에서 일본으로 건너가는 배는 하루 총 12번입니다.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "2시부터 24시까지 매 2시간마다 일본으로 건너가는 배가 출항합니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "출항하기전 미리 배에 오르고 있어야만 일본으로 갈 수 있음을 명심하시기 바랍니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;
COS004:
        set @pos, dialog(0, 1, 1, "감사합니다.");
        if(@pos == -1) goto COS003;
        end;
    }
    else if(@list == 2)
    {
        if(    gettime(2, 0) == 1 || gettime(2, 0) == 3 || gettime(2, 0) == 5 || gettime(2, 0) == 7 ||
            gettime(2, 0) == 9 || gettime(2, 0) == 11 || gettime(2, 0) == 13 || gettime(2, 0) == 15 ||
            gettime(2, 0) == 17 || gettime(2, 0) == 19 || gettime(2, 0) == 21 || gettime(2, 0) == 23)
        {
            if(gettime(1, 0) >= 40 && gettime(1, 0) <= 59)
            {
COS005:
                set @pos, dialog(0, 0, 1, "배가 선착장에 정박해 있습니다.");
                if(@pos == 0) end;
COS006:
                set @pos, list2(@list, 0, 1, "배에 승선하시길 원하십니까?", "네, 승선시켜 주시기 바랍니다.", "아니오, 타지 않겠습니다.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS005;
                else if(@list == 1)
                {
                    if(get_level(@sd) < 21)
                    {
                        set @pos, dialog(0, 0, 1, "레벨 21이상만 이용 가능합니다.");
                        end;
                    }

                    if(countitem(name2itemid("고구려-일본승선권")) == 0)
                    {
                        set @pos, dialog(0, 0, 1, "승선권이 없으면 선실에 탑승하실 수 없습니다.");
                        end;
                    }
                    else
                    {
                        delitem @sd, name2itemid("고구려-일본승선권"), 1;
                        message @sd, 3, "고구려-일본승선권 주었습니다";
COS007:
                        set @pos, dialog(0, 0, 1, "그럼, 선실에 승선시켜드리도록 하겠습니다.");
                        if(@pos == 0) end;
COS008:
                        set @pos, dialog(0, 1, 1, "배가 출항한뒤 일정 시간 후에 일본에 도착하게 됩니다. 즐거운 여행 되시길..");
                        if(@pos == 0) end;
                        else if(@pos == -1) goto COS007;

                        warp @sd, name2mapid("고구려-일본배선실"), rand(2, 15), 7;
                        end;
                    }
                }
                else if(@list == 2)
                {
                    set @pos, dialog(0, 0, 1, "네, 안녕히 가십시오.");
                    end;
                }
                else end;
            }
            else
            {
                set @pos, dialog(0, 0, 0, "배가 선착장에 없습니다.");
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 0, "배가 선착장에 없습니다.");
            end;
        }
    }
    else end;
}