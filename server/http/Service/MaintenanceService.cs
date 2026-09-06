using Http.Model;
using Http.Redis;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    public class MaintenanceService
    {
        private const string MaintenanceKeyPrefix = "fb:maintenance:";

        private static readonly double RecurringScheduleScore = ((DateTimeOffset)new DateTime(2099, 12, 31, 23, 59, 59, DateTimeKind.Utc)).ToUnixTimeSeconds();

        private readonly RedisService _redisService;
        private readonly SessionService _sessionService;
        private readonly RabbitMqService _rabbitMqService;
        private readonly ServerStateService _serverStateService;
        private readonly ILogger<MaintenanceService> _logger;

        public MaintenanceService(
            RedisService redisService,
            SessionService sessionService,
            RabbitMqService rabbitMqService,
            ServerStateService serverStateService,
            ILogger<MaintenanceService> logger)
        {
            _redisService = redisService;
            _sessionService = sessionService;
            _rabbitMqService = rabbitMqService;
            _serverStateService = serverStateService;
            _logger = logger;
        }

        public async Task<bool> IsMaintenanceActive(uint world, DateTime? checkTime = null)
        {
            var info = await GetMaintenanceInfo(world, checkTime);
            return info != null && info.IsActive;
        }

        public async Task<MaintenanceInfo> GetMaintenanceInfo(uint world, DateTime? checkTime = null)
        {
            var now = checkTime ?? DateTime.Now;
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return null;

            var key = GetMaintenanceKey(world);
            var currentTimestamp = ((DateTimeOffset)now).ToUnixTimeSeconds();

            // Get schedules that haven't ended yet (score >= currentTimestamp); member is full JSON
            var members = await redis.Connection.SortedSetRangeByScoreAsync(
                key,
                start: currentTimestamp,
                stop: double.PositiveInfinity,
                take: 10);

            foreach (var member in members)
            {
                var schedule = DeserializeSchedule(member.ToString());
                if (schedule == null || !schedule.IsActive)
                    continue;

                if (IsWithinMaintenanceWindow(schedule, now))
                {
                    return new MaintenanceInfo
                    {
                        IsActive = true,
                        Message = schedule.Message,
                        StartTime = GetWindowStart(schedule, now),
                        EndTime = GetWindowEnd(schedule, now),
                        ScheduleId = schedule.Id
                    };
                }
            }

            return null;
        }

        public async Task<bool> CreateMaintenanceSchedule(MaintenanceSchedule schedule)
        {
            if (string.IsNullOrEmpty(schedule.Id))
                schedule.Id = Guid.NewGuid().ToString();

            schedule.CreatedAt = DateTime.Now;
            schedule.IsActive = true;

            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return false;

            try
            {
                var key = GetMaintenanceKey(schedule.World);
                var scheduleJson = JsonConvert.SerializeObject(schedule);
                var score = GetScoreForSchedule(schedule);
                await redis.Connection.SortedSetAddAsync(key, scheduleJson, score);

                _logger.LogInformation("Created maintenance schedule {ScheduleId} for world {World}", schedule.Id, schedule.World);

                // If the new schedule is already active, notify game servers immediately so regular users are kicked
                // without waiting for the background service's next cycle.
                var maintenanceInfo = await GetMaintenanceInfo(schedule.World);
                if (maintenanceInfo != null && maintenanceInfo.IsActive)
                {
                    var messageCount = await ForceLogoutRegularUsers(schedule.World);
                    _logger.LogInformation("Maintenance already active for world {World}, sent immediate kick to {Count} game server(s)", schedule.World, messageCount);
                }

                return true;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to create maintenance schedule {ScheduleId}", schedule.Id);
                return false;
            }
        }

        public async Task<bool> CancelMaintenanceSchedule(uint world, string scheduleId)
        {
            var (schedule, existingMember) = await FindScheduleAndMember(world, scheduleId);
            if (schedule == null || string.IsNullOrEmpty(existingMember))
                return false;

            schedule.IsActive = false;
            schedule.CancelledAt = DateTime.Now;

            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return false;

            try
            {
                var key = GetMaintenanceKey(world);
                await redis.Connection.SortedSetRemoveAsync(key, existingMember);
                var updatedJson = JsonConvert.SerializeObject(schedule);
                var score = GetScoreForSchedule(schedule);
                await redis.Connection.SortedSetAddAsync(key, updatedJson, score);

                _logger.LogInformation("Cancelled maintenance schedule {ScheduleId} for world {World}", scheduleId, world);
                return true;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to cancel maintenance schedule {ScheduleId}", scheduleId);
                return false;
            }
        }

        public async Task<List<MaintenanceSchedule>> GetAllSchedules(uint world)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return new List<MaintenanceSchedule>();

            var key = GetMaintenanceKey(world);
            var members = await redis.Connection.SortedSetRangeByRankAsync(key, 0, -1);
            var schedules = new List<MaintenanceSchedule>();
            foreach (var member in members)
            {
                var schedule = DeserializeSchedule(member.ToString());
                if (schedule != null)
                    schedules.Add(schedule);
            }

            return schedules.OrderByDescending(s => s.CreatedAt).ToList();
        }

        public async Task<bool> ApplyToAllWorlds(MaintenanceSchedule schedule)
        {
            var worlds = _redisService.GetConfiguredWorlds();
            if (worlds.Count == 0)
                return false;

            var success = true;
            foreach (var world in worlds)
            {
                var worldSchedule = new MaintenanceSchedule
                {
                    Id = Guid.NewGuid().ToString(),
                    World = world,
                    Title = schedule.Title,
                    Message = schedule.Message,
                    StartTime = schedule.StartTime,
                    EndTime = schedule.EndTime,
                    RepeatType = schedule.RepeatType,
                    IsActive = schedule.IsActive
                };

                if (!await CreateMaintenanceSchedule(worldSchedule))
                    success = false;
            }

            return success;
        }

        public async Task<int> ForceLogoutRegularUsers(uint world)
        {
            // Get maintenance info to include in message
            var maintenanceInfo = await GetMaintenanceInfo(world);
            if (maintenanceInfo == null || !maintenanceInfo.IsActive)
            {
                _logger.LogWarning("No active maintenance found for world {World}", world);
                return 0;
            }

            // Find all game servers for this world by scanning Redis heartbeat keys
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
            {
                _logger.LogWarning("Redis unified connection not available for world {World}", world);
                return 0;
            }

            var heartbeatPattern = $"fb:heart-beat:{world}:Game:*";
            var keys = await redis.Connection.ScanKeysAsync(heartbeatPattern, 1000);

            if (keys.Count == 0)
            {
                _logger.LogWarning("No running game servers found for world {World}", world);
                return 0;
            }

            // Extract server IDs from heartbeat keys (format: fb:heart-beat:World:Service:Id)
            var serverIds = new HashSet<byte>();
            foreach (var key in keys)
            {
                var keyStr = key.ToString();
                var parts = keyStr.Split(':');
                if (parts.Length == 5 && byte.TryParse(parts[4], out var serverId))
                {
                    serverIds.Add(serverId);
                }
            }

            if (serverIds.Count == 0)
            {
                _logger.LogWarning("No valid game server IDs found for world {World}", world);
                return 0;
            }

            // Send StartMaintenance message to each game server
            var messageCount = 0;
            foreach (var serverId in serverIds)
            {
                try
                {
                    await _rabbitMqService.PublishAsync(new fb.protocol._internal.response.StartMaintenance
                    {
                        Message = maintenanceInfo.Message,
                        EndTime = maintenanceInfo.EndTime.ToString("yyyy-MM-dd HH:mm:ss")
                    }, AmqpRoute.Exchange, AmqpRoute.Unicast(world, serverId));

                    messageCount++;
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to send maintenance message to game server {ServerId} in world {World}", serverId, world);
                }
            }

            if (messageCount > 0)
            {
                _logger.LogInformation("Sent maintenance start message to {Count} game servers in world {World}", messageCount, world);
            }

            return messageCount;
        }

        private static string GetMaintenanceKey(uint world) => $"{MaintenanceKeyPrefix}{world}";

        private static double GetScoreForSchedule(MaintenanceSchedule schedule)
        {
            if (schedule.RepeatType != MaintenanceRepeatType.None)
                return RecurringScheduleScore;
            return ((DateTimeOffset)schedule.EndTime).ToUnixTimeSeconds();
        }

        private MaintenanceSchedule DeserializeSchedule(string json)
        {
            if (string.IsNullOrEmpty(json))
                return null;
            try
            {
                return JsonConvert.DeserializeObject<MaintenanceSchedule>(json);
            }
            catch
            {
                return null;
            }
        }

        private async Task<(MaintenanceSchedule schedule, string memberJson)> FindScheduleAndMember(uint world, string scheduleId)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return (null, null);

            var key = GetMaintenanceKey(world);
            var members = await redis.Connection.SortedSetRangeByRankAsync(key, 0, -1);
            foreach (var member in members)
            {
                var memberStr = member.ToString();
                var schedule = DeserializeSchedule(memberStr);
                if (schedule != null && schedule.Id == scheduleId)
                    return (schedule, memberStr);
            }

            return (null, null);
        }

        private async Task<MaintenanceSchedule> GetSchedule(uint world, string scheduleId)
        {
            var (schedule, _) = await FindScheduleAndMember(world, scheduleId);
            return schedule;
        }

        public async Task<int> RemoveExpiredSchedules(uint world, DateTime cutoff)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
                return 0;

            var key = GetMaintenanceKey(world);
            var members = await redis.Connection.SortedSetRangeByRankAsync(key, 0, -1);
            var toRemove = new List<RedisValue>();
            foreach (var member in members)
            {
                var schedule = DeserializeSchedule(member.ToString());
                if (schedule != null &&
                    schedule.RepeatType == MaintenanceRepeatType.None &&
                    schedule.EndTime < cutoff)
                {
                    toRemove.Add(member);
                }
            }

            foreach (var member in toRemove)
            {
                await redis.Connection.SortedSetRemoveAsync(key, member);
            }

            if (toRemove.Count > 0)
            {
                _logger.LogInformation("Removed {Count} expired one-time maintenance schedule(s) for world {World}", toRemove.Count, world);
            }

            return toRemove.Count;
        }

        private bool IsWithinMaintenanceWindow(MaintenanceSchedule schedule, DateTime checkTime)
        {
            var windowStart = GetWindowStart(schedule, checkTime);
            var windowEnd = GetWindowEnd(schedule, checkTime);
            return checkTime >= windowStart && checkTime <= windowEnd;
        }

        private DateTime GetWindowStart(MaintenanceSchedule schedule, DateTime checkTime)
        {
            return schedule.RepeatType switch
            {
                MaintenanceRepeatType.None => schedule.StartTime,
                MaintenanceRepeatType.Daily => checkTime.Date.Add(schedule.StartTime.TimeOfDay),
                MaintenanceRepeatType.Weekly => GetWeeklyWindowStart(schedule, checkTime),
                MaintenanceRepeatType.Monthly => GetMonthlyWindowStart(schedule, checkTime),
                MaintenanceRepeatType.Yearly => GetYearlyWindowStart(schedule, checkTime),
                _ => schedule.StartTime
            };
        }

        private DateTime GetWindowEnd(MaintenanceSchedule schedule, DateTime checkTime)
        {
            var windowStart = GetWindowStart(schedule, checkTime);
            var duration = schedule.EndTime - schedule.StartTime;
            var windowEnd = windowStart.Add(duration);

            // Handle wrap-around for daily/weekly schedules that span midnight
            if (schedule.RepeatType == MaintenanceRepeatType.Daily ||
                schedule.RepeatType == MaintenanceRepeatType.Weekly ||
                schedule.RepeatType == MaintenanceRepeatType.Monthly ||
                schedule.RepeatType == MaintenanceRepeatType.Yearly)
            {
                if (schedule.EndTime.TimeOfDay < schedule.StartTime.TimeOfDay)
                {
                    windowEnd = windowEnd.AddDays(1);
                }
            }

            return windowEnd;
        }

        private DateTime GetWeeklyWindowStart(MaintenanceSchedule schedule, DateTime checkTime)
        {
            var daysDiff = ((int)checkTime.DayOfWeek - (int)schedule.StartTime.DayOfWeek + 7) % 7;
            return checkTime.Date.AddDays(-daysDiff).Add(schedule.StartTime.TimeOfDay);
        }

        private DateTime GetMonthlyWindowStart(MaintenanceSchedule schedule, DateTime checkTime)
        {
            if (checkTime.Day != schedule.StartTime.Day)
                return DateTime.MinValue; // Not this month

            return checkTime.Date.Add(schedule.StartTime.TimeOfDay);
        }

        private DateTime GetYearlyWindowStart(MaintenanceSchedule schedule, DateTime checkTime)
        {
            if (checkTime.Month != schedule.StartTime.Month || checkTime.Day != schedule.StartTime.Day)
                return DateTime.MinValue; // Not this year

            return checkTime.Date.Add(schedule.StartTime.TimeOfDay);
        }
    }
}