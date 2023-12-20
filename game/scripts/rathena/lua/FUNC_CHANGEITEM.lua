{
    set @sd, mysd();
    set @id, myid();
    set @list, list1("안쓰는 물건을 비슷한 종류의 다른 물건으로 바꿔주는 소일거리를 하고 있어요. 혹시 이런 물건 가지고 있으세요?", "낡은어전의칼", "원숭이잔털", "너구리잔털", "은여우잔털", "낡은도깨비방망이", "참나무껍질", "깨진불의수정", "낡은철도", "늑대의큰송곳니", "다랑어살코기뭉치", "노회뭉치");
    if(@list == 1)
    {
        setarray @item$[0], "낡은어전의칼", "원숭이잔털", "너구리잔털", "은여우잔털", "낡은도깨비방망이", "참나무껍질", "깨진불의수정", "낡은철도", "늑대의큰송곳니", "다랑어살코기뭉치", "노회뭉치";
        setarray @change$[0], "어전의칼", "원숭이털", "너구리털", "은여우털", "도깨비방망이", "참나무조각", "불의수정", "철도", "늑대의송곳니", "다랑어살코기", "노회";
        setarray @need[0], 10, 10, 10, 10, 10, 10, 10, 10, 1, 1, 1;
        setarray @success[0], 1, 1, 1, 1, 1, 1, 1, 1, 2, 10, 10;
        set @val, @list-1;
        set @list2, list1(@item$[@val] + " " + @need[@val] + "개를 가져오시면 " + @change[@val] + " " + @success[@val] + "개로 교환해 드려요. 하시겠어요?", "예", "아뇨");
        if(@list2 == 1)
        {
            if(countitem(name2itemid(@item$[@val])) < @need[@val])
            {
                set @pos, dialog(0, 0, 0, @item$[@val] + " " + @need$[@val] + "개를 가져오셔야 바꿔드려요.");
            }
            else
            {
                delitem @sd, name2itemid(@item$[@val]), @need[@val];
                message @sd, 3, @item$[@val] + " 주었습니다";
                additem @sd, name2itemid(@change$[@list]), @success[@val];
            }
        }
        deletearray @item$[0], getarraysize(@item$);
        deletearray @change$[0], getarraysize(@change);
        deletearray @need[0], getarraysize(@need);
        deletearray @success[0], getarraysize(@success);
    }
    return;
}