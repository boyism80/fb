using Fb.Model.EnumValue;
using Http.Service;
using Microsoft.Extensions.ObjectPool;
using System.Buffers;
using System.Security.Cryptography;
using System.Text;

namespace AdminTool.Services
{
    /// <summary>
    /// Provides authentication services for the admin tool.
    /// Handles user login and role verification.
    /// </summary>
    public class AuthService
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<AuthService> _logger;
        private readonly LogService _logService;

        // Object pool for SHA256 instances to reduce GC pressure
        private static readonly ObjectPool<SHA256> _sha256Pool = new DefaultObjectPool<SHA256>(new Sha256PooledObjectPolicy(), 64);

        /// <summary>
        /// Initializes a new instance of the <see cref="AuthService"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="logger">The logger for recording authentication operations.</param>
        /// <param name="logService">The log service for recording admin login events.</param>
        public AuthService(DbContext dbContext, ILogger<AuthService> logger, LogService logService)
        {
            _dbContext = dbContext;
            _logger = logger;
            _logService = logService;
        }

        /// <summary>
        /// Computes a SHA256 hash of the input string.
        /// Used for password hashing and security operations.
        /// Thread-safe implementation using object pooling to reduce GC pressure.
        /// </summary>
        /// <param name="value">The string value to hash.</param>
        /// <returns>A hexadecimal string representation of the SHA256 hash.</returns>
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

        /// <summary>
        /// Authenticates a user with their character name and password.
        /// Verifies credentials and checks if the user has admin or higher role.
        /// </summary>
        /// <param name="section">The section identifier (e.g., "section-1", "unified-global").</param>
        /// <param name="name">The character name to authenticate.</param>
        /// <param name="password">The password to verify.</param>
        /// <returns>A result object containing authentication status and user information.</returns>
        public async Task<AuthResult> Authenticate(string section, string name, string password)
        {
            // Get character by name
            var userId = await _dbContext.Character.GetCharacterId(section, name);
            if (!userId.HasValue)
            {
                return new AuthResult
                {
                    Success = false,
                    Error = "Character not found"
                };
            }

            var character = await _dbContext.Character.Get(section, userId.Value);
            if (character == null)
            {
                return new AuthResult
                {
                    Success = false,
                    Error = "Character not found"
                };
            }

            // Verify password
            if (character.Pw != SHA256Hash(password))
            {
                return new AuthResult
                {
                    Success = false,
                    Error = "Invalid password"
                };
            }

            // Check role (must be Admin or higher)
            if (character.Role < Role.Admin)
            {
                return new AuthResult
                {
                    Success = false,
                    Error = "Insufficient privileges. Admin role or higher required."
                };
            }

            // Log admin login event
            _logService.Write("admin_login", new
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

        /// <summary>
        /// Object pool policy for SHA256 instances.
        /// Handles creation and reset of SHA256 objects for reuse.
        /// </summary>
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

    /// <summary>
    /// Represents the result of an authentication operation.
    /// </summary>
    public class AuthResult
    {
        /// <summary>
        /// Gets or sets a value indicating whether the authentication was successful.
        /// </summary>
        public bool Success { get; set; }

        /// <summary>
        /// Gets or sets the error message if authentication failed.
        /// </summary>
        public string Error { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the user ID if authentication was successful.
        /// </summary>
        public uint? UserId { get; set; }

        /// <summary>
        /// Gets or sets the user name if authentication was successful.
        /// </summary>
        public string UserName { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the user role if authentication was successful.
        /// </summary>
        public Fb.Model.EnumValue.Role? Role { get; set; }
    }
}

