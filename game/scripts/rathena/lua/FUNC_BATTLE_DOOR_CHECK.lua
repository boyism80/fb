{
    set @target_id, getarg(0);
    if(istype(@target_id) == 2)
    {
        set @mobname$, get_mobname(@target_id);
        if(@mobname$ == "성문1" || @mobname$ == "성문2" || @mobname$ == "성문3" || @mobname$ == "성문4" ||
        @mobname$ == "성문5" || @mobname$ == "성문6" || @mobname$ == "성문7" || @mobname$ == "성문8")
        {
            end;
        }
    }
    return;
}