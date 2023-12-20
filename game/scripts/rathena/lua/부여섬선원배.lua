{
    if(    gettime(2, 0) == 1 || gettime(2, 0) == 3 || gettime(2, 0) == 5 || gettime(2, 0) == 7 ||
        gettime(2, 0) == 9 || gettime(2, 0) == 11 || gettime(2, 0) == 13 || gettime(2, 0) == 15 ||
        gettime(2, 0) == 17 || gettime(2, 0) == 19 || gettime(2, 0) == 21 || gettime(2, 0) == 23)
    {
        if(gettime(1, 0) >= 40 && gettime(1, 0) <= 59)
        {
            if(obj_getxy(21, 38) <= 0)
            {
                npcspawn 1744, 21, 38, 2, 80, "배";
            }
        }
        else
        {
            if(obj_getxy(21, 38) > 0)
            {
                deletenpc obj_getxy(21, 38), 1744;
            }
        }
    }
    else
    {
        if(obj_getxy(21, 38) > 0)
        {
            deletenpc obj_getxy(21, 38), 1744;
        }
    }
}