{
    set @target_id, obj_getxy(71, 104);
    if(@target_id > 0 && istype(@target_id) == 2 && get_mobname(@target_id) == "성문2")
    {
        deletemob @target_id, mapid();
    }
    deletenpc objid(), mapid();
}