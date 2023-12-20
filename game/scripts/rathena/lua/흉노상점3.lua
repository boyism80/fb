{
    set @sd, mysd();
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기");
    if(@ret == 1) { callshop 0, 93; }
    else if(@ret == 2) { callshop 1, 94; }
}