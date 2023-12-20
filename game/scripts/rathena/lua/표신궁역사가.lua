{
    set @sd, mysd();
    set @id, myid();

    if(get_joblevel(@sd) < 1)
    {
        set @pos, dialog(0, 0, 1, "표신궁의 역사란 참 미묘하지..");
        end;
    }

COS001:
    set @pos, list2(@list, 0, 0, "......", "표신궁에 관한 이야기를 한다.", "표신궁고문서를 판다.");
    if(@pos == 0) end;
    else if(@list == 1)
    {
        if($geumgang_rock == 0)
        {
COS002:
            set @pos, dialog(0, 0, 1, "헉! 누...누...... 누구시오!");
            if(@pos == 0) end;
COS003:
            set @pos, list2(@list_2, 0, 1, "...사람이였구려. 이거 실례했소이다. 여긴 인적이 드문 곳이라 아무도 오질 않지요. 혹시 귀신은 아닌가 해서 그만 놀라고 말았소이다. 허허.", "귀신? 귀신이 나옵니까?", "놀래켜서 죄송합니다. 전 이만...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
            else if(@list_2 == 1)
            {
COS004:
                set @pos, dialog(0, 0, 1, "모르셨소? 내 간단히 설명해드리지. 저 멀리 표신궁이 보이지요? 옛날 수많은 사람들을 학살한 대장군의 성이지. 그 대장군이 얼마나 포악했는가 하면, 대장군이 죽은 지금도 사람들이 이 성 근처에 가까지 오지 않으려 하는 것만 봐도 알 수 있지 않소?");
                if(@pos == 0) end;
COS005:
                set @pos, dialog(0, 1, 1, "이런 곳이라면 어디에 언제 귀신이 나타나도 이상하지 않지... 난 연구를 해야 하니 이 곳에 있을 수 밖에 없지만.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS004;
COS006:
                set @pos, dialog(0, 1, 1, "아, 이거 내 소개가 늦었군. 나는 저 표신궁에 얽힌 수수께끼의 역사들을 연구하는 역사가라오. 대장군이 왜 사람들을 죽였는가, 폐허가 되다시피한 표신궁 주변과는 달리, 표신궁은 어떻게 아직도 건재할 수 있는 것인가... 이런 것들을 연구하고 있지.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS005;
COS007:
                set @pos, list2(@list_3, 0, 1, "음... 이런 곳을 돌아다니는 것을 보니 당신도 보통 사람은 아닌 것 같소만... 혹시 당신, 표신궁을 탐험할 생각으로 온거요?", "예, 표신궁을 탐험하려고 왔습니다.", "어휴, 위험할 것 같아서 안되겠어요.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS006;
                else if(@list_3 == 1)
                {
COS008:
                    set @pos, dialog(0, 0, 1, "이거 반갑구려. 모험가를 만나는 게 얼마만인지... 실은 이 전에도 많은 모험가들이 표신궁에 들어갔었는데, 대부분 돌아오지 않았고 살아서 돌아온 몇몇 사람들은 입구에서 얼마 가지도 못하고 돌아왔지.");
                    if(@pos == 0) end;
COS009:
                    set @pos, dialog(0, 1, 1, "대체 표신궁 깊은 곳에는 무엇이 있는지...... 아무튼 그래서 말인데, 당신이 표신궁에 들어간다면 내 부탁 하나만 들어주지 않겠소? 사례는 후하게 드리겠소이다.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS008;
COS010:
                    set @pos, list2(@list_4, 0, 1, "표신궁에는 여러가지 고문서들이 있을 거요. 이 고문서들을 내게 가져와 주면 내용을 해석해주는 것은 물론이고, 자료를 제공한 대가 또한 지불하겠소, 어떠시오?", "예, 기꺼이 도와드리겠습니다.", "음, 그렇게 위험한 곳에는 가고 싶지 않군요.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS009;
                    else if(@list_4 == 1)
                    {
COS011:
                        set @pos, dialog(0, 0, 1, "고맙소! 당신이라면 왠지 꼭 성공할 것 같은 기분이 드는군, 행운을 빌겠소.");
                        if(@pos == 0) end;
COS012:
                        set @pos, dialog(0, 1, 1, "이것은 내 작은 성의요. 착수금조로 받아주시오.");
                        if(@pos == 0) end;
                        else if(@pos == -1) goto COS011;

                        addlegend @sd, 30, 7, 0, 1, "표신궁고문서1을 구하자.";
                        set $geumgang_rock, 1;
                        set_money @sd, get_money(@sd)+5000;
                        updatestatus @sd, 2;
                        end;
                    }
                    else if(@list_4 == 2)
                    {
                        set @pos, dialog(0, 0, 0, "하긴.. 보통 위험한 일이 아니니 다음에 기회가 되면 도와주시게.");
                        end;
                    }
                    else end;
                }
                else if(@list_3 == 2)
                {
                    set @pos, dialog(0, 0, 1, "당신도 그렇게 생각하시오? 그럼 부디 몸조심하시게.");
                    end;
                }
                else end;
            }
            else if(@list_2 == 2)
            {
                set @pos, dialog(0, 0, 1, "아니네. 허허. 그럼 잘가보게.");
                end;
            }
        }
        if($geumgang_rock == 1)
        {
COS013:
            set @pos, dialog(0, 0, 1, "오, 살아 돌아왔구려! 반갑소. 정말 반갑소. 잘 돌아오셨소이다.");
            if(@pos == 0) end;
COS014:
            set @pos, list2(@list_5, 0, 1, "그래, 표신궁에서 고문서를 찾아 오셨소?", "동장군의 원령을 물리치고 고문서를 가져왔습니다.", "아뇨, 아직...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS013;
            else if(@list_5 == 1)
            {
                if(countitem(name2itemid("표신궁고문서1")) == 0)
                {
                    set @pos, dialog(0, 0, 0, "아직 동장군을 쓰러뜨리지 못한건가? 하긴.. 동장군을 쓰러뜨리는 것은 무리일수도 있지.");
                    end;
                }
                else
                {
COS015:
                    set @pos, dialog(0, 0, 1, "동장군? ...오오! 과연... 역시 내 눈은 틀리지 않았구려. 동장군이라면 대장군의 충실한 수하인 오방장군 중 하나요. 그도 귀신이 되어 표신궁에 떠돌고 있는 모양이군...");
                    if(@pos == 0) end;
COS016:
                    set @pos, dialog(0, 1, 1, "가까스로 살아나온 사람들에게서도 그런 이야기는 듣지 못했소이다. 하물며 동장군의 원령을 쓰러뜨리기까지 하다니! 오래간만에 대단한 모험가를 만난 것 같구려. 허허허...");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS015;
COS017:
                    set @pos, dialog(0, 1, 1, "그럼 잠시만 기다려 보시오. 이 문서를 냉큼 해독해 보일 테니...");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS016;
COS018:
                    delitem @sd, name2itemid("표신궁고문서1"), 1;
                    message @sd, 3, "표신궁고문서1 주었습니다";
                    additem @sd, name2itemid("대장군의일기1"), 1;
                    set $geumgang_rock, 2;
                    set_money @sd, get_money(@sd)+10000;
                    updatestatus @sd, 2;
                    set @pos, dialog(0, 0, 1, "......음, 다 됐소. 여기 번역한 문서가 있으니 한번 읽어나 보시오. 보아하니 이것은 먼 옜날 이 땅을 통일했던 유명한 대장군의 일기인 것 같구려. 아까 이야기했던 오방장군도 이 대장군의 수하들이라오.");
                    if(@pos == 0) end;
COS019:
                    set @pos, dialog(0, 1, 1, "그 대장군은 원래 선량한 사람이었으나 천하를 통일하고 권력을 잡은 후 수많은 사람을 학살한 폭군이 되고 말았다 하오. 권력이라는게 다 그런 법이긴 하지만...");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS018;
COS020:
                    set @pos, dialog(0, 1, 1, "다 읽고 나서 말씀해 주시오. 부탁드릴 것도 있고 하니... 아, 그리고 이건 약속한 사례요. 약소하지만 받아 두시오. 앞으로 내 연구를 더 많이 도와주면 줄수록 더 많은 사례를 하겠소이다.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS019;

                    end;
                }
            }
            else if(@list_5 == 2)
            {
                set @pos, dialog(0, 0, 0, "그렇군. 너무 서두르지는 않아도 되니 천천히 하시게나.");
                end;
            }
            else end;
        }
        if($geumgang_rock == 2)
        {
            if($imperator_book_1 != 1)
            {
                set @pos, dialog(0, 0, 0, "아직 대장군의일기1을 읽어보시지 않은것 같군요.");
                end;
            }
COS021:
            set @pos, dialog(0, 0, 1, "다 읽어보셨소? 흠, 어떠시오? 고통받는 백성들을 위해 천하통일을 결심한 대장군의 웅대한 포부가 느껴지는 것 같지 않소?");
            if(@pos == 0) end;
COS022:
            set @pos, dialog(0, 1, 1, "일기의 다음이 궁금해지는군. 혹시 표신궁에 또 갈 일이 있거든 이 일기의 다음편을 찾아봐 주셨으면 하오. 물론 사례는 하겠소.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS021;
COS023:
            set @pos, dialog(0, 1, 1, "동장군의 원령이 이 일기를 가지고 있었다고 하면, 다음 일기는 아마 오방장군 중 두번째 장군인 서장군의 원령이 가지고 있지 않을까 하는데...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS022;

            set $geumgang_rock, 3;
            addlegend @sd, 30, 7, 0, 1, "표신궁고문서2를 구하자.";
            end;
        }
        if($geumgang_rock == 3)
        {
            if(countitem(name2itemid("표신궁고문서2")) == 0)
            {
                set @pos, dialog(0, 0, 0, "아직 오방장군중 하나인 서장군을 물리치지 못하신것 같군. 서장군을 물리친 후 일기를 구해와주시오.");
                end;
            }
            else
            {
COS024:
                set @pos, dialog(0, 0, 1, "오오, 두번째 고문서를 찾아오셨구려. 수고하셨소! 잠시만 기다리시오. 이번 일기는 어떤 내용일런지...");
                if(@pos == 0) end;
COS025:
                set @pos, dialog(0, 1, 1, "......음...호오...... 과연, 과연... 그랬군. 허허허...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS024;
COS026:
                delitem @sd, name2itemid("표신궁고문서2"), 1;
                message @sd, 3, "표신궁고문서2 주었습니다";
                additem @sd, name2itemid("대장군의일기2"), 1;
                set $geumgang_rock, 4;
                set_money @sd, get_money(@sd)+20000;
                updatestatus @sd, 2;
                addlegend @sd, 30, 7, 0, 1, "표신궁고문서3을 구하자.";
                set @pos, dialog(0, 0, 1, "...다 됐소. 한번 읽어 보시오. 역시 이 두번째 고문서는 대장군의 두번째 일기였소. 여기 약속한 사례도 받아가시구려.");
                if(@pos == 0) end;
COS027:
                set @pos, dialog(0, 1, 1, "다 읽고 나면 말씀해 주시오. 역시 또 부탁할 것이 있을 것 같구려. 허허...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS026;
                end;
            }
        }
        if($geumgang_rock == 4)
        {
            if($imperator_book_1 != 2)
            {
                set @pos, dialog(0, 0, 0, "아직 대장군의일기2를 읽어보시지 않은것 같군요.");
                end;
            }
            if(countitem(name2itemid("표신궁고문서3")) == 0)
            {
                set @pos, dialog(0, 0, 0, "아직 표신궁고문서3을 구하지 못했군. 남장군을 쓰러뜨리고 고문서를 얻어와주게.");
                end;
            }
            else
            {
COS028:
                set @pos, dialog(0, 0, 1, "세번째 고문서를 찾아오셨구려! 정말 수고하셨소! 당신은 여태껏 여기에 찾아온 사람들 중 가장 대단한 사람이오. 정말 놀랍소.");
                if(@pos == 0) end;
COS029:
                set @pos, dialog(0, 1, 1, "잠시만 기다려 주시오. 금방 번역해 보이겠소이다.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS028;
COS030:
                set @pos, dialog(0, 1, 1, "......음...그런 일이... 그 때는 그랬었군, 과연... 으음!");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS029;
COS031:
                delitem @sd, name2itemid("표신궁고문서3"), 1;
                message @sd, 3, "표신궁고문서3 주었습니다";
                additem @sd, name2itemid("대장군의일기3"), 1;
                addlegend @sd, 30, 7, 0, 1, "표신궁고문서4를 구하자.";
                set $geumgang_rock, 5;
                set_money @sd, get_money(@sd)+30000;
                updatestatus @sd, 2;
                set @pos, dialog(0, 0, 1, "...으음... 다 됐소. 예상한 대로 세번째 고문서는 대장군의 세번째 일기였소. 한번 읽어 보시오. 대장군이 전투에서 크게 패하고 도망치던 때에 쓴 일기인 것 같소.");
                if(@pos == 0) end;
COS032:
                set @pos, dialog(0, 1, 1, "다 읽고 나면 말씀해 주시오. 음...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS031;

                end;
            }
        }
        if($geumgang_rock == 5)
        {
            if($imperator_book_1 != 3)
            {
                set @pos, dialog(0, 0, 0, "아직 대장군의일기3을 읽어보시지 않은것 같군요.");
                end;
            }
            if(countitem(name2itemid("표신궁고문서4")) == 0)
            {
                set @pos, dialog(0, 0, 0, "아직 표신궁고문서4를 구하지 못했군. 북장군을 쓰러뜨리고 고문서를 얻어와주게.");
                end;
            }
            else
            {
COS033:
                set @pos, dialog(0, 0, 1, "네번째 고문서를 찾아오셨구려! 수고하셨소. 얼마나 기다렸는지 모르오. 자자, 어서 고문서를 보여주시오. 금방 해석해 드리리다.");
                if(@pos == 0) end;
COS034:
                set @pos, dialog(0, 1, 1, "......음...쯧쯧쯧...그랬군, 설마했더니...으음......");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS033;
COS035:
                delitem @sd, name2itemid("표신궁고문서4"), 1;
                message @sd, 3, "표신궁고문서4 주었습니다";
                additem @sd, name2itemid("대장군의일기4"), 1;
                addlegend @sd, 30, 7, 0, 1, "표신궁고문서5를 구하자.";
                set $geumgang_rock, 6;
                set_money @sd, get_money(@sd)+40000;
                updatestatus @sd, 2;
                set @pos, dialog(0, 0, 1, "......다 됐소. 한번 읽어 보시오. 매번 고맙소. 여기 사례도 받아가구려.");
                if(@pos == 0) end;
COS036:
                set @pos, dialog(0, 1, 1, "다 읽고 나면 말씀해 주시오...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS035;

                end;
            }
        }
        if($geumgang_rock == 6)
        {
            if($imperator_book_1 != 4)
            {
                set @pos, dialog(0, 0, 0, "아직 대장군의일기4를 읽어보시지 않은것 같군요.");
                end;
            }
            if(countitem(name2itemid("표신궁고문서5")) == 0)
            {
                set @pos, dialog(0, 0, 0, "아직 표신궁고문서5를 구하지 못했군. 중앙장군을 쓰러뜨리고 고문서를 얻어와주게.");
                end;
            }
            else
            {
COS037:
                set @pos, dialog(0, 0, 1, "오오, 다섯번째의 표신궁고문서를 찾아내셨구려! 수고하셨소. 그럼 이 고문서를 해독해 드릴 테니 잠시만 기다려 주시오.");
                if(@pos == 0) end;
COS038:
                set @pos, dialog(0, 1, 1, "...으음....음....!");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS037;
COS039:
                set @pos, dialog(0, 1, 1, "이런! ......호오...이런 일이 있었군......");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS038;
COS040:
                set @pos, dialog(0, 1, 1, "역시 그런 일이 있었기에... 그렇군, 과연...! 하지만 이게 정말일까? 으음...!!");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS039;
COS041:
                delitem @sd, name2itemid("표신궁고문서5"), 1;
                message @sd, 3, "표신궁고문서5 주었습니다";
                additem @sd, name2itemid("대장군의일기5"), 1;
                set $geumgang_rock, 7;
                set_money @sd, get_money(@sd)+60000;
                updatestatus @sd, 2;
                set @pos, dialog(0, 0, 1, "굉장히 흥미로운 내용이 쓰여있소. 하지만 너무 신비한 내용이라 믿어지지 않는구려. 어찌 되었건 한 번 읽어 보시오.");
                end;
            }
        }
        if($geumgang_rock == 7)
        {
            if($imperator_book_1 != 5)
            {
                set @pos, dialog(0, 0, 0, "아직 대장군의일기5를 읽어보시지 않은것 같군요.");
                end;
            }
COS042:
            set @pos, dialog(0, 0, 1, "다 읽으셨소? 보셨다시피... 대장군은... 악마와 계약한 것 인지도 모르겠소. 허, 이걸 믿어야 하나? 아무튼 그게 사실이라면 대장군이 그렇게 많은 사람들을 학살한 이유도 밝혀지는 셈이지...");
            if(@pos == 0) end;
COS043:
            set @pos, dialog(0, 1, 1, "하지만 유감스럽게도 그것이 사실인지를 파악할 수 있는 다음 장은 찢겨져 있더군.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS042;
COS044:
            set @pos, dialog(0, 1, 1, "수고스럽겠지만 표신궁을 샅샅이 뒤져봐 주시오, 진실을 밝히기 위해선 대장군의 일기 다음 장... 그러니까 여섯번째의 표신궁고문서가 꼭 필요하지. 그것을 찾아와 주셨으면 하오.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS043;
COS045:
            set @pos, dialog(0, 1, 1, "오방장군은 모두 물리쳤으니... 아마 여섯번째 장은 표신궁어딘가에 숨겨져 있을 것이오. 구석구석까지 샅샅이 살펴봐 주셨으면 하오.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS044;

            set $geumgang_rock, 8;

            addlegend @sd, 30, 7, 0, 1, "표신궁고문서6을 구하자.";
            spellaether @sd, name2spellid("표신궁고문서6찾기"), 180;
            end;
        }
        if($geumgang_rock == 8)
        {
            if(countitem(name2itemid("표신궁고문서6")) == 0)
            {
                set @pos, dialog(0, 0, 0, "여섯번째 고문서는 아마 표신궁 어딘가에 숨겨져 있는듯 하오. 구석구석 찾아봐 주시게.");
                end;
            }
COS046:
            set @pos, dialog(0, 0, 1, "오오, 숨겨진 여섯번째의 고문서를 찾아냈구려! 전에도 말했지만 당신 정말 대단한 사람이군. 수고하셨소. 그럼 이 고문서를 해독할테니 잠시만 기다려 주시오.");
            if(@pos == 0) end;
COS047:
            set @pos, dialog(0, 1, 1, "......으음......?");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS046;
COS048:
            set @pos, dialog(0, 1, 1, "음... 이런! ......호오...이럴 수가......");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS047;
COS049:
            set @pos, dialog(0, 1, 1, "음... 과연, 그렇군! 오오, 신이시여...어찌 이런 일이...!");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS048;
COS050:
            delitem @sd, name2itemid("표신궁고문서6"), 1;
            message @sd, 3, "표신궁고문서6 주었습니다";
            additem @sd, name2itemid("대장군의일기6"), 1;
            addlegend @sd, 30, 7, 0, 1, "표신궁고문서를 모두 구하다.";
            set $geumgang_rock, 9;
            set_money @sd, get_money(@sd)+60000;
            updatestatus @sd, 2;
            set @pos, dialog(0, 0, 1, "이런 무서운 일이... 으음... 이게 정말이라면 우리는 엄청난 사실을 알아낸 것일지도 모르겠소. 일단 읽어나 보시오. 여기 있소...");
            if(@pos == 0) end;
COS051:
            set @pos, dialog(0, 1, 1, "다 읽고 나면 이야기해 주시오. 음......");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS050;

            end;
        }
        if($geumgang_rock == 9)
        {
            if($imperator_book_1 != 6)
            {
                set @pos, dialog(0, 0, 0, "아직 대장군의일기6을 읽어보시지 않은것 같군요.");
                end;
            }
COS052:
            set @pos, dialog(0, 0, 1, "여섯번째 일기... 그러니까 마지막 일기인 것 같소. 아무튼 다 읽어보셨소?");
            if(@pos == 0) end;
COS053:
            set @pos, dialog(0, 1, 1, "대장군이 천하통일 후 왜 흉악한 폭군이 되었는지... 왜 그렇게 죄없는 사람들을 학살했는지... 주인 없는 표신궁이 어떻게 아직도 건재할 수 있는지... 이제야 그 이유를 알 수 있게 됐구려...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS052;
COS054:
            set @pos, dialog(0, 1, 1, "......");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS053;
COS055:
            set @pos, dialog(0, 1, 1, "......혹시!?");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS054;
COS056:
            set @pos, dialog(0, 1, 1, "이...이런!!! 큰일이오. 어쩌면... 어쩌면... 내 예상이 맞다면 정말 큰일이 일어날 것이오. 시간이 없소. 다신이 가지고 있는 일기장을 전부 조합해 보시오! 어서!");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS055;
COS057:
            set $geumgang_rock, 10;
            additem @sd, name2itemid("대장군의일기겉표지"), 1;
            addlegend @sd, 30, 7, 0, 1, "일기장을 전부 조합하자.";
            set @pos, dialog(0, 0, 1, "여기 책 겉표지도 있으니 같이 조합하시면 될 거요!");
            end;
        }
        if($geumgang_rock == 10)
        {
            if(countitem(name2itemid("대장군의일기통합본")) == 0)
            {
                set @pos, dialog(0, 0, 0, "어서 조합해보시게. 겉표지와 같이 조합하면 될걸세!");
                end;
            }
COS058:
            set @pos, dialog(0, 0, 1, "...이런 일이... 결국 이런 일이...!");
            if(@pos == 0) end;
COS059:
            set @pos, dialog(0, 1, 1, "...내가 예전에 조사했던 고문서가 있소. 어떤 날짜와 알 수 없는 숫자들이 적혀 있는 문서였는데...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS058;
COS060:
            set @pos, dialog(0, 1, 1, "이 일기들이 쓰여진 시기와 대조해 보니, 그것은... 대장군이 부활하게 될 날짜가 적힌 고문서였던 거요!");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS059;
COS061:
            set @pos, list2(@list_6, 0, 1, "게다가, 그 날짜가 이제 얼마 남지 않았구려. 으으음......", "그럴 수가...!");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS060;
            else if(@list_6 == 1)
            {
COS062:
                set @pos, dialog(0, 0, 1, "대장군은 전에 말했듯이 천하통일 후 엄청난 폭군이 되었다오. 이 대장군이 영원한 생명을 얻어 다시 이 땅에 나타난다면 천하는 피바다가 될 거요... 이런 무서운 일이...");
                if(@pos == 0) end;
COS063:
                set @pos, dialog(0, 1, 1, "...아무튼 수고하셨소. 당신 덕분에 연구에 많은 진척이 있었소. 정말 고맙소. 이것은 이제껏 연구를 도와준 사례요. 재미있게 생긴 돌이지? 꽤 값어치가 나간다고 들었소만... 아마 오랜 경력을 가진 대장장이라면 이 돌을 단번에 알아볼 거요.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS062;
COS064:
                set @pos, dialog(0, 1, 1, "걱정되는구려. 내 연구가 성과를 보였으니 학자로서는 기쁘지만 이 땅이 또다시 피로 물들게 될 무서운 사실을 알게 되니, 착잡하기 이를 데가 없구려......");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS063;

                set $geumgang_rock, 11;
                additem @sd, name2itemid("금강석조각"), 1;
                addlegend @sd, 30, 6, 0, 1, "표신궁역사가의 부탁을 들어주다.";
                set_money @sd, get_money(@sd)+100000;
                updatestatus @sd, 2;
                end;
                
            }
            else end;
        }
        if($geumgang_rock == 11)
        {
            set @pos, dialog(0, 0, 1, "요즘 어떻게 지내시나? 당신 덕분에 내 연구에 많은 진척이 있었소. 정말 고맙소.");
            end;
        }
    }
    else if(@list == 2)
    {
COS065:
        set @pos, dialog(0, 0, 1, "흠. 고문서를 팔러 왔다고?");
        if(@pos == 0) end;
COS066:
        set @pos, dialog(0, 1, 1, "고문서는 권수가 높을수록 좀 더 높은가격으로 사고있네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS065;
COS067:
        set @pos, list2(@sell, 0, 1, "그래. 어떤 고문서를 팔겠는가?", "표신궁고문서1", "표신궁고문서2", "표신궁고문서3", "표신궁고문서4", "표신궁고문서5", "표신궁고문서6");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS066;
        else if(@sell > 0)
        {
            setarray @item$[1], "표신궁고문서1", "표신궁고문서2", "표신궁고문서3", "표신궁고문서4", "표신궁고문서5", "표신궁고문서6";
            setarray @money[1], 10000, 20000, 30000, 40000, 50000, 60000;

            if(countitem(name2itemid(@item$[@sell])) > 0)
            {
                set @sell_2, list1(@item$[@sell] + "을 " + @money[@sell] + "만전에 팔겠는가?", "예, 팔겠습니다.", "아니오, 좀더 생각해볼게요.");
                if(@sell_2 == 1)
                {
                    delitem @sd, name2itemid(@item$[@sell]), 1;
                    message @sd, 3, @item$[@sell] + " 주었습니다";
                    set_money @sd, get_money(@sd)+@money[@sell];
                    updatestatus @sd, 2;
                    set @pos, dialog(0, 0, 1, "고맙네. 다음에 또 생기면 언제든지 찾아와주게나.");
                }
            }
            else
            {
                set @pos, dialog(0, 0, 0, @item$[@sell] + "을 가지고 있지 않은데?");
            }
            deletearray @item$[1], getarraysize(@item$);
            deletearray @money[1], getarraysize(@money);
        }
        else end;
    }
    else end;
}