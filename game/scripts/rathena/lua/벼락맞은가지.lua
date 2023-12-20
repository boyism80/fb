{
    set @sd, mysd();
    set @id, myid();

    image 0, 4, 4;

    if(get_state(@sd) != 1)
    {
        if(rand(1, 100) <= 80)
        {
            if($thunder_tree == 0)
            {
COS001:
                set @pos, dialog(0, 0, 1, "어서오시게, 나는 이 오동나무의 수호신일세.\\n\\n보시는 바와 같이 이 나무는 벼락을 맞았다네. 가슴아픈 일이지...");
                if(@pos == 0) end;
COS002:
                set @pos, dialog(0, 1, 1, "그 대신 벼락을 맞은 나무의 가지는 특별한 쓸모가 있다네...\\n\\n\\n혹시 자네가 이 나무를 소생시킬 수 있는 회복의 샘물을 구해온다면, 나뭇가지 하나 정도는 내어줌세.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS001;
COS003:
                set @pos, dialog(0, 1, 1, "소생천이 있다면, 그 물을 떠오면 될걸세... 행운을 비네...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS002;

                set $thunder_tree, 1;
                addlegend @sd, 2, 7, 0, 4, "회복의 샘물을 구하러....";
            }
            else if($thunder_tree == 1)
            {
                if(countitem(name2itemid("회복의샘물")) > 0)
                {
                    set @pos, dialog(0, 0, 1, "고맙네. 아주 시원한 물일세...\\n약속한대로 여기 가지 하나를 내어줌세. 요긴하게 쓰게나...");
                    if(@pos == 0) end;

                    delitem @sd, name2itemid("회복의샘물"), 1;
                    message @sd, 3, "회복의샘물 주었습니다.";
                    additem @sd, name2itemid("벼락나뭇가지"), 1;
                    set $thunder_tree, 2;
                    addlegend @sd, 2, 7, 0, 4, "회복의 샘물을 수호신에게 가져다 주었다.";
                }
            }
            else end;
        }
    }
}