{
    set @sd, mysd();
    set @id, myid();
    set @pos, dialog(0, 0, 1, "시원한 과일 화채를 만들어 드립니다.");
    if(@pos == 0) end;

    if(countitem(name2itemid("홍옥")) < 5 || countitem(name2itemid("국광")) < 5 || countitem(name2itemid("깨끗한얼음")) < 10 || countitem(name2itemid("참수박")) < 2)
    {
        set @pos, dialog(0, 0, 1, "과일 화채의 재료는 홍옥 5개, 국광 5개, 깨끗한얼음 10개, 참수박 2개가 필요합니다.");
        end;
    }
    else
    {
        delitem @sd, name2itemid("홍옥"), 5;
        delitem @sd, name2itemid("국광"), 5;
        delitem @sd, name2itemid("깨끗한얼음"), 10;
        delitem @sd, name2itemid("참수박"), 2;
        message @sd, 3, "홍옥 주었습니다";
        message @sd, 3, "국광 주었습니다";
        message @sd, 3, "깨끗한얼음 주었습니다";
        message @sd, 3, "참수박 주었습니다";
        additem @sd, name2itemid("과일화채"), 1;
        set @pos, dialog(0, 0, 1, "정말 시원하고 맛있는 과일화채가 여기 나왔습니다.");
        end;
    }
}