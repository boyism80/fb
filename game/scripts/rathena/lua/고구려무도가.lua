{
    set @sd, mysd();
    set @id, myid();

    if($handmagic == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "난 고구려의 무도가요. 얼마전부터 중국의 소림사에 대해 관심을 가지게 됐소. 정말 아무도 그 위치나 규모에 대해서 아는 사람이 없었소.");
        if(@pos == 0) end;
COS002:
        set @pos, dialog(0, 1, 1, "내 얼마전에 듣기로는 만리장성 부근에 출중한 무술 기량을 가진 사람들이 보인다 하던데. 그들은 모두 승려차림이었다하오.\\n\\n그들의 절기를 꼭 한번 실제로 보고 배우고싶은데 마땅한 기회가 없구려.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
COS003:
        set @pos, list2(@list, 0, 0, "당신이 날 대신해 중국 만리장성에가서 그들에게 절기를 익혀와 보여주지 않겠소?", "중국가는길이니 그렇게하겠소.", "난 무술에 별로 관심이 없소.");
        if(@pos == 0) end;
        else if(@list != 1) end;

        set $handmagic, 1;
        addlegend @sd, 26, 7, 0, 1, "고구려무도가의 부탁을 들어주자.";
        set @pos, dialog(0, 0, 0, "고맙소! 자세한것은 모르지만 요즘 만리장성에 승려차림의 무술고수가 보인다는 소문이 있다오. 부탁하오.");
        end;
    }
    else
    {
        if($handmagic_check == 0)
        {
            if(checkspell(name2spellid("백열장")) == 52)
            {
                set @pos, dialog(0, 0, 0, "아직 소림사의 절기를 배워오지 못하신 것 같군요.");
                end;
            }
            else
            {
COS004:
                set @pos, dialog(0, 0, 1, "그래.. 소림사의 절기는 배워오셨소? 오오.. 그 절기의 이름을 백열장이라고 하는군..");
                if(@pos == 0) end;
COS005:
                set @pos, dialog(0, 1, 1, "음.. 그렇군. 대충 기술은 이해했소. 나도 무도가라 한번 보면 대략적인 형태는 이해할수 있다오. 이후엔 수련을 해서 더 높은 경지로 갈 수 있도록 해야겠지.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS004;
COS006:
                set @pos, dialog(0, 1, 1, "이거 고마워서.. 내 뭐라도 줘야할 터인데.. 그렇지. 나의 사부님이 전해주신 손목보호대를 드리지. 뭐 아주 간단한 손목보호대라 다른건 없고, 단단한거 하난 알아주는 물건이오.\\n\\n그럼 고맙소.. 잘 돌아가시게..");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS005;

                set $handmagic_check, 1;
                additem @sd, name2itemid("청동손목보호대"), 1;
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 1, "정말 고맙소. 당신 덕분에 나도 소림사의 절기를 더욱더 배워지고 싶구만.");
            end;
        }
    }
}