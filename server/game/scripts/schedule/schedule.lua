-- Daily 18:00: restock batteries at 시계장인 (clock_time_item)

return {
    on_schedule_0 = function()
        broadcast("스케줄러 반복 실행 테스트 (10초마다)", MESSAGE_TYPE.NOTIFY)
    end,

    on_schedule_1 = function()
        log("스케줄러 일회성 실행 테스트", MESSAGE_TYPE.NOTIFY)
    end,

    on_schedule_2 = function()
        property("clock_time_item", 10)
    end,

    on_schedule_3 = function()
        local sky_maze = require('lib.sky_maze')
        sky_maze.shuffle()
        sky_maze.pk_shuffle()
    end
}
