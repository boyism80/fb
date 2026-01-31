function ON_SCHEDULE_0()
    -- Repeat schedule test script
    -- Executes every 10 seconds during the date range
    broadcast("스케줄러 반복 실행 테스트 (10초마다)", MESSAGE_TYPE.NOTIFY)
end

function ON_SCHEDULE_1()
    -- One-time schedule test script
    -- Executes once at the specified time
    log("스케줄러 일회성 실행 테스트", MESSAGE_TYPE.NOTIFY)
end