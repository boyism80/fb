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
                var claimKey = MaintenanceService.NoticeClaimKey(schedule.Id, start.Value, minutes);
                if (!await redis.Connection.StringSetAsync(claimKey, "1", remain + NoticeClaimRetention, When.NotExists))
                {
                    continue;
                }

                var message = new Response.Broadcast
                {
                    Message = MaintenanceService.StartNotice(schedule, remain),
                    Type = (byte)MessageType.Notify,
                    Host = MaintenanceService.NoticeHost,
                    Error = 0
                };

                foreach (var route in _maintenanceService.NoticeRoutes(schedule))
                {
                    await _rabbitMqService.PublishAsync(message, AmqpRoute.Exchange, route);
                }
            }
        }
    }
}
