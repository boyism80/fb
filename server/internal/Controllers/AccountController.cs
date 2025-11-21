using AutoMapper;
using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Http.Util;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.ObjectPool;
using System.Buffers;
using System.Data;
using System.Security.Cryptography;
using System.Text;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    /// <summary>
    /// Provides account management operations for the internal API.
    /// Handles user authentication, character creation, and password management.
    /// Used by the login server for account-related operations.
    /// </summary>
    [ApiController]
    [Route("account")]
    public class AccountController : ControllerBase
    {
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly RedisService _redisService;
        private readonly ILogger<AccountController> _logger;

        // Object pool for SHA256 instances to reduce GC pressure
        // Limit pool size to 64 instances (reasonable for most scenarios)
        private static readonly ObjectPool<SHA256> _sha256Pool = new DefaultObjectPool<SHA256>(new Sha256PooledObjectPolicy(), 64);

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
                // SHA256 doesn't have a Reset method, but we can ensure it's in a clean state
                // by creating a small dummy computation to clear any internal state
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


        /// <summary>
        /// Initializes a new instance of the <see cref="AccountController"/> class.
        /// </summary>
        /// <param name="mapper">The AutoMapper instance for object mapping.</param>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="logger">The logger for recording account operations.</param>
        public AccountController(IMapper mapper,
            DbContext dbContext,
            RedisService redisService,
            ILogger<AccountController> logger)
        {
            _mapper = mapper;
            _dbContext = dbContext;
            _redisService = redisService;
            _logger = logger;
        }

        /// <summary>
        /// Retrieves the unique identifier for a character by name.
        /// </summary>
        /// <param name="name">The character name to look up.</param>
        /// <returns>A response containing the character's UID if found.</returns>
        [HttpGet("uid/{name}")]
        public async Task<Response.GetUid> Uid(string name)
        {
            try
            {
                var uid = await _dbContext.Character.GetCharacterId(name) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);

                return new Response.GetUid
                {
                    Uid = uid,
                    Success = true
                };
            }
            catch (Exception)
            {
                return new Response.GetUid
                {
                    Success = false
                };
            }
        }

        /// <summary>
        /// Authenticates a user with their credentials.
        /// Verifies the user ID and password hash against stored values.
        /// </summary>
        /// <param name="request">The authentication request containing user credentials.</param>
        /// <returns>An authentication response with error code and map information.</returns>
        [HttpPost("authenticate")]
        public async Task<Response.Authenticate> Authenticate(Request.Authenticate request)
        {
            var ch = await _dbContext.Character.Get(request.Uid);
            if (ch == null)
            {
                return new Response.Authenticate
                {
                    ErrorCode = 1
                };
            }

            if (ch.Pw != SHA256Hash(request.Pw))
            {
                return new Response.Authenticate
                {
                    ErrorCode = 2
                };
            }

            return new Response.Authenticate
            {
                ErrorCode = 0,
                Map = ch.Map
            };
        }

        /// <summary>
        /// Reserves a character name for a new user.
        /// Uses a stored procedure to atomically check and reserve the name.
        /// </summary>
        /// <param name="request">The name reservation request.</param>
        /// <returns>A response indicating success and the assigned UID.</returns>
        [HttpPost("reserve")]
        public async Task<Response.ReserveName> ReserveName(Request.ReserveName request)
        {
            await using var connection = _dbContext.Connection(-1);
            var result = await connection.QueryFirstAsync<ReserveNameResult>("USP_NAME_SET", new
            {
                uname = request.Name
            }, commandType: CommandType.StoredProcedure);

            return new Response.ReserveName
            {
                Uid = result.Uid,
                Success = result.Result
            };
        }

        /// <summary>
        /// Initializes a new character with basic attributes.
        /// Creates the character record with initial stats and position.
        /// </summary>
        /// <param name="request">The character initialization request.</param>
        /// <returns>A response indicating the success of character creation.</returns>
        [HttpPost("init")]
        public async Task<Response.InitCharacter> InitCharacter(Request.InitCharacter request)
        {
            var ch = new Character
            {
                Id = request.Uid,
                Name = request.Name,
                Pw = SHA256Hash(request.Pw),
                Level = 1,
                Hp = request.Hp,
                BaseHp = request.Hp,
                Mp = request.Mp,
                BaseMp = request.Mp,
                Map = request.Map,
                PositionX = request.X,
                PositionY = request.Y,
                Direction = (byte)Direction.Bottom,
                Role = (Role)request.Role
            };
            _dbContext.Character.Set(ch);

            await _dbContext.SaveChangesAsync();
            return new Response.InitCharacter
            {
                Success = true
            };
        }

        /// <summary>
        /// Completes character creation by setting appearance attributes.
        /// Updates the character with visual characteristics like hair, sex, nation, and creature type.
        /// </summary>
        /// <param name="request">The character creation request with appearance data.</param>
        /// <returns>A response indicating the success of character completion.</returns>
        [HttpPost("make")]
        public async Task<Response.MakeCharacter> MakeCharacter(Request.MakeCharacter request)
        {
            try
            {
                var ch = await _dbContext.Character.Get(request.Uid) ??
                    throw new Exception($"user {request.Uid} not found");

                ch.Look = request.Hair;
                ch.Sex = request.Sex;
                ch.Nation = request.Nation;
                ch.Creature = request.Creature;
                _dbContext.Character.Set(ch);

                var baramTime = DateTime.Now.ToBaramTime();
                _dbContext.Achievement.Set(new Achievement
                {
                    Uid = request.Uid,
                    Model = 0,
                    Text = $"{baramTime} 생"
                });

                await _dbContext.SaveChangesAsync();
                return new Response.MakeCharacter
                {
                    Success = true
                };
            }
            catch (Exception)
            {
                return new Response.MakeCharacter
                {
                    Success = false
                };
            }
        }

        /// <summary>
        /// Changes a user's password after verifying current credentials and birthday.
        /// Validates the current password and birthday before updating to the new password.
        /// </summary>
        /// <param name="request">The password change request with verification data.</param>
        /// <returns>A response with error code indicating the result of the password change.</returns>
        [HttpPost("change-pw")]
        public async Task<Response.ChangePw> ChangePassword(Request.ChangePw request)
        {
            try
            {
                var ch = await _dbContext.Character.Get(request.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (ch.Pw != SHA256Hash(request.Before))
                    throw new LogicException(ErrorCode.PasswordNotMatched);

                if (ch.Birth != request.Birthday)
                    throw new LogicException(ErrorCode.BirthdayNotMatched);

                ch.Pw = SHA256Hash(request.After);
                _dbContext.Character.Set(ch);

                await _dbContext.SaveChangesAsync();
                return new Response.ChangePw
                {
                    ErrorCode = 0
                };
            }
            catch (LogicException e)
            {
                return new Response.ChangePw
                {
                    ErrorCode = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.ChangePw
                {
                    ErrorCode = uint.MaxValue
                };
            }
        }
    }
}

