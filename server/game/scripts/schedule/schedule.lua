function ON_SCHEDULE_0()
    broadcast("스케줄러 반복 실행 테스트 (10초마다)", MESSAGE_TYPE.NOTIFY)
end

function ON_SCHEDULE_1()
    log("스케줄러 일회성 실행 테스트", MESSAGE_TYPE.NOTIFY)
end

-- Daily 18:00: restock batteries at 시계장인 (clock_time_item)
function ON_SCHEDULE_2()
    gv("clock_time_item", 10)
end