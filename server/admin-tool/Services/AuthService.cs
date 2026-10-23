using Fb.Model.EnumValue;
using Http.Model;
using Http.Service;

namespace AdminTool.Services
{
    public class AuthService
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<AuthService> _logger;
        private readonly LogService _logService;
        private readonly PasswordAttemptService _passwordAttempts;
        private readonly RedisDistributedLockService _distributedLock;

        public AuthService(DbContext dbContext, ILogger<AuthService> logger, LogService logService, PasswordAttemptService passwordAttempts, RedisDistributedLockService distributedLock)
        {
            _dbContext = dbContext;
            _logger = logger;
            _logService = logService;
            _passwordAttempts = passwordAttempts;
            _distributedLock = distributedLock;
        }

        private const string LoginFailedError = "캐릭터명 또는 비밀번호가 올바르지 않습니다. 실패가 반복되면 잠시 후 다시 시도해 주세요.";

        public async Task<AuthResult> Authenticate(uint world, string name, string password, string ip)
        {
            var userId = await _dbContext.Character.GetCharacterId(world, name);
            if (!userId.HasValue)
            {
                return new AuthResult { Error = LoginFailedError };
            }

            var character = await _dbContext.Character.Get(world, userId.Value);
            if (character == null)
            {
                return new AuthResult { Error = LoginFailedError };
            }

            if (character.Role < Role.Admin)
            {
                return new AuthResult { Error = LoginFailedError };
            }

            var attempt = await _passwordAttempts.Begin(world, userId.Value, ip);
            if (attempt == PasswordAttempt.Locked)
            {
                return new AuthResult { Error = LoginFailedError };
            }

            if (PasswordHash.Matches(character.Pw, password, out var updatedPw) == false)
            {
                return new AuthResult { Error = LoginFailedError };
            }

            await _passwordAttempts.Succeed(world, userId.Value, ip, attempt);
            if (updatedPw != null)
            {
                await using var _ = await _distributedLock.Lock(world, Character.SaveLockKey(userId.Value));
                var current = await _dbContext.Character.Get(world, userId.Value);
                if (current != null && current.Pw == character.Pw)
                {
                    current.Pw = updatedPw;
                    _dbContext.Character.Set(world, current);
                    await _dbContext.SaveChangesAsync();
                }
            }

            // Log admin login event
            await _logService.WriteAsync("admin_login", new
            {
                account_name = character.Name,
                uid = userId.Value,
                role = character.Role.ToString()
            });

            return new AuthResult
            {
                Success = true,
                UserId = userId.Value,
                UserName = character.Name,
                Role = character.Role
            };
        }
    }

    public class AuthResult
    {
        public bool Success { get; set; }

        public string Error { get; set; } = string.Empty;

        public uint? UserId { get; set; }

        public string UserName { get; set; } = string.Empty;

        public Fb.Model.EnumValue.Role? Role { get; set; }
    }
}
