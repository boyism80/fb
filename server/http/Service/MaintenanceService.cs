using Http.Model;
using Http.Redis;
using Newtonsoft.Json;
using StackExchange.Redis;

namespace Http.Service
{
    public class MaintenanceService
    {
        private const string ScheduleKey = "fb:maintenance";

        private static readonly double RecurringScheduleScore = ((DateTimeOffset)new DateTime(2099, 12, 31, 23, 59, 59, DateTimeKind.Utc)).ToUnixTimeSeconds();

        private static readonly TimeSpan CancelledScheduleRetention = TimeSpan.FromDays(7);

        private static readonly TimeZoneInfo OperatingTimeZone = TimeZoneInfo.FindSystemTimeZoneById("Asia/Seoul");

        private readonly RedisService _redisService;
        private readonly ILogger<MaintenanceService> _logger;

        public MaintenanceService(RedisService redisService, ILogger<MaintenanceService> logger)
        {
            _redisService = redisService;
            _logger = logger;
        }

        public async Task<List<MaintenanceInfo>> GetActiveMaintenances(DateTime? checkTime = null)
        {
            var now = checkTime ?? DateTime.UtcNow;
            var result = new List<MaintenanceInfo>();
            foreach (var (schedule, _) in await ReadSchedules())
            {
                if (!TryGetActiveWindow(schedule, now, out var start, out var end))
                {
                    continue;
                }

                result.Add(new MaintenanceInfo
                {
                    Labels = schedule.Labels,
                    Message = schedule.Message,
                    StartTime = start,
                    EndTime = end,
                    FinishMatches = schedule.FinishMatches,
                    ScheduleId = schedule.Id
                });
            }

            return result;
        }

        public async Task<MaintenanceInfo> FindActiveMaintenance(string label, DateTime? checkTime = null)
        {
            var maintenances = await GetActiveMaintenances(checkTime);
            return maintenances.FirstOrDefault(x => MaintenanceLabel.Matches(x.Labels, label));
        }

        public async Task<bool> CreateMaintenanceSchedule(MaintenanceSchedule schedule)
        {
            if (string.IsNullOrEmpty(schedule.Id))
            {
                schedule.Id = Guid.NewGuid().ToString();
            }

            schedule.CreatedAt = DateTime.UtcNow;
            schedule.IsActive = true;
            schedule.Revision = 0;
            schedule.Occurrence = null;

            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
            {
                return false;
            }

            try
            {
                await redis.Connection.SortedSetAddAsync(ScheduleKey, JsonConvert.SerializeObject(schedule), GetScoreForSchedule(schedule));
                _logger.LogInformation("Created maintenance schedule {ScheduleId} for {Labels}", schedule.Id, string.Join(",", schedule.Labels));
                return true;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to create maintenance schedule {ScheduleId}", schedule.Id);
                return false;
            }
        }

        // A started window keeps its start, labels and repeat type; moving them would end and restart the maintenance.
        // started is what the caller saw; a window that began or ended since then fails the update.
        public async Task<bool> UpdateMaintenanceSchedule(MaintenanceSchedule updated, bool started)
        {
            var (current, member) = await FindScheduleAndMember(updated.Id);
            if (current == null || !current.IsActive || current.Revision != updated.Revision)
            {
                return false;
            }

            var now = DateTime.UtcNow;
            if (TryGetActiveWindow(current, now, out var start, out _) != started)
            {
                return false;
            }

            var next = JsonConvert.DeserializeObject<MaintenanceSchedule>(member);
            next.Title = updated.Title;
            next.Message = updated.Message;
            next.FinishMatches = updated.FinishMatches;
            next.NoticeMinutes = updated.NoticeMinutes;
            if (started)
            {
                var end = updated.EndTime < now ? now : updated.EndTime;
                if (current.RepeatType == MaintenanceRepeatType.None)
                {
                    next.EndTime = end;
                }
                else
                {
                    next.Occurrence = new MaintenanceOccurrence { Start = start, End = end };
                }
            }
            else
            {
                if (updated.EndTime <= updated.StartTime || updated.Labels.Count == 0)
                {
                    return false;
                }

                next.Labels = updated.Labels;
                next.StartTime = updated.StartTime;
                next.EndTime = updated.EndTime;
                next.RepeatType = updated.RepeatType;
            }
            next.Revision = current.Revision + 1;

            return await ReplaceSchedule(member, next);
        }

