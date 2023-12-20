{
    set @sd, mysd();
    set @id, myid();
    image 0, 544, 0;
    if(get_joblevel(@sd) == 3)
    {
        if(get_job(@sd) == 1 || get_job(@sd) == 2) { set @hp, 1400000; set @mp, 0; }
        else if(get_job(@sd) == 3) { set @hp, 650000; set @mp, 900000; }
        else if(get_job(@sd) == 4) { set @hp, 600000; set @mp, 750000; }

        if(get_purehp(@sd) < @hp || get_puremp(@sd) < @mp) goto COS00008;

        if(!$fourclass) goto COS00000;
        else if($fourclass == 1) goto COS00001;
        else if($fourclass == 2) goto COS00002;
        else if($fourclass == 3) goto COS00003;
        else if($fourclass == 4) goto COS00004;
        else if($fourclass == 5) goto COS00005;
        else if($fourclass == 6) goto COS00007;
        else if($fourclass == 7) goto COS00008;
        else goto COS00008;
COS00000:
        set @pos, dialog(1, 0, 1, "자네도 이 심상치 않은 기운이 느껴지는가?\\n\\n요사이, 이곳 저곳에서 불길한 징조가 나타나고\\n있었다네.\\n\\n곳곳의 풍수를 살펴보면 만물이 있어야 할\\n자리를 지키지 못하고 있으며, \\n별자리마저 알 수\\n없는 형태로 일그러져버렸네.");
        if(@pos == 0) end;
        set @pos, list2(@ret, 1, 1, "정녕 이대로 끝이란 말인가?", "제가 오제의 음모를 막아보겠습니다.", "신들이 하는 일을 인간이 어쩔 순 없겠죠.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS00000;
        else if(@ret == 1)
        {
            set $fourclass, 1;
            addlegend @sd, 7, 7, 0, 16, "소호 암살의 임무를 받다.";
        }
        end;
COS00001:
        if(countitem(name2itemid("소호의증표")) > 0)
        {
            set $fourclass, 2;
            delitem @sd, name2itemid("소호의증표"), 1;
            set @pos, dialog(1, 0, 1, "소호를 잘 물리쳐 주었네. 하지만, 이제\\n시작이라네.\\n\\n이번엔 전욱을 상대해야 하네.\\n\\n전욱의 신전이 위치한 곳은 국내성이네.");
            addlegend @sd, 7, 7, 0, 16, "전욱 암살의 임무를 받다.";
            end;
        }
        else
        {
            set @pos, dialog(1, 0, 0, "아직 소호를 물리치지 못했군..");
            end;
        }
        end;
COS00002:
        if(countitem(name2itemid("전욱의증표")) > 0)
        {
            set $fourclass, 3;
            delitem @sd, name2itemid("전욱의증표"), 1;
            set @pos, dialog(1, 0, 1, "전욱을 잘 물리쳐 주었네.\\n\\n이번엔 제곡을 상대해야 하네.\\n\\n제곡의 신전이 위치한 곳은 부여성이네.");
            addlegend @sd, 7, 7, 0, 16, "제곡 암살의 임무를 받다.";
            end;
        }
        else
        {
            set @pos, dialog(1, 0, 0, "아직 전욱을 물리치지 못했군..");
            end;
        }
        end;
COS00003:
        if(countitem(name2itemid("제곡의증표")) > 0)
        {
            set $fourclass, 4;
            delitem @sd, name2itemid("제곡의증표"), 1;
            set @pos, dialog(1, 0, 1, "제곡을 잘 물리쳐 주었네.\\n\\n이번엔 요제를 상대해야 하네.\\n\\n요제의 신전이 위치한 곳은 일본이네.");
            addlegend @sd, 7, 7, 0, 16, "요제 암살의 임무를 받다.";
            end;
        }
        else
        {
            set @pos, dialog(1, 0, 0, "아직 제곡을 물리치지 못했군..");
            end;
        }
        end;
COS00004:
        if(countitem(name2itemid("요제의증표")) > 0)
        {
            set $fourclass, 5;
            delitem @sd, name2itemid("요제의증표"), 1;
            set @pos, dialog(1, 0, 1, "요제를 잘 물리쳐 주었네.\\n\\n이번엔 순제를 상대해야 하네.\\n\\n순제의 신전이 위치한 곳은 광동성이네.");
            addlegend @sd, 7, 7, 0, 16, "순제 암살의 임무를 받다.";
            end;
        }
        else
        {
            set @pos, dialog(1, 0, 0, "아직 요제를 물리치지 못했군..");
            end;
        }
        end;
COS00005:
        if(countitem(name2itemid("순제의증표")) > 0)
        {
COS00006:
            set @pos, dialog(1, 0, 1, "대단하네! 드디어 오제를 모두 멸하였군.\\n\\n휴우~ 이것으로 끝난 것인가. 이제 반고의\\n기운이 사라지겠지...?!\\\n\\n아니, 이것은? 이것이 어떻게 된 일이란 말인가?\\n\\n반고의 기운이 이토록 온 천지를 덮고 있다니....\\n어째서 이토록 증폭되고 있단 말인가!");
            if(@pos == 0) end;
            set @pos, list2(@ret, 1, 1, "결국 우리 인간은 신들의 예언대로 자멸하고 마는군.\\n\\n으흑흑!", "이 땅이 아직 건재한데, 포기하긴 이릅니다.", "반고를 제거하면 되지 않겠습니까?");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS00006;
            else if(@ret == 1)
            {
                set $fourclass, 6;
                delitem @sd, name2itemid("순제의증표"), 1;
                set @pos, dialog(1, 0, 1, "자네에게 이런 짐을 지우는 것이 못내\\n안쓰럽네만, 기회는 지금 뿐일세.\\n\\n어서 광동성 신전으로 돌아가 반고를 물리치고\\n그 심장을 구해오게!");
                addlegend @sd, 7, 7, 0, 16, "반고 암살의 임무를 받다.";
                end;
            }
        }
        else
        {
            set @pos, dialog(1, 0, 0, "아직 순제를 물리치지 못했군..");
            end;
        }
        end;
COS00007:
        if(countitem(name2itemid("반고의심장")) > 0)
        {
            set $fourclass, 7;
            dellegend @sd, 7;
            set @pos, dialog(1, 0, 1, "반고의 심장을 들고 천상계의 천성랑을\\n찾아가게.\\n\\n이번 일로 최고의 경지에 오른 것을 증명한\\n자네인만큼, 그에 걸맞는 새로운 지위를\\n인정받을 수 있을 것이네.");
            end;
        }
        else
        {
            set @pos, dialog(1, 0, 0, "아직 반고를 물리치지 못했군..");
            end;
        }
        end;
    }
    else
    {
COS00008:
        set @pos, dialog(1, 0, 0, "자네도 이 심상치 않은 기운이 느껴지는가?\\n\\n요사이, 이곳 저곳에서 불길한 징조가 나타나고\\n있었다네.\\n\\n곳곳의 풍수를 살펴보면 만물이 있어야 할\\n자리를 지키지 못하고 있으며, \\n별자리마저 알 수\\n없는 형태로 일그러져버렸네.");
        end;
    }
}