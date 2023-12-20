{
    set @obj_id, objid();
    set @rand, rand(1,30);
     if(@rand == 10) say @obj_id, 0, "부여성에 오신걸 환영합니다~";
}