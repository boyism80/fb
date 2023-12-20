{
    set @sd, mysd();
    set @id, myid();

    if($alcoholic_drink == 3 && countitem(name2itemid("청심사주")) > 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "허허, 주경원이 청심사주를 만드는것을 자네가 도와주었지? 내 신통력으로 다 보고 있었지.");
        if(@pos == 0) end;

        set @pos, dialog(0, 1, 1, "정말 오랫만에 보는 술이군! 고맙네. 이 술을 다 마셔버리지 않고 가져다 준 정직한 자네에게 선물을 주고 싶군. 요즘은 자네같은 사람도 드물다네. 내 고대금속조각'음을 하나 주지. 어딘가에 쓸모가 있을게야..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        delitem @sd, name2itemid("청심사주"), 1;
        message @sd, 3, "청심사주 주었습니다";

        set $alcoholic_drink, 4;
        additem @sd, name2itemid("고대금속조각'음"), 1;
        addlegend @sd, 40, 7, 0, 20, "청심사주를 천선도사에게 전해주었다.";
    }
    else
    {
        set @pos, dialog(0, 0, 1, "압록강의 물은 정말 신비롭지...");
        end;
    }
}