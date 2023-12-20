{
    set @obj_id, objid();
    set @rand, rand(1,30);
    if(@rand == 10) say @obj_id, 0, "여기는 물망동입니다. 80 레벨부터 99 레벨까지 사냥하기에좋습니다.";
}