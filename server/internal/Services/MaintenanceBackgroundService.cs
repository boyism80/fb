using Fb.Model.EnumValue;
using Http.Model;
using Http.Service;
using StackExchange.Redis;
using Response = fb.protocol.@internal.response;

namespace Internal.Services
{
    public class MaintenanceBackgroundService : BackgroundService
    {
        private readonly RedisService _redisService;
        private readonly MaintenanceService _maintenanceService;
        private readonly RabbitMqService _rabbitMqService;
        private readonly ILogger<MaintenanceBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromSeconds(10);
        private static readonly TimeSpan CleanupLockTtl = TimeSpan.FromSeconds(60);
        private static readonly TimeSpan NoticeClaimRetention = TimeSpan.FromDays(1);
        private const string CleanupLockKey = "fb:maintenance:background:lock";
        private const string NoticeKeyPrefix = "fb:maintenance:notice:";
        private const uint NoticeHost = 0xFFFFFFFF;

        public MaintenanceBackgroundService(
            RedisService redisService,
            MaintenanceService maintenanceService,
            RabbitMqService rabbitMqService,
            ILogger<MaintenanceBackgroundService> logger)
        {
            _redisService = redisService;
            _maintenanceService = maintenanceService;
            _rabbitMqService = rabbitMqService;
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    var redis = _redisService.GetUnifiedConnection();
                    if (redis != null)
                    {
                        await SendNoticesAsync(redis);

                        if (await redis.Connection.StringSetAsync(CleanupLockKey, "1", CleanupLockTtl, When.NotExists))
                        {
                            await _maintenanceService.RemoveExpiredSchedules(DateTime.UtcNow);
                        }
                    }
                }
                catch (Exception e)
                {
                    _logger.LogError(e, "Error in maintenance background service: {Message}", e.Message);
                }

                await Task.Delay(ProcessingInterval, stoppingToken);
            }
        }

        // Every internal instance runs this; the claim key lets only one of them send each notice.
        // A notice lost after its claim is not resent, because a resend could show it twice.
        private async Task SendNoticesAsync(Http.Service.Redis redis)
        {
            var now = DateTime.UtcNow;
            foreach (var schedule in await _maintenanceService.GetAllSchedules())
            {
                var noticeMinutes = schedule.NoticeMinutes.Where(x => x > 0).ToList();
                if (noticeMinutes.Count == 0)
                {
                    continue;
                }

                var start = MaintenanceService.FindNextStart(schedule, now, TimeSpan.FromMinutes(noticeMinutes.Max()));
                if (start == null)
                {
                    continue;
                }

                var remain = start.Value - now;
                var minutes = noticeMinutes.Where(x => remain <= TimeSpan.FromMinutes(x)).Min();
                var claimKey = $"{NoticeKeyPrefix}{schedule.Id}:{start.Value.Ticks}:{minutes}";
                if (!await redis.Connection.StringSetAsync(claimKey, "1", remain + NoticeClaimRetention, When.NotExists))
                {
                    continue;
                }

                var remainMinutes = (int)Math.Ceiling(remain.TotalMinutes);
                var when = remainMinutes >= 60 && remainMinutes % 60 == 0 ? $"{remainMinutes / 60}시간" : $"{remainMinutes}분";
                var text = string.IsNullOrWhiteSpace(schedule.Title)
                    ? $"{when} 뒤 점검이 시작됩니다."
                    : $"{when} 뒤 점검이 시작됩니다. ({schedule.Title})";
                var message = new Response.Broadcast
                {
                    Message = text,
                    Type = (byte)MessageType.Notify,
                    Host = NoticeHost,
                    Error = 0
                };

                var routes = new HashSet<string>();
                foreach (var label in schedule.Labels)
                {
                    if (label == MaintenanceLabel.All)
                    {
                        foreach (var world in _redisService.GetConfiguredWorlds())
                        {
                            routes.Add(AmqpRoute.Home("global", world));
                        }

                        routes.Add(AmqpRoute.Home("global", null));
                    }
                    else if (label == MaintenanceLabel.Cross)
                    {
                        routes.Add(AmqpRoute.Home("global", null));
                    }
                    else if (MaintenanceLabel.TryParseWorld(label, out var world))
                    {
                        routes.Add(AmqpRoute.Home("global", world));
                    }
                }

                foreach (var route in routes)
                {
                    await _rabbitMqService.PublishAsync(message, AmqpRoute.Exchange, route);
                }
            }
        }
    }
}
