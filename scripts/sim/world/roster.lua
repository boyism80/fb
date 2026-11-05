return {
    solo_count = 40,
    solo = {
        data = "sim.data.snapshots_1",
        classes = { WARRIOR = true, ROGUE = true },
        band = 10,
        per = 2,
        duration = 600000,
        rezen_multiplier = 20.0,
        exp_multiplier = 100.0,
        exp_limit = false,
        latency = 100,
        step = 60,
        transfer = 4500,
        output = "logs/brain-sim",
    },
    team = {
        data = "sim.data.snapshots_1",
        duration = 1800000,
        rezen_multiplier = 20.0,
        exp_multiplier = 100.0,
        exp_limit = false,
        latency = 100,
        step = 60,
        transfer = 4500,
        acts = true,
        together = true,
        output = "logs/brain-sim/team",
        teams = {
            {
                name = "WA+PO",
                ground = { map = 957, bundle = "남경" },
                members = {
                    { class = "WARRIOR", low = 63, high = 79, spells = { "건곤대나이" } },
                    { class = "POET", low = 60, high = 79, spells = { "혼마술", "공력증강" } },
                }
            },
            {
                name = "RO+PO",
                ground = { map = 957, bundle = "남경" },
                members = {
                    { class = "ROGUE", low = 50, high = 69, spells = { "필살검무", "투명", "비영승보" } },
                    { class = "POET", low = 60, high = 79, spells = { "혼마술", "공력증강" } },
                }
            },
            {
                name = "WA9+PO",
                ground = {},
                members = {
                    { class = "WARRIOR", low = 63, high = 79, without = { "건곤대나이" } },
                    { class = "POET", low = 60, high = 79, spells = { "혼마술", "공력증강" } },
                }
            },
            {
                name = "MA",
                members = {
                    { class = "MAGE", low = 60, high = 79, spells = { "공력증강", "마비", "절망", "중독" } },
                }
            },
            {
                name = "PO",
                members = {
                    { class = "POET", low = 40, high = 79, spells = { "공력증강" } },
                }
            },
            {
                name = "WA",
                members = {
                    { class = "WARRIOR", low = 63, high = 79, spells = { "건곤대나이" } },
                }
            },
            {
                name = "RO",
                members = {
                    { class = "ROGUE", low = 50, high = 69, spells = { "필살검무" } },
                }
            },
            {
                name = "profile 63+65",
                ground = {},
                members = {
                    { class = "WARRIOR", profile = "warrior63" },
                    { class = "POET", profile = "poet65" },
                },
            },
            {
                name = "profile 63+65 west",
                ground = { bundle = "서쪽해안가" },
                members = {
                    { class = "WARRIOR", profile = "warrior63" },
                    { class = "POET", profile = "poet65" },
                },
            },
            {
                name = "profile 63+65 form",
                ground = { form = true },
                members = {
                    { class = "WARRIOR", profile = "warrior63" },
                    { class = "POET", profile = "poet65" },
                },
            },
            {
                name = "profile 63+65 no strike",
                ground = {},
                members = {
                    { class = "WARRIOR", profile = "warrior63", drop = { "건곤대나이" } },
                    { class = "POET", profile = "poet65" },
                },
            },
            {
                name = "WA63+PO65",
                ground = {},
                members = {
                    { class = "WARRIOR", low = 62, high = 66 },
                    { class = "POET", low = 63, high = 67 },
                },
            },
            {
                name = "WA+PO 세작",
                ground = { map = 535, bundle = "국내비밀세작의집" },
                members = {
                    { class = "WARRIOR", low = 80, high = 89, armed = true },
                    { class = "POET", low = 83, high = 89, spells = { "생명의기원", "공력증강" } },
                }
            },
            {
                name = "RO+PO 세작",
                ground = { map = 535, bundle = "국내비밀세작의집" },
                members = {
                    { class = "ROGUE", low = 80, high = 89, armed = true },
                    { class = "POET", low = 83, high = 89, spells = { "생명의기원", "공력증강" } },
                }
            },
            {
                name = "WA+PO 흉가",
                ground = { map = 572, bundle = "국내성흉가" },
                members = {
                    { class = "WARRIOR", low = 90, high = 99, armed = true },
                    { class = "POET", low = 90, high = 99, spells = { "생명의기원", "공력증강" } },
                }
            },
            {
                name = "RO+PO 흉가",
                ground = { map = 572, bundle = "국내성흉가" },
                members = {
                    { class = "ROGUE", low = 90, high = 99, armed = true },
                    { class = "POET", low = 90, high = 99, spells = { "생명의기원", "공력증강" } },
                }
            },
        },
    },
}
