local siege = require('schedule.siege')

return {
    on_init = function(sm)
        if siege.has_heart_map() == false then
            return
        end
        sm:every('tick', 1000, 'on_tick')
    end,

    on_tick = function(sm)
        siege.end_due_sieges(sm)
    end,
}
