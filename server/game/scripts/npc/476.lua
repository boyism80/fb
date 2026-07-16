-- npc: 오른이
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

local function handle_jingogyun_lt1(me, npc)
    local cases = {
        {
            "특이한 인성초들이 둥글게 모여 앉아 있다.",
            "서로 마주보고 앉은 인성초들은 가까이 다가가도 공격을 해 오지 않는다.",
            "조용히 눈빛을 주고받는 모습이 마치 대화라도 나누고 있는 듯 하다.",
        },
        {
            "그래그래 화화의 냄새가 진하게 난다.",
            "어라라... 이건 화화의 기운인데.",
            "화화의 친구일까?",
            "글쎄? 화화의 친구일지도 모르지.",
            "우리가 잘못 느끼는게 아닐까? 우린 바쁘고 힘들잖아. 착각 일지도 몰라.",
            "맞아맞아. 우린 너무 바쁘고 힘들어. 너무 지쳐서 잘못 느끼고 있는지도 몰라.",
            "화화는 잘 있을까?",
            "그래, 난 화화가 잘 있을지 궁금해. 정말 궁금해.",
        },
        {
            "오늘은 닭이 밝은데?",
            "정말, 정말 닭이 밝다.",
            "근데 정말 화화의 친구면 어떻게하지?",
            "그래, 정말 화화의 친구일지도 모르지.",
            "화화가 보고 싶다.",
            "나도, 나도 정말 화화가 보고 싶어.",
            "달빛이 아름답다. 화화가 좋아하겠네.",
            "그래그래, 분명 좋아할거야. 화화가 보고 싶다.",
            "후후후.",
            "하하하하하. 화화는 기분이 좋아서 웃고 있을거야.",
        },
        {
            "화화에게 새 친구가 생겼을까?",
            "글쎄? 저게 새 친구일지도 모르지.",
            "아닐지도 모르지.",
            "그래그래, 아닐 수도 있지.",
        },
    }
    local c = cases[math.random(1, 4)]
    if not run_dialogs(me, npc, c) then
        return
    end
    me:dialog(npc, "(당신은 똑같은 얼굴들이 똑같은 목소리로 똑같은 이야기를 반복하는 모습을 지켜보다가 질려서 듣기를 멈추었다.)", false, false)
end

