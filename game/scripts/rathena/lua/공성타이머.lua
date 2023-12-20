{
    set @id, objid();
    if(#clan_start == 1)
    {
        set @target_id, obj_getxy(20, 8);
        if(@target_id > 0)
        {
            if(istype(@target_id) == 1)
            {
                set @target_sd, id2sd(@target_id);
                if(strcmp(get_clanname(@target_sd), #clan_mapcheck$))
                {
                    if(#clan_namecheck == 0)
                    {
                        set #clan_name$, get_name(@target_sd);
                        set #clan_namecheck, 1;
                    }
                    else
                    {
                        if(get_name(@target_sd) == #clan_name$)
                        {
                            set #clan_timer, #clan_timer+1;
                            switch(#clan_timer)
                            {
                            case 1:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 5:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 15:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 20:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 35:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 45:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 55:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 60:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 65:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 70:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 73:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 74:
                                announce 5, "[알림] " + get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 옥좌를 " + (#clan_timer*4) + "초간 지키고 계십니다.";
                                break;
                            case 75:
                                if(#clan_mapname$ == "주작성") { set #bird_castle$, get_clanname(@target_sd); }
                                else if(#clan_mapname$ == "현무성") { set #turtle_castle$, get_clanname(@target_sd); }
                                else if(#clan_mapname$ == "청룡성") { set #dragon_castle$, get_clanname(@target_sd); }
                                else if(#clan_mapname$ == "백호성") { set #tiger_castle$, get_clanname(@target_sd); }

                                announce 5, get_clanname(@target_sd) + " 문파의 " + get_name(@target_sd) + "님이 " + #clan_mapname$ + "을 탈환하였습니다.";

                                if(#clan_mapname$ == "주작성") { npcspawn name2mapid("주작성입구"), 70, 105, 2, 228, "성문문지기1"; npcspawn name2mapid("주작성입구"), 71, 105, 2, 242, "성문문지기2"; npcspawn name2mapid("주작성입구"), 72, 105, 2, 243, "성문문지기3"; npcspawn name2mapid("주작성입구"), 73, 105, 2, 244, "성문문지기4"; npcspawn name2mapid("주작성입구"), 74, 105, 2, 245, "성문문지기5"; npcspawn name2mapid("주작성입구"), 75, 105, 2, 246, "성문문지기6"; npcspawn name2mapid("주작성입구"), 76, 105, 2, 247, "성문문지기7"; npcspawn name2mapid("주작성입구"), 77, 105, 2, 248, "성문문지기8"; }
                                else if(#clan_mapname$ == "청룡성") { npcspawn name2mapid("청룡성입구"), 70, 105, 2, 228, "성문문지기1"; npcspawn name2mapid("청룡성입구"), 71, 105, 2, 242, "성문문지기2"; npcspawn name2mapid("청룡성입구"), 72, 105, 2, 243, "성문문지기3"; npcspawn name2mapid("청룡성입구"), 73, 105, 2, 244, "성문문지기4"; npcspawn name2mapid("청룡성입구"), 74, 105, 2, 245, "성문문지기5"; npcspawn name2mapid("청룡성입구"), 75, 105, 2, 246, "성문문지기6"; npcspawn name2mapid("청룡성입구"), 76, 105, 2, 247, "성문문지기7"; npcspawn name2mapid("청룡성입구"), 77, 105, 2, 248, "성문문지기8"; }
                                else if(#clan_mapname$ == "백호성") { npcspawn name2mapid("백호성입구"), 70, 105, 2, 228, "성문문지기1"; npcspawn name2mapid("백호성입구"), 71, 105, 2, 242, "성문문지기2"; npcspawn name2mapid("백호성입구"), 72, 105, 2, 243, "성문문지기3"; npcspawn name2mapid("백호성입구"), 73, 105, 2, 244, "성문문지기4"; npcspawn name2mapid("백호성입구"), 74, 105, 2, 245, "성문문지기5"; npcspawn name2mapid("백호성입구"), 75, 105, 2, 246, "성문문지기6"; npcspawn name2mapid("백호성입구"), 76, 105, 2, 247, "성문문지기7"; npcspawn name2mapid("백호성입구"), 77, 105, 2, 248, "성문문지기8"; }
                                else if(#clan_mapname$ == "현무성") { npcspawn name2mapid("현무성입구"), 70, 105, 2, 228, "성문문지기1"; npcspawn name2mapid("현무성입구"), 71, 105, 2, 242, "성문문지기2"; npcspawn name2mapid("현무성입구"), 72, 105, 2, 243, "성문문지기3"; npcspawn name2mapid("현무성입구"), 73, 105, 2, 244, "성문문지기4"; npcspawn name2mapid("현무성입구"), 74, 105, 2, 245, "성문문지기5"; npcspawn name2mapid("현무성입구"), 75, 105, 2, 246, "성문문지기6"; npcspawn name2mapid("현무성입구"), 76, 105, 2, 247, "성문문지기7"; npcspawn name2mapid("현무성입구"), 77, 105, 2, 248, "성문문지기8"; }

                                set #clan_start, 0;
                                set #clan_timer, 0;
                                set #clan_namecheck, 0;
                                set #clan_name$, "";
                                set #clan_mapname$, "";
                                set #clan_mapcheck$, "";

                                timecount_all 2, 1;

                                deletenpc @id, mapid();
                                break;
                            default:
                                break;
                            }
                        }
                        else
                        {
                            set #clan_timer, 0;
                            set #clan_namecheck, 0;
                        }
                    }
                }
                else
                {
                    set #clan_timer, 0;
                    set #clan_namecheck, 0;
                }
            }
            else
            {
                set #clan_timer, 0;
                set #clan_namecheck, 0;
            }
        }
        else
        {
            set #clan_timer, 0;
            set #clan_namecheck, 0;
        }
    }
    else
    {
        announce 5, "[알림] 옥좌를 점령한 유저가 없기에 성은 기존문파로 돌아갑니다.";

        if(#clan_mapname$ == "주작성") { npcspawn name2mapid("주작성입구"), 70, 105, 2, 228, "성문문지기1"; npcspawn name2mapid("주작성입구"), 71, 105, 2, 242, "성문문지기2"; npcspawn name2mapid("주작성입구"), 72, 105, 2, 243, "성문문지기3"; npcspawn name2mapid("주작성입구"), 73, 105, 2, 244, "성문문지기4"; npcspawn name2mapid("주작성입구"), 74, 105, 2, 245, "성문문지기5"; npcspawn name2mapid("주작성입구"), 75, 105, 2, 246, "성문문지기6"; npcspawn name2mapid("주작성입구"), 76, 105, 2, 247, "성문문지기7"; npcspawn name2mapid("주작성입구"), 77, 105, 2, 248, "성문문지기8"; }
        else if(#clan_mapname$ == "청룡성") { npcspawn name2mapid("청룡성입구"), 70, 105, 2, 228, "성문문지기1"; npcspawn name2mapid("청룡성입구"), 71, 105, 2, 242, "성문문지기2"; npcspawn name2mapid("청룡성입구"), 72, 105, 2, 243, "성문문지기3"; npcspawn name2mapid("청룡성입구"), 73, 105, 2, 244, "성문문지기4"; npcspawn name2mapid("청룡성입구"), 74, 105, 2, 245, "성문문지기5"; npcspawn name2mapid("청룡성입구"), 75, 105, 2, 246, "성문문지기6"; npcspawn name2mapid("청룡성입구"), 76, 105, 2, 247, "성문문지기7"; npcspawn name2mapid("청룡성입구"), 77, 105, 2, 248, "성문문지기8"; }
        else if(#clan_mapname$ == "백호성") { npcspawn name2mapid("백호성입구"), 70, 105, 2, 228, "성문문지기1"; npcspawn name2mapid("백호성입구"), 71, 105, 2, 242, "성문문지기2"; npcspawn name2mapid("백호성입구"), 72, 105, 2, 243, "성문문지기3"; npcspawn name2mapid("백호성입구"), 73, 105, 2, 244, "성문문지기4"; npcspawn name2mapid("백호성입구"), 74, 105, 2, 245, "성문문지기5"; npcspawn name2mapid("백호성입구"), 75, 105, 2, 246, "성문문지기6"; npcspawn name2mapid("백호성입구"), 76, 105, 2, 247, "성문문지기7"; npcspawn name2mapid("백호성입구"), 77, 105, 2, 248, "성문문지기8"; }
        else if(#clan_mapname$ == "현무성") { npcspawn name2mapid("현무성입구"), 70, 105, 2, 228, "성문문지기1"; npcspawn name2mapid("현무성입구"), 71, 105, 2, 242, "성문문지기2"; npcspawn name2mapid("현무성입구"), 72, 105, 2, 243, "성문문지기3"; npcspawn name2mapid("현무성입구"), 73, 105, 2, 244, "성문문지기4"; npcspawn name2mapid("현무성입구"), 74, 105, 2, 245, "성문문지기5"; npcspawn name2mapid("현무성입구"), 75, 105, 2, 246, "성문문지기6"; npcspawn name2mapid("현무성입구"), 76, 105, 2, 247, "성문문지기7"; npcspawn name2mapid("현무성입구"), 77, 105, 2, 248, "성문문지기8"; }

        set #clan_start, 0;
        set #clan_timer, 0;
        set #clan_namecheck, 0;
        set #clan_name$, "";
        set #clan_mapname$, "";
        set #clan_mapcheck$, "";

        deletenpc @id, mapid();
    }
}