using StackExchange.Redis;

namespace Http.Service
{
    public enum PasswordCheck
    {
        Locked,
        NotMatched,
        Matched,
    }

    public class PasswordAttemptService
    {
        private enum PasswordAttempt
        {
            Locked,
            Allowed,
            Counted,
        }

        private const int MaxFailuresPerIp = 5;
        private static readonly TimeSpan IpLockDuration = TimeSpan.FromMinutes(10);
        private const int MaxUntrustedFailuresPerAccount = 20;
        private static readonly TimeSpan AccountWindow = TimeSpan.FromHours(1);
        private static readonly TimeSpan TrustedIpDuration = TimeSpan.FromDays(30);
        private const int MaxBirthdayFailures = 5;
        private static readonly TimeSpan BirthdayWindow = TimeSpan.FromHours(24);
        private const string UnknownIp = "unknown";

        // Returns 0 when locked, 1 when allowed without counting the account, 2 when the account was counted.
        private const string BeginScript = @"
if tonumber(redis.call('GET', KEYS[1]) or '0') >= tonumber(ARGV[1]) then
    return 0
end
local ip = redis.call('INCR', KEYS[1])
if ip == 1 or ip == tonumber(ARGV[1]) then
    redis.call('PEXPIRE', KEYS[1], ARGV[2])
end
local trusted = tonumber(redis.call('GET', KEYS[3]) or '-1')
if trusted >= tonumber(redis.call('GET', KEYS[4]) or '0') then
    return 1
end
if tonumber(redis.call('GET', KEYS[2]) or '0') >= tonumber(ARGV[3]) then
    return 0
end
if redis.call('INCR', KEYS[2]) == 1 then
    redis.call('PEXPIRE', KEYS[2], ARGV[4])
end
return 2";

        // Undoes the account count only when Begin made it: the trust key can appear or expire in between.
        private const string SucceedScript = @"
redis.call('DEL', KEYS[1])
if ARGV[3] == '1' and redis.call('EXISTS', KEYS[2]) == 1 then
    if redis.call('DECR', KEYS[2]) <= 0 then
        redis.call('DEL', KEYS[2])
    end
end
if ARGV[2] == '1' then
    local now = redis.call('TIME')
    redis.call('SET', KEYS[3], string.format('%d', now[1] * 1000 + math.floor(now[2] / 1000)), 'PX', ARGV[1])
end
return 1";

        private const string PasswordChangedScript = @"
local now = redis.call('TIME')
local changed = string.format('%d', now[1] * 1000 + math.floor(now[2] / 1000))
redis.call('SET', KEYS[4], changed, 'PX', ARGV[1])
if ARGV[2] == '1' then
    redis.call('SET', KEYS[3], changed, 'PX', ARGV[1])
end
return 1";

        private const string BirthdayFailScript = @"
if redis.call('INCR', KEYS[1]) == 1 then
    redis.call('PEXPIRE', KEYS[1], ARGV[1])
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
            $"fb:account:pw-trust:{world}:{uid}:{ip}",
            $"fb:account:pw-changed:{world}:{uid}"
        ];

        public async Task<(PasswordCheck Result, string UpgradedPw)> CheckPassword(uint world, uint uid, string ip, string storedPw, string password)
        {
            var attempt = await Begin(world, uid, ip);
            if (attempt == PasswordAttempt.Locked)
            {
                return (PasswordCheck.Locked, null);
            }

            if (PasswordHash.Matches(storedPw, password, out var upgradedPw) == false)
            {
                return (PasswordCheck.NotMatched, null);
            }

            await Succeed(world, uid, ip, attempt);
            return (PasswordCheck.Matched, upgradedPw);
        }

        public async Task<PasswordCheck> CheckBirthday(uint world, uint uid, uint? storedBirth, uint birth)
        {
            if (await BirthdayLocked(world, uid))
            {
                return PasswordCheck.Locked;
            }

            if (storedBirth != birth)
            {
                await BirthdayFailed(world, uid);
                return PasswordCheck.NotMatched;
            }

            return PasswordCheck.Matched;
        }

        private async Task<PasswordAttempt> Begin(uint world, uint uid, string ip)
        {
            ip = string.IsNullOrEmpty(ip) ? UnknownIp : ip;
            var keys = Keys(world, uid, ip);
            var redis = _redisService.GetShardConnection(world, keys[1].ToString());
            if (redis == null)
            {
                return PasswordAttempt.Allowed;
            }

            var result = await redis.EvalAsync(BeginScript, keys,
            [
                MaxFailuresPerIp,
                (long)IpLockDuration.TotalMilliseconds,
                MaxUntrustedFailuresPerAccount,
                (long)AccountWindow.TotalMilliseconds
            ]);
            return (long)result switch
            {
                0 => PasswordAttempt.Locked,
                2 => PasswordAttempt.Counted,
                _ => PasswordAttempt.Allowed,
            };
        }

        private async Task Succeed(uint world, uint uid, string ip, PasswordAttempt attempt)
        {
            var trusted = string.IsNullOrEmpty(ip) == false;
            ip = trusted ? ip : UnknownIp;
            var keys = Keys(world, uid, ip);
            var redis = _redisService.GetShardConnection(world, keys[1].ToString());
            if (redis == null)
            {
                return;
            }

            await redis.EvalAsync(SucceedScript, keys,
            [
                (long)TrustedIpDuration.TotalMilliseconds,
                trusted ? "1" : "0",
                attempt == PasswordAttempt.Counted ? "1" : "0"
            ]);
        }

        public async Task PasswordChanged(uint world, uint uid, string ip)
        {
            await BirthdaySucceeded(world, uid);

            var trusted = string.IsNullOrEmpty(ip) == false;
            ip = trusted ? ip : UnknownIp;
            var keys = Keys(world, uid, ip);
            var redis = _redisService.GetShardConnection(world, keys[1].ToString());
            if (redis == null)
            {
                return;
            }

            await redis.EvalAsync(PasswordChangedScript, keys,
            [
                (long)TrustedIpDuration.TotalMilliseconds,
                trusted ? "1" : "0"
            ]);
        }

        private async Task<bool> BirthdayLocked(uint world, uint uid)
        {
            var key = BirthdayKey(world, uid);
            var redis = _redisService.GetShardConnection(world, key);
            if (redis == null)
            {
                return false;
            }

            var failures = await redis.Connection.StringGetAsync(key);
            return failures.HasValue && (long)failures >= MaxBirthdayFailures;
        }

        private async Task BirthdayFailed(uint world, uint uid)
        {
            var key = BirthdayKey(world, uid);
            var redis = _redisService.GetShardConnection(world, key);
            if (redis == null)
            {
                return;
            }

            await redis.EvalAsync(BirthdayFailScript, [key], [(long)BirthdayWindow.TotalMilliseconds]);
        }

        private async Task BirthdaySucceeded(uint world, uint uid)
        {
            var key = BirthdayKey(world, uid);
            var redis = _redisService.GetShardConnection(world, key);
            if (redis == null)
            {
                return;
            }

            await redis.Connection.KeyDeleteAsync(key);
        }

        private static string BirthdayKey(uint world, uint uid) => $"fb:account:birth-fail:{world}:{uid}";
    }
}
