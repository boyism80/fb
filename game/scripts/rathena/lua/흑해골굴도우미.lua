{
COS00:
    set @pos, dialog(0, 0, 1, "여기는 흑해골굴입니다. 65 레벨부터 90 레벨까지 사냥하기에 좋습니다.\\n");
    if(@pos != 1) end;
    set @pos, dialog(0, 1, 0, "각종 해골 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 호박입니다.\\n");
    if(@pos == -1) goto COS00;    
}