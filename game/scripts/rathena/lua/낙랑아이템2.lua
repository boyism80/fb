{
    set @sd, mysd();
    image 1, 19, 0;
    if($nakrang2 == 1)
    {
        if($nakrang2_count == 2)
        {
            set @pos, dialog(0, 0, 1, "잘 하셨습니다!! 상으로\\n목도를 드릴게요!!");
            if(@pos == 0) end;

            delitem @sd, name2itemid("낙랑의두루마리2"), 1;
            message @sd, 3, "낙랑의두루마리2 주었습니다";
            additem @sd, name2itemid("낙랑의두루마리3"), 1;
            additem @sd, name2itemid("초심자의목도"), 1;

            if(get_level(@sd) < 5) { giveexp @sd, 50; set @pos, dialog(0, 0, 1, "<보상>\\n'목도',\\n'낙랑의두루마리3'을 얻다!!!\\n경험치 50 상승!!"); }
            else { set @pos, dialog(0, 1, 1, "<보상>\\n'목도',\\n'낙랑의두루마리3'을 얻다!!!"); }

            set $nakrang2_count, 0; set $nakrang2, 0;
        }
        else { set @pos, dialog(1, 0, 1, "실패하셨군요!! 좀 더 연습한 후 다시\\n시도해보세요.."); }
    }
    else            
    {
COS001:
        set @pos, dialog(1, 0, 1, "<임무>\\n 도톨 2개 사용\\n\\n<내용>\\n 첫번째 임무는 도톨을 사용하는 것!! 도톨\\n2개를 드렸으니, 두루마리를 사용하듯이 이\\n도톨도 사용해보세요!! 시간은 5초 드립니다!!\\n\\n<보상>\\n 목도, 경험치 50 (레벨5미만)");
        if(@pos == 0) end;

        set @pos, dialog(1, 1, 1, "<힌트>\\n 도톨을 사용하기 위해서는 오른쪽의\\n'소지품(단축키i)'을 클릭하신 다음 제가 드린\\n도톨을 더블클릭하면 됩니다. 단축키 'u(use)'를\\n누른 다음, 도톨이 있는 슬롯의 알파벳을\\n누르셔도 됩니다. 'b'슬롯에 있다면, 'u'를\\n누르신 후 'b'를 누르시면 됩니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set $nakrang2, 1;
    }
}