{
COS00:
    set @pos, dialog(0, 0, 1, "여기는 사슴굴입니다. 10 레벨\\n부터 25 레벨까지 사냥하기에\\n좋습니다.\\n");
    if(@pos != 1) end;
    set @pos, dialog(0, 1, 0, "주로 사슴들이 등장하며, 얻을\\n 수 있는 아이템은 주로 사슴\\n고기입니다.\\n");
    if(@pos == -1) goto COS00;    
}