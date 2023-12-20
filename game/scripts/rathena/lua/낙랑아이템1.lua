{
    set @sd, mysd();
    image 1, 19, 0;
COS001:
    set @pos, dialog(0, 0, 1, "벌써 두루마리 사용법을\\n익히셨군요!! 잘 하셨어요!!\\n그럼 제가 주막으로\\n보내드릴께요...");
    if(@pos == 0) end;
COS002:
    set @pos, dialog(0, 1, 1, "주막에서 두루마리를 다시\\n열어보세요... 꼭이요!!");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS001;

    set @pos, dialog(0, 1, 1, "그럼... 뿅~");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS002;

    if(get_nation(@sd) == 1)
    {
        setarray @mapid[0], 29, 30, 31, 8016, 8017, 8018, 8043, 8044, 8045, 8070, 8071, 8072, 8097, 8098, 8099, 8124, 8125, 8126, 8151, 8152, 8153, 8178, 8179, 8180, 8205, 8206, 8207, 8232, 8233, 8234, 8259, 8260, 8261;
        setarray @x[0], 3, 4, 7, 8, 3, 4, 7, 8;
        setarray @y[0], 5, 5, 5, 5, 9, 9, 9, 9;
    }
    else
    {
        setarray @mapid[0], 15, 16, 17, 7716, 7717, 7718, 7743, 7744, 7745, 7770, 7771, 7772, 7797, 7798, 7799, 7824, 7825, 7826, 7851, 7852, 7853, 7878, 7879, 7880, 7905, 7906, 7907, 7932, 7933, 7934, 7959, 7960, 7961;
        setarray @x[0], 3, 4, 7, 8, 3, 4, 7, 8;
        setarray @y[0], 5, 5, 5, 5, 9, 9, 9, 9;
    }

    set @warp_mapid, @mapid[rand(0, getarraysize(@mapid)-1)];
    set @warp_x, @x[rand(0, getarraysize(@x)-1)];
    set @warp_y, @y[rand(0, getarraysize(@y)-1)];

    deletearray @mapid[0], getarraysize(@mapid);
    deletearray @x[0], getarraysize(@x);
    deletearray @y[0], getarraysize(@y);

    warp @sd, @warp_mapid, @warp_x, @warp_y;

    delitem @sd, name2itemid("낙랑의두루마리1"), 1;
    message @sd, 3, "낙랑의두루마리1 주었습니다";
    additem @sd, name2itemid("낙랑의두루마리2"), 1;
    additem @sd, name2itemid("노란비서"), 1;
    additem @sd, name2itemid("도톨"), 2;
    set $nakrang, 0;
}