-- Smoke callbacks for map:set_timer / map:cancel_timer

return {
    on_tick = function()
        log('script_timer smoke: on_tick')
    end,

    on_once = function()
        log('script_timer smoke: on_once')
    end,
}
