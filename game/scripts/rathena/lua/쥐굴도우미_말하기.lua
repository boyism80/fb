{
    set @obj_id, objid();
    set @rand, rand(1,30);
    if(@rand == 10) say @obj_id, 0, "여기는 쥐굴입니다. 초보자에게 적합한 던전으로, 5 레벨부터 10 레벨까지 사냥하기에 좋습니다.";
}