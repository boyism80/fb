{
    set @sd, mysd();
    set @id, myid();

    if($dragon_king == 0)
    {
        if(get_level(@sd) < 40)
        {
            set @pos, dialog(0, 0, 0, "아직 나를 돕기엔 많이 부족한 것 같군..");
            end;
        }
COS001:
        set @pos, list2(@list, 0, 0, "자넨 뭍사람이 아닌가? 여기 용궁은 무얼하러 왔는지 궁금하네만 그보다 내 부탁좀 들어줄 수 있겠나?", "물론이지오.", "싫어요");
        if(@pos == 0) end;
        else if(@list == 1)
        {
COS002:
            set @pos, dialog(0, 0, 1, "고맙네.. 사실 자네도 이곳 용궁에 대해서 소문을 들었을지 모르겠네만 요즘 용궁 주위에 심상찮은 분위기가 맴돌고 있다네...");
            if(@pos == 0) end;
COS003:
            set @pos, dialog(0, 1, 1, "이에 내가 조사를 해 본 결과 용궁 8대 장군 중 하나인 복어장군이 반란에 가담하고 있다는 정보를 입수하게 되었다네..");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
COS004:
            set @pos, dialog(0, 1, 1, "아마도 복어장군뿐 아니라 다른 이들도 관련 있는 것 같네만 아직 그것까지는 알수가 없다네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS003;
COS005:
            set @pos, list2(@list_2, 0, 0, "그래서 말인데 자네가 복어장군에게 정보를 캐내 올수 있겠는가?", "좋습니다. 제가 해보지요.", "너무 위험한 일인 것 같아서 못하겠어요.");
            if(@pos == 0) end;
            else if(@list_2 == 1)
            {
                set $dragon_king, 1;
                addlegend @sd, 19, 7, 0, 1, "복어장군 생포 임무를 받다.";
                set @pos, dialog(0, 0, 1, "고맙네 어서 정보를 얻어오게나. 복홍이라면 복어장군이 어디있는지 알고 있을거라네.");
                end;
            }
            else end;
        }
        else end;        
    }
    if($dragon_king == 1)
    {
        if(countitem(name2itemid("태자전음문서")) == 0)
        {
            set @pos, dialog(0, 0, 0, "어서 복어장군에게 정보를 얻어오게.");
            end;
        }
        else
        {
COS006:
            set @pos, dialog(0, 0, 1, "오오 정말로 이 문서를 가지고 왔구만.");
            if(@pos == 0) end;
COS007:
            set @pos, dialog(0, 1, 1, "내 자네에게 보답을 하겠네");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS006;
COS008:
            set @pos, list2(@gift, 0, 1, "자네 능력치 중에서 어떤 것을 올리고 싶은가?", "힘이요!", "지식이요!", "민첩성이요!");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS007;

            set $dragon_king, 2;

            delitem @sd, name2itemid("태자전음문서"), 1;
            message @sd, 3, "태자전음문서 주었습니다";

            addlegend @sd, 19, 7, 0, 1, "복어장군 생포 임무 완료.";

            switch(@gift)
            {
            case 1:
                set_might @sd, get_might(@sd)+1;
                break;
            case 2:
                set_grace @sd, get_grace(@sd)+1;
                break;
            case 3:
                set_will @sd, get_will(@sd)+1;
                break;
            }
            updatestatus @sd, 0;
            set @pos, dialog(0, 0, 1, "그럼 자네의 힘을 올려주도록 하겠네. 우옷!");
            end;
        }
    }
    if($dragon_king == 2)
    {
        if(get_level(@sd) < 50)
        {
            set @pos, dialog(0, 0, 0, "아직 나를 돕기엔 많이 부족한 것 같군..");
            end;
        }
COS009:
        set @pos, dialog(0, 0, 1, "자네가 지난번 복어장군에게서 구해준 태자전음문서에서 게장군 또한 연루되었다는 사실을 알게 되었네.");
        if(@pos == 0) end;
COS010:
        set @pos, list2(@list_3, 0, 0, "이번에는 내 직접 게장군을 심문해 볼 터이니 게장군을 생포해 줄 수 있겠나?", "좋습니다.", "별루여...");
        if(@pos == 0) end;
        else if(@list_3 == 1)
        {
            set $dragon_king, 3;
            addlegend @sd, 19, 7, 0, 1, "게장군 생포 임무를 받다.";
COS011:
            set @pos, dialog(0, 0, 1, "고맙네, 내 듣기로 게장군이 더 강한 힘을 얻기 위해 근래 탈피를 했다더군.\\n\\n아직은 몸이 덜 굳어서 활동을 극히 자제하고 있을 걸세.");
            if(@pos == 0) end;
COS012:
            set @pos, dialog(0, 1, 1, "하지만 게장군은 자기 수하들을 극히 아낀다 들었네.\\n\\n자네가 게장군의 수하들을 손봐주고 있으면 그도 참지 못하고 모습을 드러낼걸세.\\n\\n그럼 되도록 빨리 잡아오도록 하게");
            if(@pos == -1) goto COS011;
            end;
        }
        else end;
    }
    if($dragon_king == 3)
    {
        if($crap == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 게장군을 잡아오지 못한 모양이군..");
            end;
        }
        else
        {
            set $dragon_king, 4;
            additem @sd, name2itemid("산해진미"), 1;

            addlegend @sd, 19, 7, 0, 1, "게장군 생포 임무 완료.";
COS013:
            set @pos, dialog(0, 0, 1, "고맙네, 내 산해진미를 줄 터이니 한번 맛이나 보시게나.");
            if(@pos == 0) end;
COS014:
            set @pos, dialog(0, 1, 1, "이건 아주 몸에 좋은 음식이니 혼자 몰래 들게나....하하하");
            if(@pos == -1) goto COS013;
            end; 
        }
    }
    if($dragon_king == 4)
    {
        if(get_level(@sd) < 60)
        {
            set @pos, dialog(0, 0, 0, "아직 나를 돕기엔 많이 부족한 것 같군..");
            end;
        }
COS015:
        set @pos, dialog(0, 0, 1, "지난번 자네의 도움으로 많은 정보를 얻게 되었네.\\n\\n예상보다 많은 자들이 이번 반란에 연루 되었더군.");
        if(@pos == 0) end;
COS016:
        set @pos, list2(@list_4, 0, 0, "그래서 말인데, 자네가 이번에는 문어장군을 좀 잡아들일 수 있겠나?", "걱정 마십시오!!!.", "별로 안내키는데요...");
        if(@pos == 0) end;
        else if(@list_4 == 1)
        {
            set $dragon_king, 5;
            addlegend @sd, 19, 7, 0, 1, "문어장군 생포 임무를 받다";
            set @pos, dialog(0, 0, 1, "고맙네. 문어장군은 생김새가 산적같이 생긴 자인데...\\n\\n매우 포악하니 조심해야 하네");
            end;
        }
        else end;
    }
    if($dragon_king == 5)
    {
        if($octopus == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 문어장군을 잡아오지 못했군..");
            end;
        }
        else
        {
            set @pos, list2(@list_5, 0, 0, "문어장군은 어디에 있고 왜 혼자 있나?", "문어장군은 자신의 결백을 증명해 보이고 싶다고", "자결을 하였습니다.");
            if(@pos == 0) end;
            else if(@list_5 > 0 && @list_5 <= 2)
            {
                set @pos, list2(@list_6, 0, 0, "뭐라? 자결을?", "그렇습니다.", "용왕님에게 의심을 받을 바에야 죽어서", "결백을 증명해 보이는 길을 택하겠다고 하며", "자결을 하며 이 다문창을 전해주라 하였습니다");
                if(@pos == 0) end;
                else if(@list_6 > 0 && @list_6 <= 4)
                {
                    set @pos, list2(@list_7, 0, 1, "음..그래? 안타깝네 그려.. 허나, 죽음으로 밝혀지는 건 아무것도 없네.", "...");
                    if(@pos == 0) end;
                    else if(@list_7 != 1) end;

                    set @pos, list2(@list_8, 0, 0, "이 다문창은 문어장군이 가장 아끼던 무기인데, 자네가 보관해 두게나.", "고맙습니다.");
                    if(@pos == 0) end;
                    else if(@list_8 == 1)
                    {
                        set $dragon_king, 6;
                        additem @sd, name2itemid("다문창"), 1;
                        addlegend @sd, 19, 7, 0, 1, "문어장군 생포 임무 완료.";
                        end;
                    }
                    else end;
                }
                else end;
            }
            else end;
        }
    }
    if($dragon_king == 6)
    {
        if(get_level(@sd) < 70)
        {
            set @pos, dialog(0, 0, 0, "아직 나를 돕기엔 많이 부족한 것 같군..");
            end;
        }
COS017:
        set @pos, list2(@list_9, 0, 0, "오...어서오게. 내가 자네를 얼마나 찾았는지 자넨 알고 있었나?.", "무슨 급한 일이 있으신지요?");
        if(@pos == 0) end;
        else if(@list_9 == 1)
        {
COS018:
            set @pos, dialog(0, 0, 1, "이번 반란에 대한 새로운 정보를 얻었네.\\n\\n그동안 자네가 내 부탁을 아주 잘 그리고 훌륭하게 들어 주었기 때문에 이번에도 자네의 도움을 좀 얻어볼까 해서야.");
            if(@pos == 0) end;
COS019:
            set @pos, list2(@list_10, 0, 0, "어떤가. 또다시 내 부탁을 좀 들어주겠나?", "물론이지요.", "죄송합니다만..바뻐서...");
            if(@pos == 0) end;
            else if(@list_10 == 1)
            {
                set $dragon_king, 7;
                addlegend @sd, 19, 7, 0, 1, "해마장군의 생포 임무를 받다";
COS020:
                set @pos, dialog(0, 0, 1, "고맙네. 자네가 도와 준다면 안심이라네.");
                if(@pos == 0) end;
COS021:
                set @pos, dialog(0, 1, 1, "이번에 얻은 정보는 용궁 8대장군 중 해마장군이라는 자가 있는데\\n이 자가 반란군의 주력 부대를 생산하는 역할을 담당하고 있다 하는군.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS020;
COS022:
                set @pos, dialog(0, 1, 1, "그래서 이자와 주력부대를 소탕하려고 하는데\\n그에 앞서 자네가 내부로 침투해서 혼란을 야기시켜 주면 좋겠네.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS021;
COS023:
                set @pos, dialog(0, 1, 1, "가능하다면 해마장군을 사로잡거나 쓰러뜨려주면 더욱 좋겠네.\\n\\n그럼 부탁하네.");
                if(@pos == -1) goto COS022;
                end;
            }
            else end;
        }
        else end;
    }
    if($dragon_king == 7)
    {
        if(get_job(@sd) == 4)
        {
            if($seahorse_king == 0)
            {
                set @pos, dialog(0, 0, 0, "아직 해마장군을 쓰러트리지 못한 모양이군.");
                end;
            }
            else
            {
                set $dragon_king, 8;
                additem @sd, name2itemid("진주반지"), 1;
                addlegend @sd, 19, 7, 0, 1, "해마장군 생포 임무 완료.";
COS030:
                set @pos, dialog(0, 0, 1, "지금 해마장군 소속 병사들이 대 혼란에 빠졌다고 들었네.\\n\\n이 모든 것이 자네 덕분이야.");
                if(@pos == 0) end;
COS031:
                set @pos, dialog(0, 1, 1, "지금이라면 해마장군 소속 병사들을 와해시키기 쉬울 듯 하네.\\n\\n정말 고맙네. 이것은 내 조그마한 성의니 받아두게");
                if(@pos == -1) goto COS030;
                end;
            }
        }
        else
        {
            if($dragon_king_quest_1 == 0)
            {
                if($seahorse_king == 0)
                {
                    set @pos, dialog(0, 0, 0, "아직 해마장군을 쓰러트리지 못한 모양이군.");
                    end;
                }
                else
                {
                    set $dragon_king_quest_1, 1;
                    addlegend @sd, 19, 7, 0, 1, "해마장군의 주력부대를 소탕하자.";
                    set @pos, dialog(0, 0, 1, "결국 해마장군을 쓰러트렸군. 고맙네. 이제 그의 주력부대를 소탕해주게.");
                    end;
                }
            }
            else
            {
                if($dragon_king_quest == 0)
                {
                    if($seahorse < 100)
                    {
                        set @pos, dialog(0, 0, 0, "아직 부족하네. 조금 더 적진을 휘저어 주게나.");
                        end;
                    }
                    else
                    {
                        set $dragon_king_quest, 1;
                        set @pos, dialog(0, 0, 1, "슬슬 해마병사들이 동요하기 시작했네.\\n\\n하지만 아직 주변부에 불과하니 더욱 분발해 주게나.");
                        end;
                    }
                }
                if($dragon_king_quest == 1)
                {
                    if($seahorse < 200)
                    {
                        set @pos, dialog(0, 0, 0, "아직 부족하네. 조금 더 적진을 휘저어 주게나.");
                        end;
                    }
                    else
                    {
                        set $dragon_king_quest, 2;
COS024:
                        set @pos, dialog(0, 0, 1, "해마병사들의 수군거림이 전보다 더 심해진 모양이라네.");
                        if(@pos == 0) end;
COS025:
                        set @pos, dialog(0, 1, 1, "슬슬 장수들도 불안해 하는 것 같기는 하지만 아직 견고하게 버티고 있지.\\n\\n아무래도 자네가 조금 더 수고해 주어야겠어.");
                        if(@pos == -1) goto COS024;
                        end;
                    }
                }
                if($dragon_king_quest == 2)
                {
                    if($seahorse < 300)
                    {
                        set @pos, dialog(0, 0, 0, "아직 부족하네. 조금 더 적진을 휘저어 주게나.");
                        end;
                    }
                    else
                    {
                        set $dragon_king_quest, 3;
COS026:
                        set @pos, dialog(0, 0, 1, "슬슬 적들도 이탈자가 나오기 시작하는 것 같구만");
                        if(@pos == 0) end;
COS027:
                        set @pos, dialog(0, 1, 1, "하지만 중심쪽은 장군들이 컨트롤 가능 한 모양이야.\\n\\n조금 더 노력해서 적득을 혼란시켜 주게나.");
                        if(@pos == -1) goto COS026;
                        end;
                    }
                }
                if($dragon_king_quest == 3)
                {
                    if($seahorse < 400)
                    {
                        set @pos, dialog(0, 0, 0, "아직 부족하네. 조금 더 적진을 휘저어 주게나.");
                        end;
                    }
                    else
                    {
                        set $dragon_king_quest, 4;
COS028:
                        set @pos, dialog(0, 0, 1, "적들이 눈에띄게 동요하기 시작했다네.\\n\\n이제 거의 성공 직전인 것 같아.");
                        if(@pos == 0) end;
COS029:
                        set @pos, dialog(0, 1, 0, "조금만 더 혼란시키면 그들을 쉽게 와해시킬 수 있을 것 같다네.");
                        if(@pos == -1) goto COS028;
                        end;
                    }
                }
                if($dragon_king_quest == 4)
                {
                    if($seahorse < 500)
                    {
                        set @pos, dialog(0, 0, 0, "아직 부족하네. 조금 더 적진을 휘저어 주게나.");
                        end;
                    }
                    else
                    {
                        set $dragon_king, 8;
                        additem @sd, name2itemid("진주반지"), 1;
                        addlegend @sd, 19, 7, 0, 1, "해마장군 생포 임무 완료.";
COS100:
                        set @pos, dialog(0, 0, 1, "지금 해마장군 소속 병사들이 대 혼란에 빠졌다고 들었네.\\n\\n이 모든 것이 자네 덕분이야.");
                        if(@pos == 0) end;
COS101:
                        set @pos, dialog(0, 1, 1, "지금이라면 해마장군 소속 병사들을 와해시키기 쉬울 듯 하네.\\n\\n정말 고맙네. 이것은 내 조그마한 성의니 받아두게");
                        if(@pos == -1) goto COS100;
                        end;
                    }
                }
            }
        }
    }
    if($dragon_king == 8)
    {
        if(get_level(@sd) < 99)
        {
            set @pos, dialog(0, 0, 0, "아직 나를 돕기엔 많이 부족한 것 같군..");
            end;
        }
COS032:
        set @pos, dialog(0, 0, 1, "어서오게.\\n\\n자네 덕분에 반란은 사전에 많이 진압을 했네.");
        if(@pos == 0) end;
COS033:
        set @pos, dialog(0, 1, 1, "하지만 아직 그 주동자를 알아내지 못해고, 지금은 교착 상태에 빠져 있네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS032;
COS034:
        set @pos, dialog(0, 1, 1, "곰곰히 생각해 보니, 나는 지금껏 게장군의 말만 믿고 반란에 관한 일을 진행해 오지 않았나.\\n\\n지금 와서 드는 생각이네만, 게장군의 말을 전부 믿을 수 없을 것 같네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS033;
COS035:
        set @pos, dialog(0, 1, 1, "그래서 말인데, 게장군도 인어장군에게 설득을 당해 이번 반란에 가담하게 되었다고 했으니, 인어장군은 뭔가 더 많은 것을 알고 있을것이라 생각하네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS034;
COS036:
        set @pos, list2(@list_11, 0, 0, "그러니 이번에 자네가 인어장군에게 정보를 좀 얻어 올 수 있겠나?", "그 정도야 가뿐하지요.", "이번만은 좀 힘들 것 같군요...");
        if(@pos == 0) end;
        else if(@list_11 == 1)
        {
            set $dragon_king, 9;
            addlegend @sd, 19, 7, 0, 1, "인어장군 생포 임무를 받다.";
            set @pos, dialog(0, 0, 1, "역시 자네는 시원 시원 하구만, 이번 일을 잘 해 오면 성의의 표시를 하지.\\n\\n잘 부탁하네");
            end;
        }
        else end;
    }
    if($dragon_king == 9)
    {
        if($mermaid == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 인어장군을 잡지 못하였군..");
            end;
        }
        else
        {
COS037:
            set @pos, list2(@list_12, 0, 0, "그래, 인어장군 일은 어떻게 되었나?", "인어장군을 사로잡아 물어보니", "반란의 죄를 인정하였습니다.");
            if(@pos == 0) end;
            else if(@list_12 > 0 && @list_12 <= 2)
            {
COS038:
                set @pos, list2(@list_13, 0, 0, "그래? 음..역시.. 그래서 지금 인어장군은 어디에 있나?", "그게, 인어장군 어미가 많이 아프셔서", "임종을 지켜보기를 간곡히 부탁하길래", "여기 이 내통문서를 받고 풀어주었습니다.", "어미일이 끝나는대로 용왕님을 찾아와서", "죄를 달게 받는다고 했습니다.");
                if(@pos == 0) end;
                else if(@list_13 > 0 && @list_13 <= 5)
                {
                    set @pos, list2(@list_15, 0, 0, "음.. 그렇군, 그 내통문서를 이리 줘 보게", "...");
                    if(@pos == 0) end;
                    else if(@list_15 != 1) end;

                    if(countitem(name2itemid("내통문서")) == 0)
                    {
                        set @pos, dialog(0, 0, 0, "내통문서가 없지 않은가?");
                        end;
                    }

                    set @pos, list2(@list_16, 0, 0, "음.. 아무래도 자네가 속은 듯 하네.", "...");
                    if(@pos == 0) end;
                    else if(@list_16 != 1) end;

                    set @pos, list2(@list_17, 0, 1, "이 문서를 도저히 알아 볼 수 없는 언어로 되어 있네 그려, 동방언어가 아니라 아마 서방언어인 듯 싶은데..", "...");
                    if(@pos == 0) end;
                    else if(@list_17 != 1) end;

                    set @pos, list2(@list_18, 0, 0, "아무래도 인어장군은 자네나 내가 이 문서의 내용을 알아 볼 수 없다는 것을\\n짐작하고 쉽게 내어 준 것 같네.", "...");
                    if(@pos == 0) end;
                    else if(@list_18 != 1) end;

                    set @pos, list2(@list_19, 0, 0, "아마도 어미의 일도 거짓일테지. 인어장군, 역시 간사하구나, 저런 저런...", "아니, 그게 정말이옵니까? 이리 줘 보십시오.");
                    if(@pos == 0) end;
                    else if(@list_19 != 1) end;

                    set @pos, list2(@list_20, 0, 0, " ", "This is the secret document....", "으허..이게 도대체 무슨 말인지..", "제 실수로 인어장군을 그냥 놓아 주었군요.", "아마도 인어장군은 지금쯤", "어디론가 모습을 감추었겠네요.");
                    if(@pos == 0) end;
                    else if(@list_20 > 0 && @list_20 <= 5)
                    {
                        set @pos, list2(@list_21, 0, 0, " ", "제 부주의로 인한 것이니,", "제가 반드시 이 문서를 번역해 오겠습니다.", "이것도 분명 언어이니", "누군가가 번역할수 있겠지요");
                        if(@pos == 0) end;
                        else if(@list_21 > 0 && @list_21 <= 4)
                        {
                            set @pos, list2(@list_22, 0, 0, "그래, 자네만 믿고 기다리겠네. 아마 중요한 정보가 담겨 있을 것이야.", "최선을 다하겠습니다.");
                            if(@pos == 0) end;
                            else if(@list_22 == 1)
                            {
                                set $dragon_king, 10;
                                end;
                            }
                            else end;
                        }
                        else end;
                    }
                    else end;
                }
                else end;
            }
            else end;
        }
    }
    if($dragon_king == 10)
    {
        if(countitem(name2itemid("번역된내통문서")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 내통문서를 번역하지 못했나 보군..");
            end;
        }
        else
        {
            set @pos, list2(@list_23, 0, 0, "오.. 내통문서를 번역해 왔나 보군.", "네, 여기 있습니다.");
            if(@pos == 0) end;
            else if(@list_23 != 1) end;

            set $dragon_king, 11;
            delitem @sd, name2itemid("번역된내통문서"), 1;
            message @sd, 3, "번역된내통문서 주었습니다";
            additem @sd, name2itemid("인어장군지팡이"), 1;
            addlegend @sd, 19, 7, 0, 1, "인어장군 생포 임무 완료.";
            set @pos, dialog(0, 0, 1, "고맙네. 이 지팡이를 받아 주게. 그리고 앞으로도 자주 용궁에 들러 주게.");
            end;
        }
    }
    if($dragon_king == 11)
    {
        if(get_level(@sd) < 99)
        {
            set @pos, dialog(0, 0, 0, "아직 나를 돕기엔 많이 부족한 것 같군..");
            end;
        }
COS039:
        set @pos, list2(@list_24, 0, 0, "오. 마침 적절할 때 잘 왔네.", "무슨 일이신가요?");
        if(@pos == 0) end;
        else if(@list_24 != 1) end;
COS040:
        set @pos, dialog(0, 0, 1, "자네가 구해다 준 내통문서 덕분에 누가 반역에 가담했는지 알게 되었다네.");
        if(@pos == 0) end;
COS041:
        set @pos, dialog(0, 1, 1, "그런데 알고보니 문어장군은 정말로 반역에 가담하지 않았더군.\\n\\n내 문어장군 같이 훌륭한 충신을 의심하다니....아...문어장군.....");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS040;
COS042:
        set @pos, dialog(0, 1, 1, "문어장군. 내 조그맣게 속죄하는 뜻에서 그대의 의지를 용궁 대대로 전해지게 하겠네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS041;
COS043:
        set @pos, dialog(0, 1, 1, "그나저나 내통문서에 적힌대로 상어장군에게 첩자를 심어놓았는데\\n적들이 반란을 시도하기 위한 준비를 거의 다 끝냈다는 사실을 알아냈다네.\\n\\n지금 적들의 사기가 하늘을 찌르고 있다고 하더군.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS042;
COS044:
        set @pos, list2(@list_25, 0, 0, "그래서 자네가 적들의 기세를 좀 꺽어주지 않겠는가?", "매우 쉬운 일이군요.", "저 개인이 무슨 힘이 있어서....");
        if(@pos == 0) end;
        else if(@list_25 != 1) end;

        set $dragon_king, 12;
        addlegend @sd, 19, 7, 0, 1, "상어장군의 생포 임무를 받다. [1/2]";
COS045:
        set @pos, dialog(0, 0, 1, "정말 고맙네.\\n\\n마침 상어장군이 돌격대장을 함께 역임하고 있다고 하니, 그자를 제거하면 적의 기세가 한풀 꺽일걸세,");
        if(@pos == 0) end;
COS046:
        set @pos, dialog(0, 1, 0, "상어장군을 제거하고 돌아오면 그대에게 험난한 세상을 해어 나가는데 도움이 될 힘을 주겠네.");
        if(@pos == -1) goto COS045;
        end;
    }
    if($dragon_king == 12)
    {
        if($shark_count == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 상어장군을 잡아오지 못한 모양이군..");
            end;
        }
        else
        {
            set @pos, list2(@list_26, 0, 0, "그래 상어장군을 잡아왔는가?", "예. 여기있습니다.");
            if(@pos == 0) end;
            else if(@list_26 != 1) end;

            set @pos, list2(@list_27, 0, 1, "아니, 저자는 상어장군이 아니지 않은가?", "예? 분명히 저자가", "자신의 입으로 상어장군이라고 했는데...");
            if(@pos == 0) end;
            else if(@list_27 > 0 && @list_27 <= 2)
            {
                set @pos, dialog(0, 0, 1, "저자는 상어장군의 심복인 상어장수라네. 자네가 저자의 농간에 속았구만.");
                if(@pos == 0) end;

                set @pos, list2(@list_28, 0, 0, " ", "이런. 제가 다시 가서 잡아오도록 하겠습니다.");
                if(@pos == 0) end;
                else if(@list_28 != 1) end;

                addlegend @sd, 19, 7, 0, 1, "상어장군의 생포 임무를 받다. [2/2]";

                set $dragon_king, 13;
                set @pos, dialog(0, 0, 1, "그래주겠나? 그럼 다시한번 부탁하네.");
                end;
            }
            else end;
        }
    }
    if($dragon_king == 13)
    {
        set @nt, callfunc("Server_Nowtime");
        if($shark_time < @nt)
        {
            if($shark_count != 2)
            {
                set @pos, dialog(0, 0, 0, "다시 상어장군을 잡아오게나. 이번에도 속지 않게 조심하게.");
                end;
            }
            else
            {
                if($shark_check == 1)
                {
                    set $shark_re, 1;
                    set @nt, callfunc("Server_Nowtime");
                    set $shark_time, @nt+259200;
COS047:
                    set @pos, dialog(0, 0, 1, "자네마저 나에 기대에 배신하다니....믿을 수 없군. 믿을수가 없어...");
                    if(@pos == 0) end;
COS048:
                    set @pos, dialog(0, 1, 1, "믿는 도끼에 발등 찍힌다더니 자네가 딱 그꼴이지 않은가!!!\\n\\n지금은 자네를 보기도 싫으니 어서 사라지게!!!");
                    if(@pos == -1) goto COS047;
                    end;
                }
                else
                {
COS049:
                    set @pos, dialog(0, 0, 1, "자네가 상어장군을 잡아올 때 상어장군이 한 말이 전 병사들에게 퍼져서 사기가 뚝 떨어졌다는 제보가 들어왔다네.");
                    if(@pos == 0) end;
COS050:
                    set @pos, dialog(0, 1, 1, "자네에게 약속대로 힘을 부여하겠네.");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS049;
COS051:
                    set @pos, dialog(0, 1, 1, "그럼 다음에 또 보세나");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS050;

                    set $dragon_king, 14;
                    addlegend @sd, 19, 7, 0, 1, "상어장군 생포 임무 완료";
                    set_purehp @sd, get_purehp(@sd)+500;
                    set_puremp @sd, get_puremp(@sd)+250;
                    end;
                }
            }
        }
        else
        {
            set @pos, dialog(0, 0, 1, "아직 내 화가 다 안풀렸으니 다음에 다시 오게!!!");
            end;
        }
    }
    if($dragon_king == 14)
    {
        if(get_joblevel(@sd) < 1)
        {
            set @pos, dialog(0, 0, 0, "아직 나를 돕기엔 많이 부족한 것 같군..");
            end;
        }
COS052:
        set @pos, list2(@list_29, 0, 0, "이거 번번히 신세를 져서 미안하구만.", "또 무슨 일이 있으신가 보지요?");
        if(@pos == 0) end;
        else if(@list_29 != 1) end;
COS053:
        set @pos, dialog(0, 0, 1, "지금 우리 군대와 반란군의 군대가 한참 전쟁일 벌이고 있는데, 적의 주모자를 제거하면 우리에게 유리한 방향으로 이끌어 나갈 수 있을 것 같네.");
        if(@pos == 0) end;
COS054:
        set @pos, dialog(0, 1, 1, "그런데 적군의 장수 중에서 해파리 장군이 그 정보를 알고 있다고 하더군");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS053;
COS055:
        set @pos, list2(@list_30, 0, 1, "자네가 나서서 그 정보를 좀 알아와 줄 수 있는가?", "물론입니다.", "별로 내키지 않는군요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS052;
        else if(@list_30 != 1) end;

        set $dragon_king, 15;
        addlegend @sd, 19, 7, 0, 1, "해파리장군 생포 임무를 받다.";
        set @pos, dialog(0, 0, 1, "이거 번번히 정말 고맙네. 그럼 부탁하겠네.");
        end;
    }
    if($dragon_king == 15)
    {
        if($jellyfish == 0)
        {
            set @pos, dialog(0, 0, 0, "하직 해파리장군을 잡아오지 못한것 같군.");
            end;
        }
        else
        {
COS056:
            set @pos, list2(@list_31, 0, 0, "해파리장군을 잡아왔는가?", "예");
            if(@pos == 0) end;
            else if(@list_31 != 1) end;
COS057:
            set @pos, list2(@list_32, 0, 0, "그럼 적의 전략문서는 어디 있는가?", "미처 발견하지 못했습니다.");
            if(@pos == 0) end;
            else if(@list_32 != 1) end;
COS058:
            set @pos, list2(@list_33, 0, 0, "음. 아마도 해파리 장군이 잡히기 직전에 측근들에게 넘겨주었을 걸세.", "...");
            if(@pos == 0) end;
            else if(@list_33 != 1) end;
COS059:
            set @pos, list2(@list_34, 0, 0, "머저 해파리부하를 여러마리 잡고나서 해파리수하를 닥달해보면 겁을 집어먹고 줄 것일세.", "그렇다면 제가 가서", "그 전략문서를 찾아오도록 하겠습니다.");
            if(@pos == 0) end;
            else if(@list_34 > 0 && @list_34 <= 2)
            {
                set $dragon_king, 16;
                addlegend @sd, 19, 7, 0, 1, "전략문서를 얻자.";
                set @pos, dialog(0, 0, 1, "그래주겠다면 정말 고맙네.");
                end;
            }
            else end;
        }
    }
    if($dragon_king == 16)
    {
        if(countitem(name2itemid("전략문서")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 전략문서를 구하지 못한 모양이군. 해파리수하와 해파리부하를 잡다보면 줄걸세.");
            end;
        }
        else
        {
COS060:
            set @pos, dialog(0, 0, 1, "오 문서를 가져왔군. 이리 주시게.");
            if(@pos == 0) end;
COS061:
            set @pos, list2(@list_35, 0, 0, "음....으으으........으윽...", "앗! 무슨 일이십니까?");
            if(@pos == 0) end;
            else if(@list_35 != 1) end;
COS062:
            set @pos, dialog(0, 0, 1, "처...청의태자가 반란의 수괴였다니.....믿을 수 없어.....이럴수가....");
            if(@pos == 0) end;
COS063:
            set @pos, list2(@list_36, 0, 0, "가만히 기다리면 저절로 자신의 손에 들어갈 용왕의 자리가 그렇게도 탐이 났단 말인가....아....", "고정하시지요.");
            if(@pos == 0) end;
            else if(@list_36 != 1) end;
COS064:
            set @pos, dialog(0, 0, 1, "여하튼 자네에게 커다란 빚을 지는구만,");
            if(@pos == 0) end;
COS065:
            set @pos, dialog(0, 1, 1, "내 자네에게 보답으로 용왕의반지를 주겠네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS064;

            set $dragon_king, 17;
            delitem @sd, name2itemid("전략문서"), 1;
            message @sd, 3, "전략문서 주었습니다";
            additem @sd, name2itemid("용왕의반지"), 1;
            addlegend @sd, 19, 7, 0, 1, "해파리장군 생포 임무 완료.";
            end;
        }
    }
    if($dragon_king == 17)
    {    
        if(get_joblevel(@sd) < 2)
        {
            set @pos, dialog(0, 0, 0, "아직 나를 돕기엔 많이 부족한 것 같군..");
            end;
        }
COS066:
        set @pos, list2(@list_37, 0, 1, "으음. 이제서야 겨우 충격에서 회복되는군.", "괜찮으십니까?");
        if(@pos == 0) end;
        else if(@list_37 != 1) end;
COS067:
        set @pos, dialog(0, 0, 1, "좀 나아진것 같기는 하구만. 신경 써 주어서 고맙다네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS066;
COS068:
        set @pos, dialog(0, 1, 1, "청의태자가 아직 어릴 때 양자로 들여서 여태까지 그렇게 사랑을 쏟아부었는데 이럴줄은 몰랐네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS067;
COS069:
        set @pos, dialog(0, 1, 1, "하여간 충격에서 벗어나고 나니까 이제는 화가 치밀어 오르는구만.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS068;
COS070:
        set @pos, dialog(0, 1, 1, "내 처음에는 그놈의 정에 이끌려서 청의태자가 도망가게 내버려 둘까 생각도 해 보았네만,");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS069;
COS071:
        set @pos, dialog(0, 1, 1, "얼마전에 인어장군이 심어놓은 첩자가 여의주를 훔쳐서 청의태자에게 도망가 버렸다네.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS070;
COS072:
        set @pos, dialog(0, 1, 1, "이제는 용서하고 싶어도 용서할 수 없는 지경에 이르렀지.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS071;
COS073:
        set @pos, list2(@list_38, 0, 1, "그래서 말인데 자네가 도와주던 김에 확실히 좀 도아주겠는가?", "끝을 보는 게 옳겠지요.", "남의 가족사에는 끼고 싶지 않아요.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS072;
        else if(@list_38 != 1) end;

        set $dragon_king, 18;
        addlegend @sd, 19, 7, 0, 1, "청의태자 생포 임무를 받다. [1/2]";
COS074:
        set @pos, dialog(0, 0, 1, "정말 고맙네. 고마워.");
        if(@pos == 0) end;
COS075:
        set @pos, dialog(0, 1, 1, "가서 청의태자를 붙잡고 여의주를 되찾아 와 주시게");
        if(@pos == -1) goto COS074;
        end;
    }
    if($dragon_king == 18)
    {
        if(countitem(name2itemid("여의주")) == 0)
        {
            set @pos, dialog(0, 0, 0, "어서 청의태자를 물리치고 여의주를 되찾아 와 주시게");
            end;
        }
        else
        {
            set @pos, list2(@list_39, 0, 0, "그래 청의태자는 어떻게 되었는가?", "청의태자는 그만 여의주의 힘을");
            if(@pos == 0) end;
            else if(@list_39 != 1) end;

            set @pos, list2(@list_40, 0, 0, " ", ",견디지 못하고 사라져 버렸습니다.");
            if(@pos == 0) end;
            else if(@list_40 != 1) end;

            set @pos, list2(@list_41, 0, 0, "그래. 그렇다면 여의주를 나에게 되돌려 주게나.", "예. 여기있습니다.", "아니. 드릴 수 없습니다.");
            if(@pos == 0) end;
            else if(@list_41 != 1) end;

            set @pos, list2(@list_42, 0, 0, "앗. 이러수가.", "왜 그러십니까?");
            if(@pos == 0) end;
            else if(@list_42 != 1) end;

            set @pos, list2(@list_43, 0, 0, "청의태자는 아직 죽은 것이 아니었다네.", "헉. 그렇다면......");
            if(@pos == 0) end;
            else if(@list_43 != 1) end;

            set @pos, list2(@list_44, 0, 0, "청의태자가 죽기 직전에 여의주의 힘을 피해서 잠시 아공간으로 사라졌던 모양이야.", "그럴수가...");
            if(@pos == 0) end;
            else if(@list_44 != 1) end;

            set @pos, list2(@list_45, 0, 0, "어서 가서 그자를 잡아다 주게.", "예. 알겠습니다.");
            if(@pos == 0) end;
            else if(@list_45 != 1) end;

            set $dragon_king, 19;
            delitem @sd, name2itemid("여의주"), 1;
            addlegend @sd, 19, 7, 0, 1, "청의태자 생포 임무를 받다. [1/2]";
            message @sd, 3, "여의주 주었습니다";
            end;
        }
    }
    if($dragon_king == 19)
    {
        if($cjddmlxowk == 0)
        {
            set @pos, dialog(0, 0 ,0, "아직 청의태자를 잡아오지 못한 건가?");
            end;
        }
        else
        {

            set @pos, list2(@list_46, 0, 0, "오오. 청의태자를 잡아왔는가", "......");
            if(@pos == 0) end;
            else if(@list_46 != 1) end;

            set @pos, list2(@list_47, 0, 1, " ", "...청의태자를 잡았으나 여의주를");
            if(@pos == 0) end;
            else if(@list_47 != 1) end;

            set @pos, list2(@list_48, 0, 0, " ", "사용하다가 사라져버렸습니다.");
            if(@pos == 0) end;
            else if(@list_48 != 1) end;

            set @pos, dialog(0, 0, 1, "그렇다면 둘이 함께 여의주의 힘을 사용한 모양이로군.");
            if(@pos == 0) end;
            set @pos, list2(@list_49, 0, 0, "둘이 나눠서 여의주의 힘을 받으면 버틸 수 있을 것이라고 믿었던 모양이군. 어리석구나...어리석어...", "그런데 여의주의 힘이 무엇인가요?");
            if(@pos == 0) end;
            else if(@list_49 != 1) end;
COS088:
            set @pos, dialog(0, 0, 1, "여의주. 그것은 우리 용궁에 있는 비보들을 한군데로 모을수 있게 해 주는 힘을 가진 구슬이라네.");
            if(@pos == 0) end;
COS089:
            set @pos, dialog(0, 1, 1, "더 이상은 우리 용궁의 비밀이라 말해줄 수 없군.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS088;
COS090:
            set @pos, dialog(0, 1, 1, "이제 반란이 자네의 덕분으로 무사히 진압이 되었네");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS089;

            set @pos, dialog(0, 1, 1, "내 감사의 뜻으로 이 용왕의투구를 주겠네.");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS090;

            set $dragon_king, 20;
            additem @sd, name2itemid("용왕의투구"), 1;
            addlegend @sd, 19, 6, 0, 1, "용궁 8대 반란을 임무를 완수하다.";

            set @pos, dialog(0, 0, 1, "언제든지 용궁의 은인으로 반갑게 맞이할 테니 종종 놀러 오게나.");
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, get_name(@sd) + "이군. 반란을 막아준걸 정말 고맙게 생각하고 있네.");
        end;
    }
}