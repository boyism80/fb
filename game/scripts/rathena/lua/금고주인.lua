{
    set @sd, mysd();
    set @id, myid();

    if($strongbox == 0)
    {
COS001:
        set @pos, dialog(0, 0, 1, "으이구... 이 금고를 어떻게 연다? 열쇠가 없으니 도무지 열 방법이 없구만! 이를 어쩌지? 이를 어쩐다?");
        if(@pos == 0) end;
COS002:
        set @pos, list2(@list, 0, 1, "이 금고를 대체 어쩌면 좋지", "무슨 일이라도 있나요?", "신경쓰지 말자.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        else if(@list == 1)
        {
COS003:
            set @pos, dialog(0, 0, 1, "아 글쎄, 이 금고를 열어야 하는데 열쇠를 잃어 버렸지 뭐야. 내가 어렸을 때 나중에 커서 열어 보겠다고 잠근 금고인데, 막상 열려고 보니 열쇠가 없네.");
            if(@pos == 0) end;
COS004:
            set @pos, list2(@list_2, 0, 1, "뭐 방법이 없을까?", "열지 못하면 그냥 부수는 건 어때요?", "그냥 포기하세요...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS003;
            else if(@list_2 == 1)
            {
COS005:
                set @pos, dialog(0, 0, 1, "뭐? 부순다고? 맞아! 그런 방법이 있었군! 내가 왜 그 생각을 못했지? 으하하하!");
                if(@pos == 0) end;
COS006:
                set @pos, dialog(0, 1, 1, "..............");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS005;
COS007:
                set @pos, list2(@list_3, 0, 1, "...그런데 어떻게 부수지?", "이리 주세요. 제가 부숴 드리죠.", "몰라요. 알아서 하세요.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS006;
                else if(@list_3 == 1)
                {
COS008:
                    set @pos, dialog(0, 0, 1, "으음...! 음, 뭐, 좋아. 자네가 부숴 주겠다고? 그래. 자네한테 맡기지. 성공하면 보수도 주겠어.");
                    if(@pos == 0) end;
COS009:
                    set @pos, dialog(0, 1, 1, "단, 한가지 약속해줘야 할 게 있어!");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS008;
COS010:
                    set @pos, list2(@list_4, 0, 1, "이걸 열면 안에서 봉투가 하나 나올 텐데, 절대 그 봉투를 열어봐선 안돼! 약속할 수 있겠나?", "예, 절대 안 열어보고 그대로 가져다 드리지요.", "에이, 그게 뭐길래... 그냥 열어보면 안되나요?");
                    if(@pos == 0) end;
                    else if(@pos == -1) goto COS009;
                    else if(@list_4 == 1)
                    {
COS011:
                        set @pos, dialog(0, 0, 1, "좋아! 여기 금고가 있네. 알아서 부순 다음 안에 들어있는 봉투만 내게 주면 돼.");
                        if(@pos == 0) end;
COS012:
                        set @pos, dialog(0, 1, 1, "그리고! 다시 한 번 말해두지만 안에 있는 봉투를 절대 열어봐서는 안 돼. 약속했지? 좋아, 수고하게.");
                        if(@pos == 0) end;
                        else if(@pos == -1) goto COS011;

                        set $strongbox, 1;
                        addlegend @sd, 27, 7, 0, 1, "금고주인의 부탁을 들어주자.";
                        additem @sd, name2itemid("낡은금고"), 1;
                        end;
                    }
                    else if(@list_4 == 2)
                    {
                        set @pos, dialog(0, 0, 0, "안돼! 그..안에는...중요한 물건이...");
                        end;
                    }
                    else end;
                }
                else if(@list_3 == 2)
                {
                    set @pos, dialog(0, 0, 0, "...음...어떻게 부순담...");
                    end;
                }
                else end;
            }
            else if(@list_2 == 2)
            {
                set @pos, dialog(0, 0, 0, "결국 그래야 하는 건가...");
                end;
            }
            else end;
        }
        else end;
    }
    if($strongbox == 1)
    {
        if(countitem(name2itemid("낡은봉투")) > 0)
        {
            set $strongbox, 2;
            set $strongbox_2, 0;
            delitem @sd, name2itemid("낡은봉투"), 1;
            message @sd, 3, "낡은봉투 주었습니다";
            additem @sd, name2itemid("흑장단검"), 1;
            addlegend @sd, 27, 6, 0, 1, "금고주인의 부탁을 들어주었다.";
            set @pos, dialog(0, 0, 1, "오! 봉투를 가져왔군. 수고했어. 설마했는데 정말 안 열어봤군. 좋았어! 내 아끼던 거지만 상으로 이 흑장단검을 주지. 중국에서 가져온 귀한 칼이야. 그럼 잘 가게!");
            end;
        }
        else if(countitem(name2itemid("낡은연애편지")) > 0)
        {
COS013:
            set @pos, list2(@list_5, 0, 0, "아직 멀었나?", "아뇨, 다 됐습니다.", "예, 아직 멀었어요.");
            if(@pos == 0) end;
            else if(@list_5 == 1)
            {
COS014:
                set @pos, dialog(0, 0, 1, "이...이놈! 내가 그렇게 말했는데 결국 봉투를 열어봤구나! 나쁜 놈! 약속을 어기다니! 에잇! 말한대로 보수는 없다!");
                if(@pos == 0) end;

                set $strongbox, 2;
                set $strongbox_2, 1;
                delitem @sd, name2itemid("낡은연애편지"), 1;
                message @sd, 3, "낡은연애편지 주었습니다";
                spellaether @sd, name2spellid("금고주인의저주"), 1250;
                set_ac @sd, get_ac(@sd)+49;
                end;
            }
            else if(@list_5 == 2)
            {
                set @pos, dialog(0, 0, 0, "그런가? 그럼 서둘러주게.");
                end;
            }
            else end;
        }
        else
        {
            set @pos, dialog(0, 0, 0, "아직 멀었나 보군..");
            end;
        }
    }
    if($strongbox == 2)
    {
        if($strongbox_2 == 0)
        {
            set @pos, dialog(0, 0, 1, "잘 지내시나? 저번엔 고마웠네.");
            end;
        }
        else
        {
            if($strongbox_2 == 1) { set @pos, dialog(0, 0, 1, "....."); end; }
COS015:
            set @pos, list2(@list_6, 0, 0, "에이! 네놈만 보면 화가 난다. 설마 했지만 그래도 믿었는데, 보란 듯이 약속을 어기다니! 저리 가! 꼴도 보기 싫다!", "......", "죄송합니다. 호기심에 그만...");
            if(@pos == 0) end;
            else if(@list_6 == 2)
            {
COS016:
                set @pos, dialog(0, 0, 1, "분명히 봉투를 열지 말라고 했는데도! 내가 몇 번이나 말했는데도 그걸 열다니!\\n\\n에이! 못된 놈 같으니라고!");
                if(@pos == 0) end;
COS017:
                set @pos, dialog(0, 1, 1, "쯧...\\n\\n\\n...하지만 그걸 열기 위해 저 멀리 중국까지 다녀오며 고생 한 것도 사실이니, 내 그 사정은 좀 봐 주지.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS016;
COS018:
                set @pos, list2(@list_7, 0, 1, "대신! 자네가 읽은 그 편지 내용은 아무에게도 말하지 말 것! 어때, 약속할 수 있겠나?", "예! 물론입니다. 아무렇게도 말하지 않겠어요.", "그건 좀...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS017;
                else if(@list_7 == 1)
                {
                    set $strongbox_2, 1;
                    additem @sd, name2itemid("흑장단검"), 1;
                    addlegend @sd, 27, 6, 0, 1, "금고주인의 부탁을 들어주었다.";
COS019:
                    set @pos, dialog(0, 0, 1, "좋아. 그럼 자네가 노력한 것을 봐서 보수를 주지. 여기 흑장단검을 가져가게. 중국에서 가져온 귀한 물건이야.");
                    if(@pos == 0) end;
COS020:
                    set @pos, dialog(0, 1, 1, "그럼 잘 가게. 편지 내용은 아무에게도 말하지 않겠다고 약속한 것도 잊지 말고!");
                    if(@pos == -1) goto COS019;
                    end;
                }
                else if(@list_7 == 2)
                {
                    set @pos, dialog(0, 0, 0, "반성의 여지가 없군! 얘기는 끝일세!");
                    end;
                }
                else end;
            }
            else end;
        }
    }
}