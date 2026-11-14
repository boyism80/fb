return {
    required = { REVIVE = true, DIALOG = true },
    { name = "DEAD", tasks = { "REVIVE" } },
    { name = "RETURN", tasks = { "ESCAPE", "DEFEND", "AID", "RECOVER" } },
    { name = "PREPARE", tasks = { "DIALOG", "REST", "SELL", "SUPPLY", "BUFF", "OUTFIT", "EQUIP", "LEARN", "JOB", "GROUP", "PART" } },
    { name = "PROGRESS", tasks = { "ORDER", "TUTORIAL" } },
    { name = "TRAVEL", tasks = { "TRAFFIC" } },
    { name = "HUNT", tasks = { "LOOT", "FIELD" } },
}
