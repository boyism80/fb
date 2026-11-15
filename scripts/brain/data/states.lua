return {
    required = { REVIVE = true, DIALOG = true },
    -- Tasks that go where the hunting plan says (the ground, its floors, the trips and the crew it is made for): started
    -- only on the plan of the bot's state (planner.stale).
    planned  = {
        FIELD = true,
        SELL = true,
        SUPPLY = true,
        OUTFIT = true,
        LEARN = true,
        JOB = true,
        GROUP = true,
        PART = true,
        ORDER = true,
        TUTORIAL = true
    },
    { name = "DEAD", tasks = { "REVIVE" } },
    { name = "RETURN", tasks = { "ESCAPE", "GUARD", "DEFEND", "AID", "RECOVER" } },
    { name = "PREPARE", tasks = { "DIALOG", "REST", "SELL", "SUPPLY", "BUFF", "OUTFIT", "EQUIP", "LEARN", "JOB", "GROUP", "PART" } },
    { name = "PROGRESS", tasks = { "ORDER", "TUTORIAL" } },
    { name = "TRAVEL", tasks = { "TRAFFIC" } },
    { name = "HUNT", tasks = { "LOOT", "FIELD" } },
}
