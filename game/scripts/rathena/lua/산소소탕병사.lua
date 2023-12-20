{
    set @rand, rand(1, 3);
    if(@rand == 1)
    {
        set @pos, dialog(0, 0, 1, "여긴 어쩐일이요?");
    }
    else if(@rand == 2)
    {
        set @pos, dialog(0, 0, 1, "우리와 같이 산소소탕을 하러 오신건가?");
    }
    else
    {
        set @pos, dialog(0, 0, 1, "에헴.. 내가 산소들을 몽땅 소탕해주지!");
    }
}