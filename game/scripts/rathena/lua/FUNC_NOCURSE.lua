{
    set @target_id, getarg(0);
    set @type, getarg(1);
    if(@type == 0)
    {
        callfunc "MANA_DELAY", 0, name2spellid("용의제일주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제이주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제삼주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제사주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제오주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제육주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제칠주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제팔주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제구주"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제일노"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제이노"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제삼노"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제사노"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제오노"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제육노"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제칠노"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제팔노"), id2sd(@target_id);
        callfunc "MANA_DELAY", 0, name2spellid("용의제구노"), id2sd(@target_id);
    }
    else if(@type == 1)
    {
        set @target_sd, id2sd(@target_id);
        if(spellaethered(@target_sd, name2spellid("용의제일노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제이노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제삼노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제사노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제오노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제육노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제칠노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제팔노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제구노")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제일주")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제이주")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제삼주")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제사주")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제오주")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제육주")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제칠주")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제팔주")) == 1 ||
           spellaethered(@target_sd, name2spellid("용의제구주")) == 1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if(@type == 2)
    {
        if(mobspellaethered(@target_id, name2spellid("용의제일노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제이노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제삼노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제사노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제오노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제육노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제칠노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제팔노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제구노")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제일주")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제이주")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제삼주")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제사주")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제오주")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제육주")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제칠주")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제팔주")) == 1 ||
           mobspellaethered(@target_id, name2spellid("용의제구주")) == 1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return;
}