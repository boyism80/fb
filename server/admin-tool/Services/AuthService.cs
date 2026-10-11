using Fb.Model.EnumValue;
using Http.Service;
using Microsoft.Extensions.ObjectPool;
using System.Buffers;
using System.Security.Cryptography;
using System.Text;

namespace AdminTool.Services
{
    public class AuthService
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<AuthService> _logger;
        private readonly LogService _logService;
        private readonly PasswordAttemptService _passwordAttempts;

        // Object pool for SHA256 instances to reduce GC pressure
        private static readonly ObjectPool<SHA256> _sha256Pool = new DefaultObjectPool<SHA256>(new Sha256PooledObjectPolicy(), 64);

        public AuthService(DbContext dbContext, ILogger<AuthService> logger, LogService logService, PasswordAttemptService passwordAttempts)
        {
            _dbContext = dbContext;
            _logger = logger;
            _logService = logService;
            _passwordAttempts = passwordAttempts;
        }

        private static string SHA256Hash(string value)
        {
            var sha256 = _sha256Pool.Get();
            try
            {
                var hash = sha256.ComputeHash(Encoding.ASCII.GetBytes(value));

                // Use ArrayPool to reduce allocations for StringBuilder buffer
                var buffer = ArrayPool<char>.Shared.Rent(hash.Length * 2);
                try
                {
                    var span = buffer.AsSpan(0, hash.Length * 2);

                    // Clear the buffer to ensure no leftover data from previous uses
                    span.Clear();

                    for (int i = 0; i < hash.Length; i++)
                    {
                        // Check if TryFormat succeeds and handle failure
                        if (!hash[i].TryFormat(span.Slice(i * 2, 2), out _, "x2"))
                        {
                            throw new InvalidOperationException($"Failed to format byte {hash[i]} at index {i}");
                        }
                    }
                    return new string(span);
                }
                finally
                {
                    ArrayPool<char>.Shared.Return(buffer);
                }
            }
            finally
            {
                _sha256Pool.Return(sha256);
            }
        }

        private const string LoginFailedError = "캐릭터명 또는 비밀번호가 올바르지 않습니다. 실패가 반복되면 잠시 후 다시 시도해 주세요.";

        public async Task<AuthResult> Authenticate(uint world, string name, string password, string ip)
        {
            var userId = await _dbContext.Character.GetCharacterId(world, name);
            if (!userId.HasValue)
                return new AuthResult { Error = LoginFailedError };

            var character = await _dbContext.Character.Get(world, userId.Value);
            if (character == null)
                return new AuthResult { Error = LoginFailedError };

            if (character.Role < Role.Admin)
                return new AuthResult { Error = LoginFailedError };

            if (await _passwordAttempts.Begin(world, userId.Value, ip) == false)
                return new AuthResult { Error = LoginFailedError };

            if (character.Pw != SHA256Hash(password))
                return new AuthResult { Error = LoginFailedError };

            await _passwordAttempts.Succeed(world, userId.Value, ip);

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

        private class Sha256PooledObjectPolicy : PooledObjectPolicy<SHA256>
        {
            public override SHA256 Create()
            {
                return SHA256.Create();
            }

            public override bool Return(SHA256 obj)
            {
                if (obj == null)
                    return false;

                // Reset the SHA256 instance state for reuse
                try
                {
                    obj.ComputeHash(Array.Empty<byte>());
                    return true;
                }
                catch
                {
                    // If there's any issue with the instance, don't return it to the pool
                    obj.Dispose();
                    return false;
                }
            }
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