        public async Task<bool> EndMaintenanceSchedule(string scheduleId, int revision)
        {
            var (schedule, member) = await FindScheduleAndMember(scheduleId);
            if (schedule == null || schedule.Revision != revision)
            {
                return false;
            }

            var now = DateTime.UtcNow;
            if (!TryGetActiveWindow(schedule, now, out var start, out _))
            {
                return false;
            }

            if (schedule.RepeatType == MaintenanceRepeatType.None)
            {
                schedule.EndTime = now;
            }
            else
            {
                schedule.Occurrence = new MaintenanceOccurrence { Start = start, End = now };
            }

            schedule.Revision++;
            return await ReplaceSchedule(member, schedule);
        }

        public async Task<bool> CancelMaintenanceSchedule(string scheduleId)
        {
            var (schedule, member) = await FindScheduleAndMember(scheduleId);
            if (schedule == null || !schedule.IsActive)
            {
                return false;
            }

            schedule.IsActive = false;
            schedule.CancelledAt = DateTime.UtcNow;
            schedule.Revision++;
            return await ReplaceSchedule(member, schedule);
        }

        public async Task<List<MaintenanceSchedule>> GetAllSchedules()
        {
            var schedules = await ReadSchedules();
            return schedules.Select(x => x.schedule).OrderByDescending(s => s.CreatedAt).ToList();
        }

        // Removes by the exact stored member, so a schedule edited after it was read here is left alone.
        public async Task<int> RemoveExpiredSchedules(DateTime cutoff)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
            {
                return 0;
            }

            var removed = 0;
            foreach (var (schedule, member) in await ReadSchedules())
            {
                var expired = schedule.RepeatType == MaintenanceRepeatType.None && schedule.EndTime < cutoff;
                var cancelledLongAgo = !schedule.IsActive &&
                    schedule.CancelledAt.HasValue &&
                    schedule.CancelledAt.Value < cutoff - CancelledScheduleRetention;
                if ((expired || cancelledLongAgo) && await redis.Connection.SortedSetRemoveAsync(ScheduleKey, member))
                {
                    removed++;
                }
            }

            if (removed > 0)
            {
                _logger.LogInformation("Removed {Count} expired or cancelled maintenance schedule(s)", removed);
            }

            return removed;
        }

        public static bool TryGetActiveWindow(MaintenanceSchedule schedule, DateTime now, out DateTime start, out DateTime end)
        {
            start = DateTime.MinValue;
            end = DateTime.MinValue;
            if (!schedule.IsActive)
            {
                return false;
            }

            var occurrence = schedule.Occurrence;
            if (occurrence != null && occurrence.Start <= now && now <= occurrence.End)
            {
                start = occurrence.Start;
                end = occurrence.End;
                return true;
            }

            var windowStart = GetWindowStart(schedule, now);
            if (windowStart == DateTime.MinValue || (occurrence != null && windowStart == occurrence.Start))
            {
                return false;
            }

            var windowEnd = GetWindowEnd(schedule, windowStart);
            if (now < windowStart || now > windowEnd)
            {
                return false;
            }

            start = windowStart;
            end = windowEnd;
            return true;
        }

        public static DateTime? FindNextStart(MaintenanceSchedule schedule, DateTime now, TimeSpan horizon)
        {
            if (!schedule.IsActive)
            {
                return null;
            }

            if (schedule.RepeatType == MaintenanceRepeatType.None)
            {
                return schedule.StartTime > now && schedule.StartTime - now <= horizon ? schedule.StartTime : null;
            }

            var template = ToOperating(schedule.StartTime);
            var operatingNow = ToOperating(now);
            for (var day = operatingNow.Date; day <= (operatingNow + horizon).Date; day = day.AddDays(1))
            {
                var start = day.Add(template.TimeOfDay);
                var startUtc = ToUtc(start);
                if (startUtc <= now || startUtc - now > horizon)
                {
                    continue;
                }

                var matched = schedule.RepeatType switch
                {
                    MaintenanceRepeatType.Daily => true,
                    MaintenanceRepeatType.Weekly => start.DayOfWeek == template.DayOfWeek,
                    MaintenanceRepeatType.Monthly => start.Day == template.Day,
                    MaintenanceRepeatType.Yearly => start.Month == template.Month && start.Day == template.Day,
                    _ => false
                };
                if (matched)
                {
                    return startUtc;
                }
            }

            return null;
        }

        // Admins enter recurring schedules in KST; month days and weekdays must be matched there, not in UTC.
        private static DateTime ToOperating(DateTime utc) =>
            TimeZoneInfo.ConvertTimeFromUtc(DateTime.SpecifyKind(utc, DateTimeKind.Utc), OperatingTimeZone);

        private static DateTime ToUtc(DateTime operating) =>
            TimeZoneInfo.ConvertTimeToUtc(DateTime.SpecifyKind(operating, DateTimeKind.Unspecified), OperatingTimeZone);

