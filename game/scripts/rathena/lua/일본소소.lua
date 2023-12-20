{
    set @sd, mysd();
    set @myid, myid();
    set @haircolor, get_haircolor(@sd);
    image 0, 547, 4;
    set @temp, list1("안녕하세요. 어떻게 오셨나요?", "머리모양을 바꾸고 싶어요", "머리염색");
    if(@temp == 1)
    {
        set @temp, list1("가격에 따라 다른 머리모양을 해드릴 수 있습니다.\\n\\n원하시는 가격을 선택해주세요.", "10만전");
        if(@temp == 1)
        {
REHAIR1:
            set @face, 0;
HAIR1:
            set @pos, list3(@ret, 0, 0, @face, @haircolor, "이 모습으로 바꾸시겠습니까?\\n\\n비용은 10만전 입니다.", "이 모습을 선택하겠습니다.", "다른 머리모양을 보여주세요.");
            if(@pos == 0) { end; }
            if(@ret == 1)
            {
                if(get_money(@sd) < 100000)
                {
                    set @pos, dialog(0, 0, 0, "금액이 " + (10000-get_money(@sd)) + "전 부족합니다.");
                    end;
                }
                set_face @sd, @face;
                set_money @sd, get_money(@sd)-100000;
                updatestatus @sd, 2;
                updatexy @sd;
                updatelookpc @sd, 0;
                updateallpc @sd;
                set @pos, dialog(0, 0, 0, "선택하신 머리로 설정해 드렸습니다.");
            }
            else if(@ret == 2)
            {
                set @face, @face+1;
                if(@face < 99) { goto HAIR1; }
                else
                {
                    set @temp, list1("방금 보신 것이 마지막입니다.\\n\\n처음부터 다시 보시겠습니까?", "예", "아니오");
                    if(@temp == 1) { goto REHAIR1; }
                    else { end; }
                }
            }
        }
    }
    else if(@temp == 2)
    {
        set @pos, dialog(0, 0, 1, "염색을 하시려구요?\\n염색 비용은 100전 이고, 이번에 새로 가져온 염색약은 비싸서 500전 이에요.");
        if(@pos == 0) { end; }
        set @colornumber, list1("원하는 염색을 선택해라", "원래 색깔로 돌려주세요.", "녹색", "감청색", "회갈색", "적갈색", "황토색", "연보라색", "보라색", "바다색", "흰색(500전)", "귤색(500전)", "짙은곤색(500전)", "회색(500전)", "오렌지색(500전)", "갈색(500전)", "연갈색(500전)", "짙은회색(500전)", "밝은회색(500전)");
        if(@colornumber < 9)
        {
            if(get_money(@sd) < 100)
            {
                set @pos, dialog(0, 0, 0, "금액이 " + (100-get_money(@sd)) + "전 부족합니다.");
                end;
            }
            set_haircolor @sd, @colornumber-1;
            set_money @sd, get_money(@sd)-100;
            updatestatus @sd, 2;
            updatexy @sd;
            updatelookpc @sd, 0;
            updateallpc @sd;
            set @pos, dialog(0, 0, 0, "예쁘게 염색이 되었습니다~");
        }
        else if(@colornumber <= 18)
        {
            if(get_money(@sd) < 500)
            {
                set @pos, dialog(0, 0, 0, "금액이 " + (500-get_money(@sd)) + "전 부족합니다.");
                end;
            }
            set_haircolor @sd, @colornumber-1;
            set_money @sd, get_money(@sd)-500;
            updatestatus @sd, 2;
            updatelook @sd;
            updatelookpc @sd, 0;
            set @pos, dialog(0, 0, 0, "예쁘게 염색이 되었습니다~");
        }
    }
    updatelook @sd;
    updatelookpc @sd, 0;
}