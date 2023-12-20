{
    image 0, 612, 0;
    set @sd, mysd();
    set @id, myid();
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "물건 고치기", "모두 고치기", "무기 별칭 명명", "아이템 파괴");
    if(@ret == 1)
    {
        set @ret, list1("무엇을 사시게요?", "기타");
        if(@ret == 1) { callshop 0, 50; }
    }
    else if(@ret == 2) { callshop 1, 51; }
    else if(@ret == 3) { callrepair(); }
    else if(@ret == 4) { callrepairall(); }
    else if(@ret == 5) { callnaming(); }
    else if(@ret == 6) { callfunc "DESTROYITEM"; }
}