{
    image 0, 4, 4;
    set @sd, mysd();
    set @id, myid();
    set_birth @sd, -2;
POS0001:
    set @pos, dialog(0, 0, 1, "대단히 중요하니 끝까지 읽어주세요! 빈번히 발생하는 아이디 해킹을 미연에 방지하기 위해 또 하나의 2차 비밀번호를 정해야 합니다.");
    if(@pos == 0) end;
POS0002:
    set @pos, dialog(0, 1, 1, "이 2차 비밀번호는 한번 결정을 하시면 변경이 불가능 합니다. 이 2차 비밀번호는 자신의 게임 접속 비밀번호를 바꿀 때에만 사용합니다.");
    if(@pos == -1) goto POS0001;
    else if(@pos == 0) end;

POS0003:
    set @pos, dialog(0, 1, 1, "이 2차 비밀번호는 자신의 생년월일로 해주세요. 예를 들면 자신의 생일이 92년 04월 17일이면, 920417으로 입력해주세요. (반드시 6자리)");
    if(@pos == -1) goto POS0002;
    else if(@pos == 0) end;

POS0004:
    set @pos, dialog(0, 1, 1, "이 2차 비밀번호는 분실하실 이유가 없기 때문에(자신의 생년월일이므로) 이 2차 비밀번호를 모르시는 분은 해당 아이디에 대한 소유권을 박탈합니다.");
    if(@pos == -1) goto POS0003;
    else if(@pos == 0) end;

POS0005:
    set @pos, dialog(0, 1, 1, "이 2차 비밀번호를 모르시고 처음의 비밀번호 분실신고를 하셔도 저희는 가르쳐 드릴수가 없습니다. 그 만큼 중요한 것이오니 신중히 그리고 정확히 입력해주세요.");
    if(@pos == -1) goto POS0004;
    else if(@pos == 0) end;

POS0006:
    set @pos, dialog(0, 1, 1, "또한 이 두번째 비밀번호(생년월일)를 입력하지 않으시고 다음에 비밀번호 분실신고를 하셔도 저희는 해드릴수가 없으니 반드시 두번째 비번을 입력하시기 바랍니다.");
    if(@pos == -1) goto POS0005;
    else if(@pos == 0) end;

POS0007:
    set @pos, input2(@birth, 0, 6, "자신의 생년월일을 입력해 주세요.", "저의 생년월일은", "입니다.");
    if(@pos == 0) goto POS0007;

    if(@birth <= 0)
    {
POS0008:
        set @pos, dialog(0, 1, 1, "두번째 비밀번호를 정하지 않으면 게임을 할 수 없습니다. 반드시 해주세요.");
        goto POS0007;
    }

    set @ret, menu("당신의 생년월일이 " + @birth + "가 맞습니까?", "예", "아니오");
    if(@ret == 1)
    {
        set_birth @sd, @birth;
    }
    else
    {
        goto POS0008;
    }
    return;
}