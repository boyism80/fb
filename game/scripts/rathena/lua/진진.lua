{
    set @sd, mysd();
    set @id, myid();

    if($jinjin_dosirak == 0)
    {
        if(countitem(name2itemid("초보도시락")) > 0)
        {
            set @pos, list2(@list, 0, 0, "잠깐만! 혹시 여유가 있다면 그 도시락을 나에게 주시지 않겠소?", "네, 그러지요.", "주기 싫은데요.");
            if(@pos == 0) end;
            else if(@list != 1) end;

            delitem @sd, name2itemid("초보도시락"), 1;
            message @sd, 3, "초보도시락 주었습니다";
            additem @sd, name2itemid("선장의일기1"), 1;
            addlegend @sd, 32, 7, 0, 1, "진백랑의 부탁을 들어주자. [2/6]";
            set $jinjin_dosirak, 1;
            set @pos, dialog(0, 0, 1, "고맙소! 일이 급해서 도시락 만들 여유가 없었거든! 자, 이거라도 받으시오!");
            end;
        }
    }
    set @pos, dialog(0, 0, 1, "일을 가야하는데 도시락을 깜빡 잊었네. 이를 어쩐담?! 이제와서 만들 시간도 없는데. 누가 초보 도시락이라도 팔지 않으려나.");
}