local function handle_jingogyun_1(me, npc, q_jingo)
    if not run_dialogs(me, npc, {
        "앗, 이건 진인랑의 기운이다. 진인랑과 화화의 기운이 동시에 느껴져.",
        "그럼 우리편이네. 우리편이지? 우리편일거야, 그렇지?",
        "그래그래, 우리편이야. 틀림없을거야.",
    }) then
        return true
    end
    local sel, btn = me:list(npc, "인성초들이 맑은 눈으로 이쪽을 바라보며 묻는다. '넌 우리편이지?' ", {
        "응, 난 화화의 친구야.",
        "내가 왜 너희 편을 들어주냐... 난 나쁘다구.",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 2 then
        run_dialogs(me, npc, {
            "음, 아니래. 무늬만 우리편이래.",
            "싫다. 정말 싫어.",
            "화화는 친구 보는 눈이 너무 없는 것 같아. 그래, 친구 보는 눈이 없는거야.",
            "맞아맞아, 돌순이 같은 친구를 사귀는게 좋을텐데.",
            "그래그래, 돌순이가 좋지. 돌순이 같은 아이라면 찬성이야.",
            "나도 찬성 찬성 찬성.",
            "(어느사이 인성초들로부터 소외되어버렸다.)",
        })
        return true
    end
    if sel == 1 then
        run_dialogs(me, npc, {
            "우리편이 맞다. 우리편이 맞다.",
            "그래그래, 참 착하게 생겼다. 참 착하게 생겼다.",
            "너는 우리편이니까, 우리 부탁을 들어줄거야, 그렇지? 그렇지?",
            "맞아맞아. 틀림없이 들어줄거야.",
        })
    end
    sel, btn = me:list(npc, "인성초들이 뚫어질듯 이쪽을 바라보며 묻는다. '도와줄꺼지?'", {
        "잠깐만, 우리 숨 좀 돌리지 않을래?",
        "물론이야. 내가 뭐든 도와줄께. 말만해.",
        "지금은 좀 무리야. 나중에 꼭 도와줄게",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 1 then
        if not run_dialogs(me, npc, {
            "잠깐? 잠깐이라고?",
            "잠시만 이라는 뜻일거야.",
            "그래, 그런뜻이지.",
            "우리에게 할 말이 있는거야. 할 말이 있는거지.",
        }) then
            return true
        end
        local sub_sel, sub_btn = me:list(npc, "인성초들이 놀란 눈으로 이쪽을 올려다본다. '왜? 무슨일이야?'", {
            "만나서 반가워, 소개 좀 들을 수 있을까?",
            "너희들은 어째서 색이 다르지?",
            "여기서 뭘 하고 있는거야?",
            "후우, 그래서 무슨 이야기를 하고 있었지?",
        }, false)
        if sub_btn == DIALOG_RESULT.QUIT or sub_sel == nil then
            return true
        end
        if sub_sel == 1 then
            run_dialogs(me, npc, {
                "그래, 만나서 반가워. 나는 꽃잎이 화려해서 화사화사라고 해.",
                "나도 만나서 반가워. 나는 잎맥이 오른쪽으로 쏠려서 오른이라고 해.",
                "만나서 반가워. 나는 잎맥이 왼쪽으로 쏠려서 외니라고 해.",
                "안녕안녕. 반가워. 나는 꽃잎이 얌전해서 수수라고 해.",
                "(인성초들의 인사를 받았지만, 누가 누구인지 분간하기 힘들었다.)",
            })
        elseif sub_sel == 2 then
            run_dialogs(me, npc, {
                "우리? 우리들은 폭염왕에 대항하고 있기 때문이야.",
                "우리들이야 말로 진정한 인성초. 사람을 공격하고 있는 우리 동족들은 폭염왕에게 지배당하고 있는거야.",
                "우린 폭염왕의 지배에서 벗어나 힘을 기르고 있어. 그래, 열심히 힘을 모으고 있지.",
                "언젠가 폭염왕을 쓰러뜨리고 우리의 자유를 되찾을거야. 모두가 행복하게 살 수 있도록 말이야",
                "그래그래, 달 밝은 밤에 모두 둥글게 모여서 춤을 추는거야.",
                "그리고 노래를 부르는거지. 우리가 사는 이 섬의 흙속으로 이어져오는 아름다운 노래를 부르는거지.",
                "닭이 밝구나. 닭이 밝아.",
                "바다 물결위에 달이 웃는다.",
                "닭이 밝구나. 닭이 밝아.",
                "(인성초들의 노래는 아름답긴 했지만, 어딘가 슬픔이 느껴지기도 했다.)",
            })
        elseif sub_sel == 3 then
            run_dialogs(me, npc, {
                "우리는 폭염왕에 대항해야해. 자유를 되찾아야해.",
                "그래, 맞아. 행복했던 시절로 돌아가야해.",
                "하지만, 우리는 너무 약하고 숫자도 적어.",
                "그래서 탄생의 씨앗을 구해다 심고, 새로운 동료를 싹틔우고 있어.",
                "정말 어려운 일이지. 정말 어려운 일이야.",
                "제일 힘든건 탄생의 씨앗을 구하는 일이야. 정말 구하기 힘들지.",
                "진인랑이 폭염왕의 힘을 막아주는 곳에서만 씨를 심을 수 있어",
                "우리는 씨앗을 돌보느라 돌아다닐 시간이 없고, 진인랑도 폭염왕의 힘을 막느라 돌아다닐 시간이 없지.",
                "게다가 특별한 씨앗은 살아있는 자의 눈으로 찾을 수가 없어.",
                "그래, 그래서 화화가 우릴 도와주지. 하지만, 화화도 씨앗을 찾으러 돌아다닐 수가 없어.",
                "너무 힘들어. 힘들지",
                "(인성초들이 슬픈 얼굴로 연거푸 한숨을 내쉰다.)",
            })
        else
            return
        end
        return true
    end
    if sel == 3 then
        run_dialogs(me, npc, {
            "대단해! 역시 화화와 진인랑의 친구다워!",
            "그래, 정말 대단해! 정말 대단해! 이런 인간은 참 오랜만이야!",
            "정말이지 용감한 사람이야. 폭염왕을 쓰러뜨리게 되면 역사에 길이 남을거야.",
            "우리에게 탄생의 씨앗을 가져다줘. 해바라기 씨앗중에 섞여서 태어나는 씨앗이야.",
            "그걸 구분해낼 수 있는건 화화 뿐이지.",
            "귀여운 화화만이 할 수 있는 일이지. 오로지 화화만이 할 수 있는 일이야.",
            "동료는 많고 많을수록 좋아. 탄생의 씨앗 10개를 구해줘.",
            "특별한 씨앗 10개! 10개! 10개!",
        })
        if q_jingo then
            q_jingo:step(2)
        end
        return true
    end
    if sel == 4 then
        run_dialogs(me, npc, {
            "들었어? 도와준다고 했어. 나중에 도와준다고 했다.",
            "나도 들었어. 나중에 꼭 도와준다고 했어.",
            "착하다. 화화는 좋은 친구를 사귀었구나.",
            "그래 착해착해 화화는 착해서 좋은 친구를 만난거야.",
            "닭이 밝다. 닭이 밝아.",
            "화화는 달을 좋아하지. 참 좋아해.",
            "(어느사이 인성초들로부터 소외되어버렸다.)",
        })
    end
    return true
end

local function handle_jingogyun_2(me, npc, q_jingo)
    if not me:has_items("탄생의씨앗", 10) then
        run_dialogs(me, npc, {
            "탄생의씨앗을 아직 다 구하지 못했어?",
            "시간은 영원하지 않아. 우리의 시간은 짧고 또 짧지.",
            "서둘러! 다시 한 번 지옥의 불길이 타오르기 전에... 서둘러야해.",
        })
        return true
    end
    if not me:rmitem("탄생의씨앗", 10, ITEM_DELETE_TYPE.GIVE) then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
        return true
    end
    if q_jingo then
        q_jingo:step(3)
    end
    run_dialogs(me, npc, {
        "씨앗을 가져와주었구나. 대단해! 훌륭해!",
        "그래, 너무 훌륭하다! 구세주야!",
        "자, 그럼 어서 이곳에 심도록 하자",
        "고마워, 이제 곧 우리 동료들이 태어날거야.",
        "열명의 동료가 태어날거야. 열명이 태어나는거라구.",
        "우리가 정성을 다하는 일만 남았어.",
        "아름다웠던 시절로 돌아가는거야.",
    })
    return true
end

local function handle_jingogyun_6(me, npc, q_jingo)
    if not run_dialogs(me, npc, {
        "음? 고균의영력을 담은 구슬이라구? 내가 몇 개 가지고 있지",
        "영혼의 구슬이지. 영웅 고균의 영력을 담아놓은 신비한 구슬...",
        "굉장히 귀한거야. 이제 이 섬에도 얼마 남지 않았지.",
        "귀하지 귀해. 이게 없으면 우리 동료들을 싹 틔울 수 없으니까 정말 귀하지.",
        "하지만, 우리를 도와주었으니까 우리도 도와줘야해.",
        "무엇이 좋을까? 무엇이 좋을까? 고균의영력을 대신 할 수 있는게 있을까?",
        "바람의 토템, 바람의 토템이라면 대신 할 수 있지. 있고 말고.",
    }) then
        return true
    end
    local sel, btn = me:list(npc, "바람의 토템 4개를 가져오면 바꿔줄께.", {
        "좋아, 곧 구해올께. 기다려줘.",
        "은혜를 갚아! 당장 달라구!",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 2 then
        me:dialog(npc, "안돼.", false, false)
        return true
    end
    if sel == 1 and q_jingo then
        q_jingo:step(7)
    end
    run_dialogs(me, npc, {
        "좋아 어서 다녀와.",
        "기다릴께.",
        "너무 서두르지 않아도 괜찮아.",
        "폭염왕의 부하들을 조심해.",
    })
    return true
end

local function handle_jingogyun_7(me, npc, q_jingo)
    local sel, btn = me:list(npc, "어서와. 어때? 다 구했니?", {
        "응, 자, 여기있어.",
        "아니, 그냥 들려봤어.",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 2 then
        me:dialog(npc, "어서 다녀와.", false, false)
        return true
    end
    if sel == 1 then
        local code = me:exchange(
            { ['item'] = { ["바람의토템"] = 4 } },
            { ['item'] = { ["영력구슬"] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "바람의토템 4개를 가져오면 바꿔줄께.", false, false)
            return true
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 영력구슬을 받을 수 없어.", false, false)
            return true
        end
        run_dialogs(me, npc, {
            "그럼 지금부터 바람의 토템으로 씨앗을 싹 틔울거야. 의식에 참여해.",
            "딴생각하거나, 딴데를 보면 의식은 반드시 실패하니까.",
            "그래그래, 엄숙하게 참여해야지. 아주 경건하게 참여해야지.",
            "새로운 동료를 맞이하자. 새로운 동료의 탄생을 축하해주자.",
            "그럼 시작하자.",
            "따스한 하늘의 바람아 여기 머물어 친구의 잠을 깨워라",
            "따스한 바다의 바람아 여기 머물어 친구의 입을 축여라.",
            "따스한 대지의 바람아 여기 머물어 친구의 배를 채워라.",
            "따스한 화염의 바람아 여기 머물어 친구의 몸을 데워라.",
            "성공이다!",
            "친구가 싹을 틔웠어!",
            "성공이야! 친구가 눈을 떴다!",
            "다행이야! 성공이야! 다행이야!",
            "자, 이건 약속한 고균의 영력이 담긴 구슬이야.",
            "요긴하게 쓰면 좋겠다. 아, 그게 좋지 좋아.",
            "이제 친구를 보살펴야해. 그래 소중하게 보살펴야하지.",
        })
        if q_jingo then
            q_jingo:step(8)
        end
    end
    return true
end

local function handle_jingogyun_ge8(me, npc)
    run_dialogs(me, npc, {
        "어서와 어서와 어서와 어서와",
        "잘 지냈어? 우리는 친구를 돌보느라 많이 바빠.",
        "바쁘고 보람차지. 아, 그럼그럼, 너무나 보람찬 일이지.",
        "이제 금방 폭염왕을 물리치고 우리의 고향을 되찾을 수 있을거야.",
        "그때는 노래하며 춤추자. 노래하고 춤을 추자.",
        "웃으면서 사는거야. 노래하고 춤추면서 사는거야.",
        "(자그마한 싹을 둘러싸고 앉아있는 인성초들은 굉장히 기뻐 보인다.)",
    })
end

function NPC_476(me, npc)
    local q_jingo = me:quest(quest.QUEST_JINGOGYUN)
    local step = (q_jingo and q_jingo:step()) or 0

    if step < 1 then
        handle_jingogyun_lt1(me, npc)
        return
    end
    if step == 1 then
        if handle_jingogyun_1(me, npc, q_jingo) then
            return
        end
    end
    if step == 2 then
        if handle_jingogyun_2(me, npc, q_jingo) then
            return
        end
    end
    if step == 6 then
        if handle_jingogyun_6(me, npc, q_jingo) then
            return
        end
    end
    if step == 7 then
        if handle_jingogyun_7(me, npc, q_jingo) then
            return
        end
    end
    if step >= 8 then
        handle_jingogyun_ge8(me, npc)
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end