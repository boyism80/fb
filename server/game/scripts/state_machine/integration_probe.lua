return {
    on_init = function(sm)
        sm:set('ticks', 0)
    end,

    on_probe_start = function(sm)
        sm:set('ticks', 0)
        sm:set('stopped_ticks', nil)
        sm:set('called', true)
        sm:every('probe', 500, 'on_probe_tick')
    end,

    on_probe_tick = function(sm)
        sm:set('ticks', sm:get('ticks') + 1)
    end,

    on_probe_stop = function(sm)
        sm:cancel('probe')
        sm:set('stopped_ticks', sm:get('ticks'))
    end,

    on_probe_hold = function(sm, ms)
        sm:set('peeked', nil)
        sm:set('holding', true)
        sleep(ms)
        sm:set('holding', false)
    end,

    on_probe_peek = function(sm)
        sm:set('peeked', sm:get('holding') == true and 'during_hold' or 'after_hold')
    end,
}
