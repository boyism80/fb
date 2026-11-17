return {
    cast_rate   = 10.0,
    multipliers = { ["투명"] = 8 },
    back_attack = 2.0,
    buff_tick   = 1,
    splashes    = { ["측면공격"] = { rate = 0.4, sides = 2 }, ["후면공격"] = { rate = 0.5, sides = 1 } },
    echoes      = { "분신" },
    spells      = {
        ["건곤대나이"] = {
            role = "STRIKE",
            type = "NORMAL",
            mp = 40,
            front = true,
            hp_cost = 0.6666666666666666,
            damage_hp = 1,
            hp = function(
                me)
                return me.hp * 2 // 3
            end,
            damage = function(me) return me.hp end
        },
        ["검신검귀"] = { role = "ENCHANT", type = "INPUT", mp = 60, damage = 20 },
        ["경수"] = { role = "DISGUISE", type = "INPUT", mp = 30 },
        ["기문방술"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["공력주입"] = { role = "TRANSFER", type = "TARGET", mp = function(me) return me.mp end },
        ["공력증강"] = { role = "CHARGE", type = "NORMAL", mp = 30, hp_cost = 0.4, hp_floor = 100, chance = 0.5, chance_int = 0.00196078431372549 },
        ["구름의기원"] = { role = "HEAL", type = "TARGET", mp = 120, hp = 500, self = false, group = false },
        ["구름의희원"] = { role = "HEAL", type = "TARGET", mp = 480, hp = 500, self = false, group = false },
        ["귀환"] = { role = "TRAVEL", type = "NORMAL", mp = 30 },
        ["극진뢰격참주"] = { role = "ATTACK", type = "TARGET", mp = 360, damage = 3580 },
        ["극진뢰격참주'첨"] = { role = "AREA", type = "NORMAL", mp = 470, damage = 3560 },
        ["극진백열참주"] = { role = "ATTACK", type = "TARGET", mp = 360, damage = 3580 },
        ["극진백열참주'첨"] = { role = "AREA", type = "NORMAL", mp = 470, damage = 3560 },
        ["극진자천무주"] = { role = "ATTACK", type = "TARGET", mp = 360, damage = 3580 },
        ["극진자천무주'첨"] = { role = "AREA", type = "NORMAL", mp = 470, damage = 3560 },
        ["극진화열참주"] = { role = "ATTACK", type = "TARGET", mp = 360, damage = 3580 },
        ["극진화열참주'첨"] = { role = "AREA", type = "NORMAL", mp = 470, damage = 3560 },
        ["금강불체"] = { role = "BUFF", type = "NORMAL", mp = 600, self = true, others = false, buff_time = 12, chance = 0.3, trigger = "EMERGENCY" },
        ["금강퇴"] = { role = "ATTACK", type = "NORMAL", mp = 0, damage = 120, front = true, hp_spend = 120 },
        ["금수"] = { role = "DISGUISE", type = "INPUT", mp = 30 },
        ["노도성황"] = { role = "DEBUFF", type = "TARGET", mp = 30, buff_time = 30, paralysis = false, blind = false },
        ["노획"] = { role = "LOOT", type = "NORMAL", mp = 30 },
        ["누리의기원"] = { role = "HEAL", type = "NORMAL", mp = 30, hp = 50, self = true, group = false },
        ["누리의빛"] = { role = "INSPECT", type = "NORMAL", mp = 30 },
        ["누리의힘"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180, boost = { buff_str = 3 }, stats = { buff_str = 3 } },
        ["대력검신"] = { role = "ENCHANT", type = "INPUT", mp = 60, damage = 30 },
        ["대지의기원"] = { role = "HEAL", type = "TARGET", mp = 50, hp = 50, self = false, group = false },
        ["마기지체"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["대지의빛"] = { role = "INSPECT", type = "TARGET", mp = 30 },
        ["대지의힘"] = { role = "BUFF", type = "TARGET", mp = 30, self = false, others = true, buff_time = 5, boost = { buff_str = 3 }, stats = { buff_str = 3 } },
        ["동귀어진"] = {
            role = "STRIKE",
            type = "NORMAL",
            mp = 10,
            hp_cost = 1.0,
            damage_hp = 2,
            hp = function(me)
                return me
                    .hp - 10
            end,
            damage = function(me) return me.hp * 2 end
        },
        ["동해의기원"] = { role = "HEAL", type = "TARGET", mp = 30, hp = 100, self = false, group = false },
        ["뢰격주"] = { role = "ATTACK", type = "TARGET", mp = 180, damage = 530 },
        ["뢰격주'첨"] = { role = "AREA", type = "NORMAL", mp = 250, damage = 510 },
        ["뢰격참주"] = { role = "ATTACK", type = "TARGET", mp = 250, damage = 740 },
        ["뢰격참주'첨"] = { role = "AREA", type = "NORMAL", mp = 330, damage = 720 },
        ["뢰진주"] = { role = "ATTACK", type = "TARGET", mp = 120, damage = 320 },
        ["뢰진주'첨"] = { role = "AREA", type = "NORMAL", mp = 180, damage = 300 },
        ["마비"] = { role = "DEBUFF", type = "TARGET", mp = 80, buff_time = 20, paralysis = true, blind = false, resists = { "PARALYSIS" } },
        ["망각"] = { role = "ESCAPE", type = "TARGET", mp = 60 },
        ["맹수"] = { role = "DISGUISE", type = "INPUT", mp = 30 },
        ["무력화"] = { role = "DISPEL", type = "TARGET", mp = 500 },
        ["무영보법"] = { role = "NONE", type = "NORMAL", mp = 0 },
        ["무형검"] = {
            role = "STRIKE",
            type = "NORMAL",
            mp = 180,
            front = true,
            hp_cost = 0.5,
            damage_hp = 1,
            hp = function(me) return me.hp // 2 end,
            damage = function(me) return me.hp + math.min(20000, me.mp) end
        },
        ["무장"] = { role = "BUFF", type = "TARGET", mp = 30, self = false, others = true, buff_time = 180, boost = { buff_phydef = 10 }, stats = { buff_phydef = -10 } },
        ["바다의기원"] = { role = "HEAL", type = "NORMAL", mp = 50, hp = 100, self = true, group = false },
        ["바다의빛"] = { role = "INSPECT", type = "TARGET", mp = 30 },
        ["반탄공"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["백령주"] = { role = "ATTACK", type = "TARGET", mp = 180, damage = 530 },
        ["백령주'첨"] = { role = "AREA", type = "NORMAL", mp = 250, damage = 510 },
        ["백열장"] = { role = "ATTACK", type = "TARGET", mp = 80, damage = 280 },
        ["백열주"] = { role = "ATTACK", type = "TARGET", mp = 120, damage = 320 },
        ["백열주'첨"] = { role = "AREA", type = "NORMAL", mp = 180, damage = 300 },
        ["백열참주"] = { role = "ATTACK", type = "TARGET", mp = 250, damage = 740 },
        ["백열참주'첨"] = { role = "AREA", type = "NORMAL", mp = 330, damage = 720 },
        ["백호검무"] = {
            role = "STRIKE",
            type = "NORMAL",
            mp = 60,
            front = true,
            replaces = { "필살검무" },
            hp_cost = 0.5,
            damage_hp = 1,
            hp = function(me) return me.hp // 2 end,
            damage = function(me) return me.hp + math.min(me.mp, 20000) end
        },
        ["백호령"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 600, boost = { buff_dam = 3 }, stats = { buff_dam = 3 } },
        ["백호참"] = {
            role = "STRIKE",
            type = "NORMAL",
            mp = 60,
            front = true,
            replaces = { "건곤대나이", "어검술" },
            hp_cost = 0.5,
            damage_hp = 1,
            hp = function(me) return me.hp // 2 end,
            damage = function(me) return me.hp end
        },
        ["백호의희원"] = {
            role = "HEAL",
            type = "TARGET",
            self = false,
            group = false,
            hp = function(me) return me.mp * 3 // 2 end,
            mp = function(me) return me.mp // 2 end,
            delay = function(me)
                local mp = me.mp or 0
                local delay = mp / 10000
                if mp > 100000 then
                    delay = delay - (mp - 100000) / 100000
                end
                delay = math.max(0, math.floor(delay))
                local pct = ({
                    ["용겸제삼봉"] = 10,
                    ["용겸제사봉"] = 15,
                    ["용겸제오봉"] = 20,
                    ["용겸제육봉"] = 25,
                    ["용겸제칠봉"] = 30,
                    ["용겸제팔봉"] = 35,
                    ["용겸제구봉"] = 40,
                })[me.weapon] or 0
                return delay - delay * pct // 100
            end
        },
        ["백호의희원'첨"] = {
            role = "HEAL",
            type = "NORMAL",
            self = false,
            group = true,
            hp = function(me)
                local count = me.crew or 0
                local left  = me.mp
                local total = 0
                for _ = 1, count do
                    total = total + left * 3 // 2
                    left  = left // 2
                end
                return count > 0 and total // count or 0
            end,
            mp = function(me)
                local left = me.mp
                for _ = 1, math.max(me.crew or 0, 1) do
                    left = left // 2
                end
                return me.mp - left
            end,
            delay = function(me)
                local mp = me.mp or 0
                for _ = 2, math.max(me.crew or 0, 1) do
                    mp = mp // 2
                end
                local delay = mp / 10000
                if mp > 100000 then
                    delay = delay - (mp - 100000) / 100000
                end
                return math.max(0, math.floor(delay))
            end
        },
        ["보호"] = { role = "BUFF", type = "TARGET", mp = 30, self = false, others = true, buff_time = 180, boost = { damage_derate = 1000 }, stats = { damage_derate = 1000 } },
        ["부활"] = { role = "REVIVE", type = "TARGET", mp = 300 },
        ["분신"] = { role = "BUFF", type = "NORMAL", mp = 0, self = true, others = false, buff_time = 625 },
        ["비영사천문"] = { role = "TRAVEL", type = "INPUT", mp = 30 },
        ["비영승보"] = { role = "AMBUSH", type = "NORMAL", mp = 0 },
        ["사자후도사"] = { role = "SHOUT", type = "INPUT", mp = 0 },
        ["사자후도적"] = { role = "SHOUT", type = "INPUT", mp = 0 },
        ["사자후술사"] = { role = "SHOUT", type = "INPUT", mp = 0 },
        ["사자후전사"] = { role = "SHOUT", type = "INPUT", mp = 0 },
        ["사중공격"] = { role = "BUFF", type = "NORMAL", mp = 150, self = true, others = false, buff_time = 600, boost = { damage_rate = 3000 }, excludes = { "삼중공격", "오중공격", "이중공격" }, stats = { damage_rate = 3000 } },
        ["삼매진화"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["삼중공격"] = { role = "BUFF", type = "NORMAL", mp = 60, self = true, others = false, buff_time = 600, boost = { damage_rate = 2000 }, excludes = { "사중공격", "오중공격", "이중공격" }, stats = { damage_rate = 2000 } },
        ["생명의기원"] = { role = "HEAL", type = "TARGET", mp = 300, hp = 5000, self = false, group = false },
        ["선풍각"] = { role = "NONE", type = "TARGET", mp = 0 },
        ["성황령"] = { role = "TRAVEL", type = "INPUT", mp = 0 },
        ["소혼강신"] = { role = "SUMMON", type = "INPUT", mp = 30 },
        ["소환"] = { role = "CALL", type = "INPUT", mp = 30 },
        ["쇄혼비무"] = {
            role = "STRIKE",
            type = "NORMAL",
            mp = 140,
            reach = 4,
            clear = 3,
            dash = 3,
            hp_cost = 0.6666666666666666,
            damage_hp = 3,
            hp = function(me) return me.hp * 2 // 3 end,
            damage = function(me) return me.hp * 3 end
        },
        ["시력회복"] = { role = "CURE", type = "TARGET", mp = 40 },
        ["신검합일"] = { role = "ENCHANT", type = "INPUT", mp = 60, damage = 10 },
        ["신령의기원"] = { role = "HEAL", type = "TARGET", mp = 1000, hp = 10000, self = false, group = false },
        ["신령지익"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["신의축복"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 600, boost = { buff_hit = 3 }, stats = { buff_hit = 3 } },
        ["심안투영"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 600 },
        ["야수"] = { role = "DISGUISE", type = "INPUT", mp = 30 },
        ["어검술"] = {
            role = "STRIKE",
            type = "NORMAL",
            mp = 40,
            front = true,
            hp_cost = 0.6666666666666666,
            damage_hp = 1,
            hp = function(me) return me.hp * 2 // 3 end,
            damage = function(me) return me.hp end
        },
        ["오중공격"] = { role = "BUFF", type = "NORMAL", mp = 200, self = true, others = false, buff_time = 600, boost = { damage_rate = 4000 }, excludes = { "사중공격", "삼중공격", "이중공격" }, stats = { damage_rate = 4000 } },
        ["운기"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 11, trigger = "MANA", surge = 0.1 },
        ["유인"] = { role = "TAUNT", type = "TARGET", mp = 50 },
        ["이기어검"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["이중공격"] = { role = "BUFF", type = "NORMAL", mp = 60, self = true, others = false, buff_time = 600, boost = { damage_rate = 1000 }, excludes = { "사중공격", "삼중공격", "오중공격" }, stats = { damage_rate = 1000 } },
        ["자무주"] = { role = "ATTACK", type = "TARGET", mp = 120, damage = 320 },
        ["자무주'첨"] = { role = "AREA", type = "NORMAL", mp = 180, damage = 300 },
        ["자영무주"] = { role = "ATTACK", type = "TARGET", mp = 180, damage = 530 },
        ["자영무주'첨"] = { role = "AREA", type = "NORMAL", mp = 250, damage = 510 },
        ["자천무주"] = { role = "ATTACK", type = "TARGET", mp = 250, damage = 740 },
        ["자천무주'첨"] = { role = "AREA", type = "NORMAL", mp = 330, damage = 720 },
        ["저주"] = { role = "DEBUFF", type = "TARGET", mp = 30, buff_time = 185, stats = { buff_phydef = 30 }, paralysis = false, blind = false, resists = { "CURSE" } },
        ["절망"] = { role = "DEBUFF", type = "TARGET", mp = 80, buff_time = 10, paralysis = false, blind = true, resists = { "DESPAIR", "BLIND" } },
        ["중독"] = { role = "DEBUFF", type = "TARGET", mp = 30, buff_time = 30, dot = 0.05, dot_floor = 100, paralysis = false, blind = false, resists = { "POISON" } },
        ["지진"] = { role = "AREA", type = "TARGET", mp = 580, damage = 1500 },
        ["지폭지술"] = {
            role = "AREA",
            type = "NORMAL",
            drain = true,
            damage = function(me) return (me.mp * 3) // 2 end,
            mp = function(me) return math.max(me.mp, 30) end
        },
        ["진뢰격참주"] = { role = "ATTACK", type = "TARGET", mp = 330, damage = 1950 },
        ["진뢰격참주'첨"] = { role = "AREA", type = "NORMAL", mp = 400, damage = 1930 },
        ["진백열참주"] = { role = "ATTACK", type = "TARGET", mp = 330, damage = 1950 },
        ["진백열참주'첨"] = { role = "AREA", type = "NORMAL", mp = 400, damage = 1930 },
        ["진백호령"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 600, boost = { buff_dam = 5 }, stats = { buff_dam = 5 } },
        ["진자천무주"] = { role = "ATTACK", type = "TARGET", mp = 330, damage = 1950 },
        ["진자천무주'첨"] = { role = "AREA", type = "NORMAL", mp = 400, damage = 1930 },
        ["진화열참주"] = { role = "ATTACK", type = "TARGET", mp = 330, damage = 1950 },
        ["진화열참주'첨"] = { role = "AREA", type = "NORMAL", mp = 400, damage = 1930 },
        ["차폐"] = { role = "BUFF", type = "NORMAL", mp = 100, self = true, others = false, buff_time = 21 },
        ["천공의기원"] = { role = "HEAL", type = "TARGET", mp = 100, hp = 200, self = false, group = false },
        ["천공의희원"] = { role = "HEAL", type = "TARGET", mp = 200, hp = 200, self = false, group = false },
        ["초혼비무"] = {
            role = "STRIKE",
            type = "NORMAL",
            mp = 140,
            reach = 4,
            clear = 3,
            pull = 1,
            hp_cost = 0.6666666666666666,
            damage_hp = 3,
            hp = function(me) return me.hp * 2 // 3 end,
            damage = function(me) return me.hp * 3 end
        },
        ["출두"] = { role = "GOTO", type = "INPUT", mp = 30 },
        ["측면공격"] = { role = "BUFF", type = "NORMAL", mp = 90, self = true, others = false, buff_time = 625 },
        ["태양의기원"] = { role = "HEAL", type = "TARGET", mp = 240, hp = 1000, self = false, group = false },
        ["태양의희원"] = { role = "HEAL", type = "TARGET", mp = 960, hp = 1000, self = false, group = false },
        ["퇴마주"] = { role = "CURE", type = "TARGET", mp = 30 },
        ["투명"] = { role = "BUFF", type = "NORMAL", mp = 0, self = true, others = false, buff_time = 180 },
        ["파력무참"] = { role = "BUFF", type = "NORMAL", mp = 0, self = true, others = false, buff_time = 45 },
        ["파혼술"] = { role = "CURE", type = "TARGET", mp = 30 },
        ["포효검황"] = {
            role = "AREA",
            type = "NORMAL",
            mp = 600,
            damage = function(me) return (me.hp * 11) // 10 end,
            hp_spend = function(me) return me.hp // 2 end
        },
        ["필살검무"] = {
            role = "STRIKE",
            type = "NORMAL",
            front = true,
            hp_cost = 0.5,
            damage_hp = 1,
            damage_mp = 1,
            drain = true,
            hp = function(
                me)
                return me.hp // 2
            end,
            mp = function(me) return math.max(me.mp, 60) end,
            damage = function(me)
                return me
                    .hp + me.mp
            end
        },
        ["하늘의기원"] = { role = "HEAL", type = "NORMAL", mp = 120, hp = 200, self = true, group = false },
        ["해독"] = { role = "CURE", type = "TARGET", mp = 30 },
        ["헬파이어"] = {
            role = "ATTACK",
            type = "TARGET",
            damage_mp = 1.5,
            drain = true,
            damage = function(me) return math.floor(me.mp * 1.5) end,
            mp = function(
                me)
                return me.mp
            end,
            delay = function(me)
                return (7) -
                    (({ ["용랑제삼봉"] = 1, ["용랑제사봉"] = 1, ["용랑제오봉"] = 2, ["용랑제육봉"] = 2, ["용랑제칠봉"] = 3, ["용랑제팔봉"] = 3, ["용랑제구봉"] = 4 })[me.weapon] or 0)
            end
        },
        ["혼돈"] = { role = "NONE", type = "TARGET", mp = 0 },
        ["혼마술"] = { role = "DEBUFF", type = "TARGET", mp = 40, buff_time = 425, stats = { buff_phydef = 50 }, paralysis = false, blind = false },
        ["화열참주"] = { role = "ATTACK", type = "TARGET", mp = 250, damage = 740 },
        ["화열참주'첨"] = { role = "AREA", type = "NORMAL", mp = 330, damage = 720 },
        ["화염주"] = { role = "ATTACK", type = "TARGET", mp = 120, damage = 320 },
        ["화염주'첨"] = { role = "AREA", type = "NORMAL", mp = 180, damage = 300 },
        ["화영열주"] = { role = "ATTACK", type = "TARGET", mp = 180, damage = 530 },
        ["화영열주'첨"] = { role = "AREA", type = "NORMAL", mp = 250, damage = 510 },
        ["환군마술"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["환기"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["환마술"] = { role = "NONE", type = "NORMAL", mp = 0 },
        ["활력"] = { role = "CURE", type = "TARGET", mp = 30 },
        ["호체주술"] = { role = "BUFF", type = "NORMAL", mp = 30, self = true, others = false, buff_time = 180 },
        ["후면공격"] = { role = "BUFF", type = "NORMAL", mp = 90, self = true, others = false, buff_time = 625 },
        ["흡성대법"] = { role = "ATTACK", type = "TARGET", mp = 60, damage = 3000, limit = 3000 },
    },
}
