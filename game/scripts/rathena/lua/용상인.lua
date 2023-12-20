{
    image 0, 87, 3;
    set @sd, mysd();
    set @id, myid();
    set @pos, list2(@ret, 0, 0, "무슨일로 날 찾으시나?", "용의비늘이 무엇인가요?", "용의비늘을 가지고 왔어요..");
    if(@ret == 1)
    {
POS00000:
        set @pos, dialog(0, 0, 1, "안녕하신가? 그래.. 용의비늘에 대해서 궁금해서 왔다고?");
        if(@pos == 0) { end; }
POS00001:
        set @pos, dialog(0, 1, 1, "허허... 용의비늘에는 여러가지가 있지.. 수룡의비늘.. 화룡의비늘.. 지룡의비늘..등등..");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS0000; }
POS00002:
        set @pos, dialog(0, 1, 1, "그 중, 옥황상제님의 병을 치료하기 위해서는 수룡,화룡의비늘이 필요하지.");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS0001; }
POS00003:
        set @pos, dialog(0, 1, 1, "하지만, 일반인들은 무엇이 수룡의비늘이고 무엇이 화룡의비늘인지 잘 구분을 못한다네..");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS0002; }
POS00004:
        set @pos, dialog(0, 1, 1, "하하하.. 이건 내 자랑이 아니고... 나는 그것을 구분할 수 있거든...");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS0003; }
POS00005:
        set @pos, dialog(0, 1, 1, "그러니.. 용의비늘을 구하거든.. 나에게 가져 오라고.. 내가 감정을 해 줄테니..");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS0004; }
        set @pos, list2(@ret, 0, 1, "용이 어디서 나오는지 알고 싶은가?", "알고 싶습니다", "아니요");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS00005; }
        if(@ret == 1)
        {
POS0000:
            set @pos, dialog(0, 0, 1, "호랑이를 잡으려면 호랑이굴로 들어가듯 용을 잡으려면 용있는 곳으로 가야하지 않겠나!!");
            if(@pos == 0) { end; }
POS0001:
            set @pos, dialog(0, 1, 1, "근데, 그 용이 사는곳이란게 애매모호 하거든?");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0000; }
POS0002:
            set @pos, dialog(0, 1, 1, "특히 수룡과 화룡은 기타 다른 용들이 사는 곳이랑 틀리다는거야..");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0001; }
POS0003:
            set @pos, dialog(0, 1, 1, "어떤이는 극지방 구석에 있다고 하고.. 어떤이는 국내성 어딘가에 살고 있다 하고..");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0002; }
POS0004:
            set @pos, dialog(0, 1, 1, "글쎄.. 이러한 말들이 많은데.. 아직 용을 직접 본 사람이 없다네.. 물론 나도 못 봤고..");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0003; }
POS0005:
            set @pos, dialog(0, 1, 1, "우리 집안이 대대로 용에 대해서 많은 연구를 해 왔는데 말이야.. 가보처럼 전해내려오는 '용문서'를 보면..");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0004; }
POS0006:
            set @pos, dialog(0, 1, 1, "수룡과 화룡에 대한 이야기가 있다네.. 그곳에 보면 이 용들은 밖으로 나가길 싫어하고");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0005; }
POS0007:
            set @pos, dialog(0, 1, 1, "아무도 만나길 원하지 않다고하지.. 그래서 호랑이의 기운을 받은자들에게 자신들을 지켜 달라고 하고 있지..");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0006; }
POS0008:
            set @pos, dialog(0, 1, 1, "만약, 자신들을 보는자가 있다면 모두 다 다시는 세상으로 못나오게 한다는 말과 함께 말일쎄..");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0007; }
POS0009:
            set @pos, dialog(0, 1, 1, "그래서 지금 그들을 호랑이들이 지키고 있다고 하더군... '용문서'에 나온 것은 여기까지고..");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0008; }
POS0010:
            set @pos, dialog(0, 1, 1, "이제부터는 자네가 풀어 나가야 할 걸쎄... 그리고 그 용들은 아주 난폭하다고 하니 조심하시게..");
            if(@pos == 0) { end; }
            else if(@pos == -1) { goto POS0009; }
        }
        else if(@ret == 2)
        {
            set @pos, dialog(0, 0, 1, "그래? 그럼..자네 마음대로 하시게! 마음대로... 흥흥!!");
            if(@pos == 0) { end; }
        }
    }
    else if(@ret == 2)
    {
        set @pos, dialog(0, 0, 1, "오호.. 용의비늘을 가지고 왔다고? 어디 보세나...");
        if(@pos == 0) { end; }
        if(countitem(name2itemid("용의비늘")) >= 1)
        {
            set @pos, dialog(0, 0, 1, "오오!! 내 잠시 책을 보면서 확인하고 오겠네 잠시만 기다리게나.");
            if(@pos == 0) { end; }

            set @rand, rand(1, 100);
            set @rate, rate(1, 100);
            if(@rand <= 30 && @rate <= 30)
            {
                delitem @sd, name2itemid("용의비늘"), 1;
                message @sd, 3, "용의비늘 주었습니다";
                additem @sd, name2itemid("수룡의비늘"), 1;
                set @pos, dialog(0, 0, 1, "이것은!! 수룡의비늘 이구려.. 주기 싫지만 어쩔수 없지 행..");
                if(@pos == 0) { end; }
            }
            else if(@rand >= 70 && @rate >= 70)
            {
                delitem @sd, name2itemid("용의비늘"), 1;
                message @sd, 3, "용의비늘 주었습니다";
                additem @sd, name2itemid("화룡의비늘"), 1;
                set @pos, dialog(0, 0, 1, "이것은!! 화룡의비늘 이구려.. 주기 싫지만 어쩔수 없지 행..");
                end;
            }
            else
            {
                delitem @sd, name2itemid("용의비늘"), 1;
                message @sd, 3, "용의비늘 주었습니다";
                set @pos, dialog(0, 0, 1, "에잉.. 이런 이건 그냥 비늘이아니라 개 털뭉치가 이렇게 뭉쳐서 보인거자나!!!");
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 1, "아니! 이보게.. 나한테 거짓말을 해도 되는가? 없지 않은가. 썩 물러 가게!!!");
            if(@pos == 0) { end; }
        }
    }
}