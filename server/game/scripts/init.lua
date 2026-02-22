-- Runs once when the game server finishes starting.
-- Use gv(key, value) to set server-wide globals (string, number, or boolean).
-- Other scripts can read them with gv(key).

-- Initial battery stock for 시계장인 (restocked daily at 18:00 by ON_SCHEDULE_2)
gv("clock_time_item", 10)
