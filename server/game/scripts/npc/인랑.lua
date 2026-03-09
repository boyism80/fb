
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

local function handle_hwahwa_not_friend(me, npc)
    if math.random(1, 2) == 1 then
        me:dialog(npc, "음? 처음 맡아보는 사람 냄새군. 이 섬에 사는 사람이 아닌듯 한데... 흠.", false, false)
    else
        me:dialog(npc, "나는 늑대들의 진실한 왕, 인랑이다. 이곳에서 당장 사라지도록 해.", false, false)
    end
    return true
end

local function handle_jingogyun_lt1(me, npc, q_jingo)
    local jingo_step = (q_jingo and q_jingo:step()) or 0
    if jingo_step >= 1 then
        return false
    end
    local b = me:dialog(npc, "화화의 내음이 잔뜩 묻어있군. 아주 오랜 시간동안 화화와 알고 지냈음이 틀림 없어.", false, true)
    if b == DIALOG_RESULT.QUIT then
        return true
    end
    b = me:dialog(npc, "그래, 화화의 친구라고 믿어도 좋겠군. 넌 누구지? 외부인이면서 어째서 죽은 자와 친구하고", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return true
    end
    b = me:dialog(npc, "저 사악한 무리들과 싸우고 있는거지? 무엇을 위해서? 무엇을 바라고 이곳에 서있는건가?", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return true
    end
    local sel, btn = me:list(npc, "대답해라. 도대체 이 곳으로 찾아온 이유가 무엇인지 말해라!", {
        "무슨 소리야? 난 부와 명성을 원할 뿐이다.",
        "나는 이 섬에 정의와 희망을 세우기 위해 왔다.",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 0 then
        run_dialogs(me, npc, {
            "뭣이? 이런 내 지인지감도 많이 녹슬었단 말인가. 하긴, 고균, 그 친구가 죽은 이후로",
            "인간들은 모두 타락해버렸지. 하하하, 허무토다, 친구여! 이것이 자네가 구한 세상이라니 말이야!",
            "(말을 마친 인랑은 하늘을 향해 길게 울부짖었다.)",
        })
        return true
    end
    if sel == 1 then
        if not run_dialogs(me, npc, {
            "그래? 하지만, 내가 순순히 그 말을 믿을거라 생각하면 오산이야. 나는 고균이 아니면, 어떤 사람도 믿지 않으니까.",
            "만약 네가 진정 그런 마음을 갖고 있다면, 남들을 위한 봉사정도야 기꺼이 할 수 있겠지? 고균또한 주변의 기대에",
            "부응하기 위해 얼토당토 않은 일들을 해결해야 했으니까! 자! 그럼 널 시험해보겠다! 마침 우리의 동료인 인성초",
            "네 자매가 곤경에 처해있던 참이야. 가서 인성초 네 자매를 도와라! 이야기는 그 다음에 듣겠다!",
        }) then
            return true
        end
        if not q_jingo then
            q_jingo = me:start_quest(QUEST_JINGOGYUN)
            if q_jingo == nil then
                return true
            end
        end
        if q_jingo then
            q_jingo:step(1)
        end
    end
    return true
end

local function handle_jingogyun_3(me, npc, q_jingo)
    local jingo_step = (q_jingo and q_jingo:step()) or 0
    if jingo_step ~= 3 then
        return false
    end
    if not run_dialogs(me, npc, {
        "역시 고균의 죽음은 헛되지 않았군. 비록 자네가 이 섬사람이 아니기는 하지만, 고균의 의지가 깃들어있다고 믿어 의심치않네.",
        "하하하하하. 정말 기쁘군. 이제부터 어딜 가서든 이 인랑의 친구라고 이야기해도 좋네. 자네가 이 섬을 떠나 멀리가더라도",
        "우리는 언제까지나 친구라네. 아주 기쁘군. 몇백년만에 느껴보는 기쁨이로다... 고균, 자네의 믿음이 틀리지 않아서 기쁘다네.",
        "하늘에서 고균이 자넬 보고 흐뭇해하겠군. 하하하하하하.",
    }) then
        return true
    end
    local list_opts = {
        "고균의 영검에 대한 전설이 있나요?",
        "절벽에 보이는 인랑은 대체 누구죠?",
        "고균은 어떤 사람이었나요?",
        "폭염왕의 정체는 대체 뭐죠?",
        "뭐 재미있는 이야기는 없을까요?",
        "화화와는 어떤 사이에요?",
        "저기 인성초들 말고도 다른 반란군이 있나요?",
    }
    local sel, btn = me:list(npc, "친구, 그래, 건강하게 지내고 있나? 오늘은 무슨 일이지? ", list_opts, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 0 then
        if not run_dialogs(me, npc, {
            "음? 고균의 영검? 그걸 자네가 어떻게 알고 있지? 설마 화화에게서 받았나? 허... 화화가 정말 친한 친구라고 인정한 모양이군.",
            "쉽게 말해서 고균이 자신의 부하들을 위해 만들어주었던 검일세. 중국인가 하는 나라에서 온 선박에서 섬주민이 되기로 마음먹은",
            "선원이 고균에게 '칠지도'라는 것을 선물했었는데, 그걸 본따 만들걸세. 흠... 보다시피 검날 양쪽에 달린 3개의 날은 방어하는데 쓰고",
            "가운데 검으로 공격을 하지. 혼란스러운 싸움터에는 그만한 무기가 없다네. 나이가 많아 방패까지 같이 챙겨들고 다니기 힘들었던",
            "고균은 이 검을 너무나 마음에 들어해서, 곧 똑같은 모양의 검을 만들어 부하들에게 나누어 주었지.",
        }) then
            return true
        end
        local sub_sel, sub_btn = me:list(npc, "음, 고균의 영검에 대해 뭐 더 알고 싶은게 있나? ", {
            "이게 그렇게 대단한 검인가요?",
            "뭐랄까 어딘가 부족한 느낌이 들어요.",
        }, false)
        if sub_btn == DIALOG_RESULT.QUIT or sub_sel == nil then
            return true
        end
        if sub_sel == 0 then
            run_dialogs(me, npc, {
                "글쎄, 그동안 동굴 속에서 많이 부식된 것처럼 보이긴 하겠지만... 여기에는 그 처절했던 독립의 의지가 깃들어 있어.",
                "비록 전성기의 모습을 갖추지 못하고 있긴 하지만, 그것만으로도 힘이 나지 않는가? 아니면, 양쪽 3개의 날로 방어하고",
                "가운데 검날로 공격하는 검술을 연습해보면, 방패조차 무색해지는 위력을 실감할 수 있을지도 모르지. 하하하하.",
            })
        else
            run_dialogs(me, npc, {
                "흠, 과연 내가 친구로 인정한 인간답군. 그래, 사실 그건 고균의 영검이긴 하지만, 영력이 사라진 모습일세.",
                "고균은 약한 부하들을 조금이라도 지켜주고자 자신의 영혼에서 힘을 빼내서 부하들의 검에 걸어주었다네... 고균이 죽으면서",
                "그 영력들은 모두 흩어져 버렸지만 말일세. 하하하. 사실 바로 작년만해도 고균이 내게 선물로 준 영력구슬을 갖고 있었지만,",
                "이런저런 이유로 모두 써버리고 나는 갖고 있는게 없군. 혹시 고균의 영력구슬을 찾는다면, 내가 고균의 영검이 숨기고 있는",
                "본 모습을 일깨워줄 수도 있지. 본 모습을 되찾은 영검은 영원히 주인의 곁을 떠나지 않는다네.",
            })
            if q_jingo then
                q_jingo:step(4)
            end
        end
        return true
    end
    if sel == 1 then
        run_dialogs(me, npc, {
            "응? 절벽에 보이는 인랑? 하하하하하하하! 뭐라고 이야기해야 할까. 뭐, 간단히 말해서 진인랑은 진인랑이지.",
            "나는 몇 백년 전에 늑대들의 왕자리를 버렸어. 지금 왕자를 노리고 서로 다투고 있는 인랑들은 쉽게 말해서",
            "왕위 계승자들이라고나 할까. 본디 그렇게 자리를 탐내거나 하는 성정은 아니지만, 폭염왕이 지맥을 이용해서",
            "독기를 섬전체에 퍼뜨린 뒤부터는 이미 늑대들의 자긍심은 사라진지 오래야. 게다가 싸움에서 지기라도 하면",
            "주저앉아 울기까지 하지. 그 얼마나 구차스런 모습인지 한숨이 다 나올 정도라네. 뭐, 이 땅에서 폭염왕을 몰아내는",
            "그 날이 온다면... 그래, 그 날이 오기만하면 우리 늑대들의 멋진 모습을 볼 수 있을거야. 암, 그렇고 말고.",
        })
        return true
    end
    if sel == 2 then
        run_dialogs(me, npc, {
            "고균! 그 이름만 들어도 나는 마음이 설렌다네. 내가 이 인랑의 이름을 걸고 존경하는 유일한 인간이야.",
            "상상을 해보게. 대부분의 인간들은 너무 약하고 너무 겁도 잘 먹지. 고균도 마찬가지야. 고균또한 약한 인간에 지나지 않았네.",
            "자네들 외부인처럼 수련을 쌓거나, 능력을 개발하거나 한 경험이 하나도 없었어. 고균은 나이 마흔이 넘어서 처음으로 검을 잡았지.",
            "싸움을 하기에는 너무 늙은 그 몸으로 폭염왕을 두려워하지 않고 싸우기 시작했네. 시작에는 오로지 고균밖에 없었네. 이 넓은 섬에",
            "폭염왕과 맞서 싸우려하는 자는 고균뿐이었어. 아아, 처음으로 고균을 만났던 날을 아직도 잊지 못하네. 그 날, 나는 절벽 주변에",
            "혹시 첩자가 숨어있지 않을까 수색을 다니고 있었어. 그때 엄청난 수의 거미와 뱀에게 둘러쌓인 고균을 발견했네.",
            "나는 폭염왕의 부하도 싫거니와, 인간들도 싫었기 때문에 가만히 서서 지켜보기만 했어. 난 틀림없이 고균이 쓰러질 줄 알았네",
            "부하들에게 들어서 고균의 존재는 알고 있었지만, 어차피 여왕거미의 식사감이나 되겠거니하고 생각했었거든. 근데 그렇지 않더군!",
            "공격은 허술하고, 방어는 빈틈투성이였는데, 어쩐지 고균이 폭염왕의 부하들을 압도하는거야. 이해할 수 없는 광경이었지.",
            "싸움이 끝나고나자, 고균이 멀리서 구경하던 나를 향해 소리치더군. 어째서 도와주지 않?말이야. 내가 폭염왕과 사이가 좋지",
            "않다는건 이미 모두가 아는 사실이었고, 그런 내가 가만히 서있었던게 이해할 수 없었던 모양이지. 그래서 간단히 대답했네.",
            "나는 인간도 좋아하지 않는다고 말일세. 그러나 고균이 말했지. 공동의 적을 쓰러뜨리기보다, 싫어하는 인간과 폭염왕이",
            "서로 싸우는 모습을 보고 즐기는 건 비겁한 행위라고, 그리고 진정 늑대의 자긍심이라는 것이 있다면 어째서 나서서 싸우지 않느냐고.",
            "절벽만 지키고 있는 것은 비겁할 뿐더러 스스로의 목을 조르는 행위에 지나지 않는다고... 처음에는 화가 났지만, 고균의 말은",
            "들으면 들을수록 묘한 설득력이 있었어. 마침내 나는 나도 모르게 고균에게 다가가 악수를 하고 말았네. 그때부터 정말 신나게 살았다네.",
            "하하하. 그래, 많은 희생이 있었고, 죽는게 더 편하겠다 싶은 고생도 많이 했지만, 그래도 신나는 하루하루였다네.",
            "그중에서 가장 기억에 남는건, 그래 동굴 전투였어. 고균의 뜻을 깨달은 인간들과 나와 내 부하들이 함께 동굴을 정벌했다네.",
            "정말 폭염왕조차 막을 수 없는 기세로 우리는 승리하고 또 승리했어. 모든 동굴들이 막힌 후에 우리는 폭염왕에게 달려들었지.",
            "아아, 정말 장렬한 전투였다네. 어떤 의미로는 처참하기까지 했지. 내 부하는 물론, 단단히 무장하고 있던 고균의 부하들도",
            "정말 많이 죽었다네. 더운 공기를 타고 섬전역에 피냄새가 퍼질만큼... 바다가 붉게 물들만큼 많은 동지들이 목숨을 잃었어.",
            "그런 숭고한 희생끝에 나와 고균은 마침내 폭염왕을 쓰러뜨렸네. 다만, 폭염왕은 불꽃의 신이라 소멸시킬 수가 없었기 때문에",
            "더이상 희생이 늘어나기 전에 폭염도의 화산굴에 봉인했지. 좀 더 힘을 내서 소멸시켰어야 했었어. 하지만, 그때는 고균도 나도",
            "금방 쓰러져 죽을만큼 힘을 다 써버린 뒤였고, 부하들도 거의 모두 쓰러지고 남은 자가 얼마 없었어. 하늘이 마지막까지는 우리를",
            "돕지 않은거지. 하하하하. 그래, 그리고 그때 오십줄에 접어든 고균은 너무 무리한 나머지 그만 목숨을 잃고 말았지.",
            "나는 고균의 시신을 옮겨다 빛도 어둠도 닿지 않는 곳에 묻었어. 나도 이제 많이 늙어서 자주 죽음에 대해 생각하곤해.",
            "죽을때가 되면 친구 곁엣거 눈을 감을 생각일세. 그러면, 이 섬에서 고균을 기억하는 자도 얼마 지나지 않아 모두 사라지겠지.",
            "마치 없었던 일처럼 말이야. 후후, 죽으면 고균을 만나서 그 친구가 그렇게 좋아하던 술이란걸 같이 마셔볼거야.",
            "아, 고균, 그 친구, 지금 하늘 위에서는 잘 지내려는지...",
        })
        return true
    end
    if sel == 3 then
        run_dialogs(me, npc, {
            "폭염왕! 이름만 들어도 분노가 머리끝까지 치솟아 오르는 이름이지! 아아, 그 날 하늘이 우리를 조금만 더 도왔더라면!!",
            "이 섬은 원래 인간과 우리들이 공존하며 살고 있었네. 서로 서로를 간섭치 않으면서, 자유롭게 말이야. 우리 늑대들은 낮에는",
            "사냥을 하고, 지치면 저녁 노을이 지는 해변가에 누워 쉬고, 밤에는 별을 보며 숲속을 자유로이 거닐었다네. 그 누구도 감히",
            "섬을 손에 넣으려거나, 자신들만 이 섬을 독차지하려 들지 않았어. 그래, 그야말로 낙원과도 같았지. 그런데 하늘에서 폭염왕이",
            "내려오더니 모든 것이 한순간에 변했네. 우리 늑대들은 숲에서 쫓겨나 절벽가로 내몰렸고, 거미와 뱀들은 폭염왕의 힘에 억눌려",
            "세뇌당했어. 인간들과 가장 사이좋게 지내던 인성초들조차 영혼을 지배당하고 말았네. 매일매일 행복하던 섬은 단숨에 죽음이",
            "지배하기 시작했네. 폭염왕은 그 거대한 불꽃으로 연기를 피어올리며 섬을 휘젓고 다니고, 거미와 뱀, 그리고 인성초들은 인간들을",
            "공격했어. 어느날 이 섬을 찾아든 선박 한 척은 인성초의 공격으로 난파되고, 선원들의 영혼마저 망령이 되어 폭염왕의 손아귀에",
            "떨어지는 일도 있었지. 아, 한시라도 빨리 폭염왕을 이 섬에서 몰아내야해. 그래, 몰아내야지. 만약 내 목숨이 필요하다고하면...",
            "언제라도 내 목숨정도야 초개처럼 던져버릴 각오가 되어있다네.",
        })
        return true
    end
    if sel == 4 then
        run_dialogs(me, npc, {
            "재미있는 이야기? 음, 좀 곤란하군. 나는 원체 시끄러운걸 싫어해서 말이지. 젊을 적에는 참 시끄럽게 놀고 돌아다니긴 했지만...",
            "그렇게 재미없게 살아온 것도 아닌데, 딱 떠오르는 이야기가 없군. 아, 그러고 보니, 꽤 오래전의 이야기지만, 혹시 찬찬이라고 아나?",
            "듣자하니 요즘은 어디서 잘 산다고 하더라만, 몇십년전만 해도 참 가관이었지. 내가 좋아하는 산책길 위가 온통 실패한 요리들로",
            "뒤덮여 있었어. 음, 거기까지는 참아줄 만했지만, 사랑 고백이니 뭐니 하면서 늦은 밤에 노래연습하러 다니는게 제일 싫었어.",
            "도무지 잠을 잘 수가 있어야지. 노래 연습하다가 고백도 못했다지?",
        })
        return true
    end
    if sel == 5 then
        run_dialogs(me, npc, {
            "아, 화화... 그 소녀말이로군. 참 안타까운 과거를 가진 소녀지. 성불하려면 친구 만명을 사귀어야 한다더군.",
            "하지만, 어디 친구가 그리 쉽게 구해지던가. 화화에게는 기껏해야 자네와 돌순이 정도겠지. 참으로 불행한 소녀야",
            "그런데 난 한번도 화화가 절망하거나 의기소침해지는 모습을 본 적이 없다네. 바로 거기서 한 가지를 배울 수 있었지.",
            "화화의 그런 자세야 말로 요즘 늑대들이 잊고 있는 그 무엇이 아닌가 하는 느낌이 들었네. 의연하다고나 할까? 물론",
            "떼를 잘 쓴다든가 하는 면도 있지만, 어린 나이에 쓸쓸하고 비참하게 죽은 탓이니까 말일세.",
        })
        return true
    end
    if sel == 6 then
        run_dialogs(me, npc, {
            "응? 반란군이라니? 우리는 독립군이라고 불러주게나. 힘적으로 열세이긴 하지만, 우린 단 한번도 폭염왕을 지배자로 받아들인 적도",
            "그 힘 아래 무릎꿇은 적도 없다네. 아시다시피 폭염왕은 우리 힘으로 저 화산굴에 봉인까지 했었으니까 말이야. 음, 우리 독립군은",
            "알게 모르게 여기저기 많이 모여있다네. 자네... 알런지 모르겠지만, 난파선에는 진짜 선장이 따로 숨어있다네. 지금은 그렇게 많은",
            "힘을 보태주지 않고 있지만, 분명한 우리의 아군이지. 그리고 우리 늑대들중에서 가장 강하다는 인랑 호위병들은 조용히 숨어서",
            "폭염왕과의 마지막 일전을 준비중일세. 비록 너무 착하고 순진해서 이 싸움에 껴주는게 미안할 정도이긴 하다만, 저기 인성초들도",
            "저렇게 노력하고 있지 않은가. 생각해보게나... 이 섬에서 제일 약하다는 인성초들마저 저리 노력하는데, 과연 눈에 띄지 않는다고해서",
            "우리 독립군의 힘이 적다고 생각할 순 없지 않겠나?",
        })
        return true
    end
    return true
end

local function handle_jingogyun_4_to_8(me, npc, q_jingo)
    local jingo_step = (q_jingo and q_jingo:step()) or 0
    if jingo_step < 4 or jingo_step > 8 then
        return false
    end
    local sel, btn = me:list(npc, "어서오게, 친구여. 그간 잘 지냈는가?", {
        "고균의 영력구슬은 어디있죠?",
        "고균의 영검에 영력을 불어넣어주세요.",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel == 0 then
        run_dialogs(me, npc, {
            "글쎄... 고균이 부하들에게 나누어준 물품이라 전쟁때 다 쓰고 얼마 남지 않았었어.",
            "내 것도 다 써버렸고... 흠, 아, 화화는 틈나는대로 섬 온 구석을 돌아다니는 취미가 있으니까",
            "어쩌면 영검 말고도 영력 구슬 한 두개쯤 가지고 있을지도 몰라.",
        })
        return true
    end
    if sel == 1 then
        if not me:has_items("영력구슬", 1) then
            me:dialog(npc, "고균의 영검에 영력을 불어넣기 위해서는 영력구슬이 필요하지.", false, false)
            return true
        end
        if not me:has_items("고균의영검", 1) then
            me:dialog(npc, "고균의 영검을 가져오면 영력을 불어넣어 주지.", false, false)
            return true
        end
        if not run_dialogs(me, npc, {
            "고균의 영검에 고균의 영력을 불어넣길 원한다면, 지금부터 딴 짓을 해선 절대 안된다네.",
            "만약 다른데를 본다거나, 딴 생각을 한다면 자칫 영검까지 망가져버릴 수 있으니까 말일세.",
            "자, 어디보자... 재료는 다 가지고 왔나?",
            "그럼! 고균의 영검에 영력을 불어넣겠네! 차아아아아앗!",
        }) then
            return true
        end
        local code = me:exchange(
            { ['item'] = { ["영력구슬"] = 1, ["고균의영검"] = 1 } },
            { ['item'] = { ["진'고균의영검"] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
            return true
        elseif code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 검을 줄 수 없네.", false, false)
            return true
        end
        if q_jingo then
            q_jingo:step(9)
        end
        run_dialogs(me, npc, {
            "하하하. 그래, 이거야! 이것이야말로 고균의 영검이 숨기고 있던 진정한 모습이지!",
            "아, 가만히 보고 있으려니 옛날이 생각나는군... 후후, 어쩐지 그때가 그립구만.",
            "여기 있네. 가져가서 좋은 일에 쓰도록 하시게.",
        })
    end
    return true
end

local function handle_jingogyun_ge9(me, npc, q_jingo)
    local jingo_step = (q_jingo and q_jingo:step()) or 0
    if jingo_step < 9 then
        return false
    end
    run_dialogs(me, npc, {
        "어서오시게, 친구. 내가 요즘 몸이 좋질 않아서 그리 오랫동안 이야기를 나눌 수 없을 것 같아 미안하군.",
        "후후후, 나이를 속일 순 없는게지. 아, 내 가슴 속에 남아있는 열정이 폭염왕을 처치할때까지는 타올라 주었으면 좋겠건만...",
        "(인랑은 금새 눈을 감고 잠이 들었다. 상처투성이의 늙은 얼굴에 어쩐지 슬픈 미소를 짓고 있다.)",
    })
    return true
end

function NPC_475(me, npc)
    local q_hwahwa = me:quest(QUEST_HWAHWA)
    local q_jingo = me:quest(QUEST_JINGOGYUN)
    local hwahwa_friend = (q_hwahwa and q_hwahwa:step() >= 1)

    if not hwahwa_friend then
        handle_hwahwa_not_friend(me, npc)
        return
    end

    if handle_jingogyun_lt1(me, npc, q_jingo) then
        return
    end
    if handle_jingogyun_3(me, npc, q_jingo) then
        return
    end
    if handle_jingogyun_4_to_8(me, npc, q_jingo) then
        return
    end
    if handle_jingogyun_ge9(me, npc, q_jingo) then
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
