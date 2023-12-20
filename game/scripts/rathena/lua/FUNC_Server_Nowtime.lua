{
    set @nowtime, gettime(0, 0) + (gettime(1, 0)*60) + (gettime(2, 0)*3600) + ((gettime(3, 0)+callfunc("Server_Lasttime", gettime(4, 0)))*86400) + ((gettime(5, 0)-2011)*31536000);
    return @nowtime;
    return;
}