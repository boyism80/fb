-- Runs once when the game server finishes starting.
-- Use property(key, value) to set server-wide properties (string, number, or boolean).
-- Other scripts can read them with property(key).

-- Initial battery stock for 시계장인 (restocked daily at 18:00 by ON_SCHEDULE_2)
property("clock_time_item", 10)

sky_maze_shuffle()
pk_sky_maze_shuffle()
