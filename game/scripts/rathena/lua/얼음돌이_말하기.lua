{
    set @obj_id, objid();
    set @rand, rand(1,30);
    if(@rand == 10) say @obj_id, 0, "땅에 떨어져서 흙이 묻은 얼음을 씻어드립니다.";
    else if(@rand == 28) say @obj_id, 0, "깨끗한 얼음이 있어야 화채를 만드실 수 있답니다.";
}