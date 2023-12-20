{
    set @obj_id, objid();
    set @rand, rand(1,30);
    if(@rand == 10) say @obj_id, 0, "여기는 사슴굴입니다. 10 레벨부터 25 레벨까지 사냥하기에좋습니다.";
}