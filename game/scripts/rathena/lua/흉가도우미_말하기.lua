{
    set @obj_id, objid();
    set @rand, rand(1,30);
    if(@rand == 10) say @obj_id, 0, "여기는 흉가입니다. 95 레벨부터 99 레벨 이상 고레벨이 사냥하기에 좋습니다.";
}