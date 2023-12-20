{
    set @sd, mysd();
    set @id, myid();

    if($sick_children == 1)
    {
        if($sick_medical == 0)
        {
            set @pos, list2(@list, 0, 0, "무슨 일인가? 급한 일이라도 있는가?", "실은 아픈 아이가 있는데...", "아니오, 아무 일도 아닙니다.");
            if(@pos == 1 && @list == 1)
            {
COS009:
                set @pos, dialog(0, 0, 1, "......음... 꼬마가 기력이 없고 말을 잘 하지 못한다... 그럴 때에는 삼전신보탕이 제격이지.");
                if(@pos == 0) end;
COS010:
                set @pos, dialog(0, 1, 1, "만드는 것은 쉽다네, 하지만 지금 재료가 다 떨어졌군. 자네가 재료를 구해다 주면 삼전신보탕을 바로 만들어 주지.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS009;
COS011:
                set @pos, dialog(0, 1, 1, "삼전보신탕을 만들기 위해서는 감초, 녹용, 국광이 필요하지. 감초는 광동성 푸줏간에서, 녹용은 어디든 푸줏간에서 쉽게 구할 수 있고, 국광은 어딘가의 미궁에서 구할 수 있다고 하네.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS010;

                set @pos, dialog(0, 1, 1, "재료를 가져오기만 하면 만드는 건 금방이라네. 그럼 힘 내게나.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS011;

                addlegend @sd, 36, 7, 0, 1, "삼전보신탕 재료를 구하자.";
                set $sick_medical, 1;
                end;
            }
            else end;
            end;
        }
        if($sick_medical == 1)
        {
            if(countitem(name2itemid("녹용")) == 0 || countitem(name2itemid("국광")) == 0 || countitem(name2itemid("감초")) == 0)
            {
                set @pos, dialog(0, 0, 0, "삼전신보신탕을 만들기 위한 재료가 부족한 것 같네만. 녹용과 국광 그리고 감초가 있어야 제작이 가능하지.");
                end;
            }
            else
            {
COS012:
                set @pos, dialog(0, 0, 1, "모든 재료를 가져왔군! 수고했네. 자, 그럼 삼전신보탕을 만들어 볼까? 잠시만 기다리게.");
                if(@pos == 0) end;

                set @pos, dialog(0, 1, 1, "자, 자 되었네. 이걸 먹으면 그 애도 건강해질 거야. 허허. 수고했네.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS012;

                delitem @sd, name2itemid("녹용"), 1;
                delitem @sd, name2itemid("국광"), 1;
                delitem @sd, name2itemid("감초"), 1;
                message @sd, 3, "녹용 주었습니다";
                message @sd, 3, "국광 주었습니다";
                message @sd, 3, "감초 주었습니다";
                additem @sd, name2itemid("삼전신보탕"), 1;
                addlegend @sd, 36, 7, 0, 1, "삼전보신탕을 만들었다.";
                set $sick_medical, 2;
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 1, "....");
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "장안성엔 어쩐일로 오셨나? 보약이라도 한 채 지어먹으러 오셨나?");
        end;
    }
}