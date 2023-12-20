{
    set @sd, mysd();
    set @id, myid();

    set_state @sd, 0;
    updatelookpc @sd, 1;
    message @sd, 3, "변신시약 해제.";
}