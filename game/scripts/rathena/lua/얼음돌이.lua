{
    set @sd, mysd();
    set @id, myid();

    image 0, 87, 3;

    set @rand, rand(1, 5);
    set @list, list1("안녕하세요. 어떻게 오셨나요?", "얼음세척", "얼음을 쪼개주세요");
    if(@list == 1)
    {
        set @pos, dialog(0, 0, 1, "땅에 떨어져 지저분해진 얼음을 깨끗이 세척해드립니다.");
        if(countitem(name2itemid("얼음")) == 0)
        {
            set @pos, dialog(0, 0, 1, "얼음을 구해오시면 씻어드리지요.");
            end;
        }
        set @pos, dialog(0, 0, 1, "여기 흐르는 물에 씻어드리겠습니다.");
        if(@rand == 2 || @rand == 4)
        {
            additem @sd, name2itemid("깨끗한얼음"), 1;
            set @pos, dialog(0, 0, 1, "얼음을 씻어드렸습니다."); 
        }
        else if(@rand == 1)
        {
            delitem @sd, name2itemid("얼음"), 1;
            set @pos, dialog(0, 0, 1, "저런, 얼음이 녹아 없어졌네요.");
        }
        else if(@rand == 3)
        {
            delitem @sd, name2itemid("얼음"), 1;
            set @pos, dialog(0, 0, 1, "물이 따뜻해서 얼음이 녹아버렸습니다.");
        }
        else
        {
            delitem @sd, name2itemid("얼음"), 1;
            set @pos, dialog(0, 0, 1, "얼음이 물에 빠져서 녹았습니다.");
        }
    }
    else if(@list == 2)
    {
        if(countitem(name2itemid("얼음")) == 0)
        {
            set @pos, dialog(0, 0, 1, "얼음을 구해오세요.");
            end;
        }
        set @pos, input2(@num, 0, 20, "1개의 얼음을 쪼개면 작은얼음 10개\\n가 됩니다.\\n작은얼음은 100개까지 소지 가능하므\\n로, 최대 10개의 얼음을 쪼개드릴 수\\n 있겠네요.\\n몇 개의 얼음을 쪼개시겠어요?", "", "개");
        if(countitem(name2itemid("얼음")) >= @num){ delitem @sd, name2itemid("얼음"), @num; additem @sd, name2itemid("작은얼음"), @num*10; }
    }    
}