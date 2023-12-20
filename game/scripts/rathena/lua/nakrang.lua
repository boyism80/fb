{
    set @sd, mysd();
    set_userexpevent @sd, 0;
    image 0, 4, 4;
    
    if($nakrang == 1)
    {
        set @pos, dialog(0, 0, 0, "두루마리를 열어보기\\n위해서는 오른쪽의\\n'소지품(단축키i)'을\\n클릭하신 다음 제가 드린\\n두루마리를 더블클릭하시면 됩니다.");
    }
    else
    {
COS001:
        set @pos, dialog(0, 0, 1, get_name(@sd)+"님, 안녕하세요?\\n바람의나라에 오신 것을\\n환영합니다!!!");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "드넓은 바람의 세계로 가시기\\n전에 '두루마리'를 하나\\n드릴테니, 시작하시기 전에\\n꼭!! 열어 보세요..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, dialog(0, 1, 1, "열어보기 위해서는 오른쪽의\\n'소지품(단축키i)'을\\n클릭하신 다음 제가 드린\\n두루마리를 더블클릭하시면\\n됩니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS002;

        set @pos, dialog(0, 1, 1, "자. 그럼, 머나먼 모험의\\n길을 떠나 보시기 바랍니다~\\n제가 드리는 두루마리 꼭!!!\\n열어보세요!!!");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;

        set $nakrang, 1;
        additem @sd, name2itemid("낙랑의두루마리1"), 1;
        image 1, 19, 0;
        set @pos, dialog(0, 0, 0, "<낙랑의두루마리1>을 얻다!!!");
    }
}