        private async Task<bool> ReplaceSchedule(string member, MaintenanceSchedule next)
        {
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
            {
                return false;
            }

            try
            {
                var transaction = redis.Connection.CreateTransaction();
                transaction.AddCondition(Condition.SortedSetContains(ScheduleKey, member));
                _ = transaction.SortedSetRemoveAsync(ScheduleKey, member);
                _ = transaction.SortedSetAddAsync(ScheduleKey, JsonConvert.SerializeObject(next), GetScoreForSchedule(next));
                return await transaction.ExecuteAsync();
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to update maintenance schedule {ScheduleId}", next.Id);
                return false;
            }
        }

        private async Task<List<(MaintenanceSchedule schedule, string member)>> ReadSchedules()
        {
            var result = new List<(MaintenanceSchedule, string)>();
            var redis = _redisService.GetUnifiedConnection();
            if (redis == null)
            {
                return result;
            }

            var members = await redis.Connection.SortedSetRangeByRankAsync(ScheduleKey, 0, -1);
            foreach (var member in members)
            {
                var memberStr = member.ToString();
                var schedule = DeserializeSchedule(memberStr);
                if (schedule != null)
                {
                    result.Add((schedule, memberStr));
                }
            }

            return result;
        }

        private async Task<(MaintenanceSchedule schedule, string member)> FindScheduleAndMember(string scheduleId)
        {
            var schedules = await ReadSchedules();
            return schedules.FirstOrDefault(x => x.schedule.Id == scheduleId);
        }

        private static double GetScoreForSchedule(MaintenanceSchedule schedule)
        {
            if (schedule.RepeatType != MaintenanceRepeatType.None)
            {
                return RecurringScheduleScore;
            }

            return ((DateTimeOffset)schedule.EndTime).ToUnixTimeSeconds();
        }

        private static MaintenanceSchedule DeserializeSchedule(string json)
        {
            if (string.IsNullOrEmpty(json))
            {
                return null;
            }

            try
            {
                return JsonConvert.DeserializeObject<MaintenanceSchedule>(json);
            }
            catch
            {
                return null;
            }
        }

        // Returns the start of the latest occurrence that began at or before checkTime, so a window that
        // crossed midnight is still found after the date changes. DateTime.MinValue means no occurrence.
        private static DateTime GetWindowStart(MaintenanceSchedule schedule, DateTime checkTime)
        {
            if (schedule.RepeatType == MaintenanceRepeatType.None)
            {
                return schedule.StartTime;
            }

            var template = ToOperating(schedule.StartTime);
            var now = ToOperating(checkTime);
            var duration = GetWindowEnd(schedule, now) - now;
            var start = schedule.RepeatType switch
            {
                MaintenanceRepeatType.Daily => GetDailyWindowStart(template, now),
                MaintenanceRepeatType.Weekly => GetWeeklyWindowStart(template, now),
                MaintenanceRepeatType.Monthly => GetDatedWindowStart(template, now, duration, false),
                MaintenanceRepeatType.Yearly => GetDatedWindowStart(template, now, duration, true),
                _ => DateTime.MinValue
            };
            return start == DateTime.MinValue ? start : ToUtc(start);
        }

        private static DateTime GetWindowEnd(MaintenanceSchedule schedule, DateTime windowStart)
        {
            var duration = schedule.EndTime - schedule.StartTime;
            if (schedule.RepeatType != MaintenanceRepeatType.None && duration < TimeSpan.Zero)
            {
                duration = duration.Add(TimeSpan.FromDays(1));
            }

            return windowStart.Add(duration);
        }

        private static DateTime GetDailyWindowStart(DateTime template, DateTime checkTime)
        {
            var today = checkTime.Date.Add(template.TimeOfDay);
            if (checkTime >= today)
            {
                return today;
            }
            else
            {
                return today.AddDays(-1);
            }
        }

        private static DateTime GetWeeklyWindowStart(DateTime template, DateTime checkTime)
        {
            var daysDiff = ((int)checkTime.DayOfWeek - (int)template.DayOfWeek + 7) % 7;
            var start = checkTime.Date.AddDays(-daysDiff).Add(template.TimeOfDay);
            if (checkTime >= start)
            {
                return start;
            }
            else
            {
                return start.AddDays(-7);
            }
        }

        private static DateTime GetDatedWindowStart(DateTime template, DateTime checkTime, TimeSpan duration, bool matchMonth)
        {
            for (var day = checkTime.Date; day >= checkTime.Subtract(duration).Date; day = day.AddDays(-1))
            {
                var start = day.Add(template.TimeOfDay);
                if (start <= checkTime && day.Day == template.Day && (matchMonth == false || day.Month == template.Month))
                {
                    return start;
                }
            }

            return DateTime.MinValue;
        }
    }
}
