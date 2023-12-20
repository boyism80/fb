{
    image 0, 0, 0;
    set @sd, mysd();
    set @id, myid();
    if(get_totem(@sd) == 2 && countitem(name2itemid("승급의서장")) > 0)
    {
        if(countitem(name2itemid("구름의옥")) > 0) { delitem @sd, name2itemid("구름의옥"), 1; message @sd, 3, "구름의옥 주었습니다"; additem @sd, name2itemid("동문열쇠"), 1; }
        if(countitem(name2itemid("물의옥")) > 0) { delitem @sd, name2itemid("물의옥"), 1; message @sd, 3, "물의옥 주었습니다"; additem @sd, name2itemid("서문열쇠"), 1; }
        if(countitem(name2itemid("불의옥")) > 0) { delitem @sd, name2itemid("불의옥"), 1; message @sd, 3, "불의옥 주었습니다"; additem @sd, name2itemid("고대마법서'하"), 1; }
        if(countitem(name2itemid("산의옥")) > 0) { delitem @sd, name2itemid("산의옥"), 1; message @sd, 3, "산의옥 주었습니다"; additem @sd, name2itemid("고대마법서'상"), 1; }
        if(countitem(name2itemid("고대마법서'상")) > 0 && countitem(name2itemid("고대마법서'하")) > 0) { set $thirdclass, 2; set @pos, dialog(0, 0, 1, "이제 모든 조건이 가춰졌으니 천성현에게 가져가세요."); }
    }
}