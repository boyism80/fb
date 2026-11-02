return {
    required = { REVIVE = true, DIALOG = true },
    { name = "DEAD", tasks = { "REVIVE" } },
    { name = "RETURN", tasks = { "ESCAPE", "DEFEND", "RECOVER" } },
    { name = "PREPARE", tasks = { "DIALOG", "REST", "SELL", "SUPPLY", "BUFF", "EQUIP", "JOB" } },
    { name = "PROGRESS", tasks = { "ORDER", "TUTORIAL" } },
    { name = "TRAVEL", tasks = {} },
    { name = "HUNT", tasks = { "LOOT", "FIELD" } },
}
