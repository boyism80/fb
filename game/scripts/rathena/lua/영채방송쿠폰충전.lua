{
    if(#broadcast_card == 1)
    {
        set #broadcast_card_count, #broadcast_card_count+1;
        if(#broadcast_card_count >= 300)
        {
            set #broadcast_card, 0;
        }
    }
}