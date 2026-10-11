using StackExchange.Redis;

namespace Http.Service
{
    public class PasswordAttemptService
    {
        private const int MaxFailuresPerIp = 5;
        private static readonly TimeSpan IpLockDuration = TimeSpan.FromMinutes(10);
        private const int MaxUntrustedFailuresPerAccount = 20;
        private static readonly TimeSpan AccountWindow = TimeSpan.FromHours(1);
        private static readonly TimeSpan TrustedIpDuration = TimeSpan.FromDays(30);
        private const string UnknownIp = "unknown";

        private const string BeginScript = @"
local ip = redis.call('INCR', KEYS[1])
if ip == 1 or ip == tonumber(ARGV[1]) then
    redis.call('PEXPIRE', KEYS[1], ARGV[2])
end
if ip > tonumber(ARGV[1]) then
    return 0
end
if redis.call('EXISTS', KEYS[3]) == 1 then
    return 1
end
local account = redis.call('INCR', KEYS[2])
if account == 1 then
    redis.call('PEXPIRE', KEYS[2], ARGV[4])
end
if account > tonumber(ARGV[3]) then
    return 0
end
return 1";

        private const string SucceedScript = @"
redis.call('DEL', KEYS[1])
if redis.call('EXISTS', KEYS[3]) == 0 and redis.call('EXISTS', KEYS[2]) == 1 then
    redis.call('DECR', KEYS[2])
end
if ARGV[2] == '1' then
    redis.call('SET', KEYS[3], '1', 'PX', ARGV[1])
end
return 1";

        private readonly RedisService _redisService;

        public PasswordAttemptService(RedisService redisService)
        {
            _redisService = redisService;
        }

        private static RedisKey[] Keys(uint world, uint uid, string ip) =>
        [
            $"fb:account:pw-fail:{world}:{uid}:{ip}",
            $"fb:account:pw-fail:{world}:{uid}",
            $"fb:account:pw-trust:{world}:{uid}:{ip}"
        ];

        public async Task<bool> Begin(uint world, uint uid, string ip)
        {
            ip = string.IsNullOrEmpty(ip) ? UnknownIp : ip;
            var keys = Keys(world, uid, ip);
            var redis = _redisService.GetShardConnection(world, keys[1].ToString());
            if (redis == null)
                return true;

            var result = await redis.EvalAsync(BeginScript, keys,
            [
                MaxFailuresPerIp,
                (long)IpLockDuration.TotalMilliseconds,
                MaxUntrustedFailuresPerAccount,
                (long)AccountWindow.TotalMilliseconds
            ]);
            return (long)result == 1;
        }

        public async Task Succeed(uint world, uint uid, string ip)
        {
            var trusted = string.IsNullOrEmpty(ip) == false;
            ip = trusted ? ip : UnknownIp;
            var keys = Keys(world, uid, ip);
            var redis = _redisService.GetShardConnection(world, keys[1].ToString());
            if (redis == null)
                return;

            await redis.EvalAsync(SucceedScript, keys,
            [
                (long)TrustedIpDuration.TotalMilliseconds,
                trusted ? "1" : "0"
            ]);
        }
    }
}
