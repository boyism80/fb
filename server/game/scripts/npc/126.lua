-- npc: 화화
local quest = require('lib.quest')
local enum = require('lib.enum')
local function run_dialogs(me, npc, messages)
    for i, msg in ipairs(messages) do
        local prev = (i > 1)
        local next = (i < #messages)
        local b = me:dialog(npc, msg, prev, next)
        if b == DIALOG_RESULT.QUIT or b == DIALOG_RESULT.PREV then
            return false
        end
    end
    return true
end

local function in_hwahwa_time_window()
    local t = datetime()
    local hour = t and t.hour or 0
    local min = t and t.min or 0
    local allowed_hours = { [1]=true, [4]=true, [7]=true, [10]=true, [13]=true, [16]=true, [19]=true, [22]=true }
    return allowed_hours[hour] and min >= 10 and min <= 40
end

local function get_smile_count(me)
    local q = me:quest(quest.QUEST_HWAHWA_SMILE)
    if not q then
        return 0
    end
    return q:step() or 0
end

local function add_smile(me, delta)
    local q = me:quest(quest.QUEST_HWAHWA_SMILE)
    if not q then
        q = me:start_quest(quest.QUEST_HWAHWA_SMILE)
        if q == nil then
            return
        end
    end
    if q then
        q:step((q:step() or 0) + delta)
        me:push_achievement(55, "화화의 미소를 " .. tostring(q:step()) .. "번 보았다", 7, 1)
    end
end

local function handle_jingogyun_4(me, npc, q_jingo)
    if not q_jingo or q_jingo:step() ~= 4 then
        return false
    end
    local sel, btn = me:list(npc, "어서와! 오늘도 와줬구나. 고마워, 정말 많이 기뻐!", {
        "응, 그냥 잘 있나해서 들렸어. 잘 지내지?",
        "반가워, 그동안 뭐 재미있는 일이라도 없었어?",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 1 or sel == 2 then
        run_dialogs(me, npc, {
            "고균의영검은 잘 쓰고 있니?",
            "뭐? 고균의 영력구슬? 음... 미안해, 마지막으로 가지고 있던건 돌순이를 줬어.",
            "돌순이는 나한테 비장의 도시락을 만들어 주느라 많이 힘들거든. 그래서 신세 갚을겸해서 줬어.",
            "아직 돌순이가 갖고 있을지도 모르겠다...",
        })
        q_jingo:step(5)
    end
    return true
end

local function handle_jingogyun_2(me, npc, q_jingo)
    if not q_jingo or q_jingo:step() ~= 2 then
        return false
    end
    local sel, btn = me:list(npc, "뭐? 탄생의 씨앗을 찾아달라구?", {
        "여기 씨앗들이 있어. 찾아줘",
        "근데 어느 씨앗을 모아야하지?",
        "아니야, 있다가 다시 올께.",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 1 then
        local reward = nil
        if math.random(1, 1000) < 12 then
            reward = { ['item'] = { ["탄생의씨앗"] = 1 } }
        end
        local code = me:exchange({ ['item'] = { ["해바라기씨"] = 1 } }, reward)
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "해바라기씨앗을 가져와~", false, false)
            return true
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 탄생의 씨앗을 줄 수 없어.", false, false)
            return true
        end
        if reward == nil then
            local msgs = {
                "아냐, 이것도 아니네. 음...요즘따라 탄생의 씨앗이 많이 줄어든 모양이네.",
                "이건 그냥 평범한 해바라기 씨앗이야. 음... 맛있다. 히히히.",
                "앗 너무 힘을 줘서 못쓰게 되어버렸네...",
                "튼실한 해바라기 씨앗이네. 탄생의 씨앗은 아니었어. 얌냠... 맛있다.",
                "으음... 탄생의 씨앗을 찾았다 싶었는데, 죽은 씨앗이었어. 유감이야.",
            }
            me:dialog(npc, msgs[math.random(1, #msgs)], false, false)
        else
            me:dialog(npc, "찾았다! 탄생의 씨앗이야! 찾아내서 다행이야!", false, false)
        end
        return true
    end
    if sel == 2 then
        run_dialogs(me, npc, {
            "음. 폭염왕은 부하 거미들을 시켜서 해바라기씨를 모으고 있어.",
            "인성초들은 선택받은 해바라기 씨에서 태어나거든. 폭염왕은 그렇게",
            "모든 해바라기 씨에서 인성초들을 태어나게 만든 다음 바로 자기 부하로 만들고 있어.",
        })
        return true
    end
    if sel == 3 then
        me:dialog(npc, "응 잘가~", false, false)
        return true
    end
    return true
end

local function do_frolic(me, npc)
    local sel, btn = me:list(npc, "신난다! 난 새벽 2시가 넘으면 사라지기 때문에 별로 시간이 없어!", {
        "많이 배고픈 것 같은데, 도시락 공양부터 받을래?",
        "가위 바위 보해서 엉덩이로 이름쓰기 하자.",
        "음, 그럼 재미있는 엣날 이야기를 해줄게!",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 1 then
        if me:has_items("비장의도시락", 1) and me:rmitem("비장의도시락", 1, ITEM_DELETE_TYPE.GIVE) then
            add_smile(me, 40)
            run_dialogs(me, npc, {
                "고마워. 잘 먹을게.\n\n그냥 날 주고 있다고 생각하고 있으면, 음식의 생령이 나한테로 옮겨올거야.",
                "아, 음식의 생령이 빠져나간 음식은 산사람이 먹으면 병에 걸리니까 버리자.",
                "잘 먹었습니다~~!\n\n고마워 잘 먹었어. 아직 조금 배가 고프긴 하지만...\n\n고마워, 정말 즐거웠어! 또 놀자!",
            })
        else
            me:dialog(npc, "뭐야? 없잖아.", false, false)
        end
        return
    end
    if sel == 2 then
        me:dialog(npc, "가위 바위 보? 음, 좋아! 절대 지지 않을거야!", false, true)
        local s2, b2 = me:list(npc, "가위...바위....보!", {
            "가위를 낸다.",
            "바위를 낸다.",
            "보를 낸다.",
        }, false)
        if b2 == DIALOG_RESULT.QUIT or s2 == nil then
            return
        end
        if s2 >= 0 and s2 <= 2 then
            if math.random(1, 100) > 5 then
                if math.random(1, 2) == 1 then
                    me:dialog(npc, "쳇! 그렇게 늦게 내면 내가 어떻게 이기니?\n\n이건 내가 진게 아니니까 엉덩이로 이름 쓰지 않을테야.", false, false)
                else
                    me:dialog(npc, "쳇, 내가 졌잖아!\n\n안해!", false, false)
                end
            else
                add_smile(me, 1)
                if math.random(1, 2) == 1 then
                    run_dialogs(me, npc, {
                        "와아~ 내가 이겼다. 하하하하.\n\n하하하. 너 엉덩이 무지 크다! 하하하하하.",
                        "음, 그것봐 내가 이긴다고 했잖아. 음, 재미있었어. 또 놀자!",
                    })
                else
                    run_dialogs(me, npc, {
                        "와~ 내가 이겼다. 헤헤헤헷\n\n엉덩이가 씰룩거리네. 하하하하. 너무 웃겨!",
                        "음, 그것봐 내가 이긴다고 했잖아. 음, 재미있었어. 또 놀자!",
                    })
                end
            end
        end
        return
    end
    if sel == 3 then
        local r = math.random(1, 1000)
        if r <= 2 then
            run_dialogs(me, npc, {
                "와, 돌순이 이야기보다 재미있을거같아! 이야기해줘!",
                "우와, 정말 신기하다! 그거 진짜야? 정말이야?",
                "와, 정말 재미있었어. 살아있으면 그런 이야기도 들을 수 있는거구나! 또 놀자!",
            })
            add_smile(me, 3)
        elseif r <= 15 then
            me:dialog(npc, "앗, 그거 내 이야기인데! 어떻게 알았어? 대단해!", false, false)
            add_smile(me, 2)
        else
            local msgs = {
                "그건 전에 돌순이에게 들었던 이야기인데?",
                "별루 재미없다...",
                "에이, 벌써 다 아는 이야기네...시시해.",
            }
            me:dialog(npc, msgs[math.random(1, #msgs)], false, false)
        end
    end
end

function NPC_126(me, npc)
    local x, y = me:position()
    if (x and x > 20) or (y and (y < 75 or y > 94)) then
        me:dialog(npc, "대체 어디서 장난을 하는거야? 내가 귀신이라고 우습게보여?", false, false)
        return
    end


    if not in_hwahwa_time_window() then
        me:dialog(npc, "앗, 큰일이야. 이제 다시 귀신의 모습으로 돌아가야해. 나중에 만나자.\n\n(소녀의 씁쓸한 목소리가 귓가에 남아 메아리쳤다.)", false, false)
        return
    end

    local q_hwahwa = me:quest(quest.QUEST_HWAHWA)
    local q_jingo = me:quest(quest.QUEST_JINGOGYUN)
    local jingo_step = (q_jingo and q_jingo:step()) or 0
    local hwahwa_friend = (q_hwahwa and q_hwahwa:step() >= 1)
    local smile = get_smile_count(me)

    if handle_jingogyun_4(me, npc, q_jingo) then
        return
    end
    if handle_jingogyun_2(me, npc, q_jingo) then
        return
    end

    if hwahwa_friend then
        me:dialog(npc, me:name() .. ", 잘 지내?", false, false)
        return
    end

    if smile >= 1500 then
        me:dialog(npc, "심심해~ 심심해~\n\n배고파~ 배고파~", false, true)
        local sel, btn = me:list(npc, me:name() .. "!!\n우리 친구하자! 어때?", {
            "음... 좋아!",
            "너랑 친구되기 싫은데?",
        }, false)
        if btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            local q = (not q_hwahwa) and me:start_quest(quest.QUEST_HWAHWA) or me:quest(quest.QUEST_HWAHWA)
            if q == nil then
                return
            end
            q:step(1)
            me:mkitem("고균의영검", 1)
            me:dialog(npc, "선물 하나 줄게!\n\n가지고 있는게 이것뿐이라...\n닳지않도록 조심히 써야 되~", false, false)
            return
        end
        if sel == 2 then
            local q = me:quest(quest.QUEST_HWAHWA_SMILE)
            if q and q:step() > 0 then
                q:step(math.max(0, (q:step() or 0) - 50))
            end
            me:dialog(npc, "꺼져!", false, false)
            return
        end
    end

    if smile > 0 then
        do_frolic(me, npc)
        return
    end

    if me:has_items("비장의도시락", 1) then
        if not run_dialogs(me, npc, {
            "소녀가 당신을 본다. 시선이 당신이 들고있는 도시락으로 향한다. 하지만, 표정은 경계심이 가득하다.",
            "어? 넌 누구야? 돌순이의 기운이 강하게 느껴지네. 돌순이의 친구니?",
        }) then
            return
        end
        local sel, btn = me:list(npc, "소녀가 호기심에 가득찬 눈으로 쳐다본다. 무어라 대답해야할까?", {
            "응, 난 돌순이의 친한 친구야.",
            "돌순이라니? 혹시 돈만 좋아하는 그 여자애말야?",
        }, false)
        if btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            if not run_dialogs(me, npc, {
                "그래?! 돌순이랑 친구였구나. 반가워, 나도 돌순이 친구야. 내 이름은 화화라고해.\n\n이쁜 이름이지? 이미 죽어서 이름이 이뻐도 아무 소용없긴 하지만...",
                "헤헤헤, 아무튼 만나서 다행이다. 돌순이가 오지 않는 날엔 하루종일 쓸쓸하거든.\n\n나는 옛날에 폐허 동굴에서 죽어서 친구가 없었거든. 그래서 죽어서도 친구를 사귀고 싶어.",
                "요즘은 돌순이가 항상 도시락을 가져다줘. 굉장히 비싼 도시락같던데. 어디서 그런 돈을 구하는걸까?",
            }) then
                return
            end
            local sel2, btn2 = me:list(npc, "그런데 이렇게 늦은 밤에 돌아다녀도 괜찮아? 늘 밤에 돌아다니는거야?", {
                "응, 난 밤이 무섭지 않거든.",
                "아니, 오늘은 어쩌다가 이렇게 돌아다니게 된거야.",
            }, false)
            if btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
                return
            end
            if sel2 == 1 or sel2 == 2 then
                do_frolic(me, npc)
            end
            return
        end
        if sel == 2 then
            me:dialog(npc, "뭐? 그게 무슨말이니?", false, false)
            return
        end
        return
    end

    me:dialog(npc, "소녀가 공허한 눈으로 당신을 쳐다본다. 당신에게서 무언가를 찾듯이 한참이나 살펴보다가 이윽고 조금 실망한 표정으로 다른 곳을 바라본다.", false, false)
end