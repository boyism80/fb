{
    set @sd, mysd();
    set @id, myid();

COS001:
    set @pos, dialog(0, 0, 1, "문신이 들어간 장신구와 재료를 가지고 오면 강력한 힘을 가진 주술을 장신구에 걸어주겠네.\\n\\n재료는 행료, 기름, 문신이 들어간 장신구일세.");
    if(@pos == 0) end;
COS002:
    set @pos, list2(@list, 0, 1, "어떤 장신구에 주술을 걸고 싶은가?", "문신장갑에 주술을 걸어주십시오.", "문신팔찌에 주술을 걸어주십시오.", "문신반지에 주술을 걸어주십시오.", "문신선류에 주술을 걸어주십시오.", "문신방패에 주술을 걸어주십시오.", "문신투구에 주술을 걸어주십시오.");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS001;
    else if(@list > 0)
    {
        setarray @item$[1], "장갑", "팔찌", "반지", "선류", "방패", "투구";
COS003:
        set @pos, dialog(0, 0, 1, "조상신이시여. 조상의 넋을 기리는 문신을 새긴 이 물건에 당신의 힘을 실어주소서...");
        if(@pos == 0) { goto failed; end; }
COS004:
        if(countitem(name2itemid("문신" + @item$[@list])) == 0 || countitem(name2itemid("향료")) == 0 || countitem(name2itemid("기름")) == 0)
        {
            set @pos, dialog(0, 0, 0, "재료가 부족한 것 같은데?");
        }
        else
        {
            delitem @sd, name2itemid("문신" + @item$[@list]), 1;
            delitem @sd, name2itemid("향료"), 1;
            delitem @sd, name2itemid("기름"), 1;
            message @sd, 3, "문신" + @item$[@list] + " 주었습니다";
            message @sd, 3, "향료 주었습니다";
            message @sd, 3, "기름 주었습니다";
            additem @sd, name2itemid("주술" + @item$[@list]), 1;
            set @pos, dialog(0, 0, 1, "문신" + @item$[@list] + "에 주술을 거는 데 성공했네... 부디 뜻깊은 일에 사용하게나. 과거 이 섬을 폭염왕의 마수에서 잠시나마 구해 줬던 영웅처럼 말일세...");
        }
        failed:
        deletearray @item$[1], getarraysize(@item$);
        end;
    }
    else end;
}