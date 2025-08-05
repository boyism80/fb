using Http.Util;
using AutoMapper;
using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.ObjectPool;
using System.Buffers;
using System.Data;
using System.Security.Cryptography;
using System.Text;
using Option = Http.Model.Option;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    /// <summary>
    /// Provides user management operations for the internal API.
    /// Handles user authentication, character creation, data saving, and user preferences.
    /// </summary>
    [ApiController]
    [Route("user")]
    public class UserController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly RedisService _redisService;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly ILogger<UserController> _logger;

        // Object pool for SHA256 instances to reduce GC pressure
        // Limit pool size to 16 instances (reasonable for most scenarios)
        private static readonly ObjectPool<SHA256> _sha256Pool = new DefaultObjectPool<SHA256>(new Sha256PooledObjectPolicy(), 64);

        /// <summary>
        /// Initializes a new instance of the <see cref="UserController"/> class.
        /// </summary>
        /// <param name="configuration">The application configuration.</param>
        /// <param name="mapper">The AutoMapper instance for object mapping.</param>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="logger">The logger for recording user operations.</param>
        public UserController(IConfiguration configuration,
            IMapper mapper,
            DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            ILogger<UserController> logger)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
            _redisService = redisService;
            _distributedLock = distributedLock;
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
        [HttpPost("reserve-name")]
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
        [HttpPost("init-ch")]
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
        [HttpPost("mk-ch")]
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

        [HttpGet("init/{uid}")]
        public async Task<Response.Init> Init(uint uid)
        {
            var ch = await _dbContext.Character.Get(uid);
            var items = await _dbContext.Item.Get(uid);
            var spells = await _dbContext.Spell.Get(uid);
            var achievements = await _dbContext.Achievement.Get(uid);
            var quests = await _dbContext.Quest.Get(uid);
            var option = await _dbContext.Option.Get(uid) ??
                _dbContext.Option.Set(new Option
                {
                    Uid = uid,
                });

            await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(uid)))
            {
                var sync = await _dbContext.CharacterSync.Get(uid) ??
                    _dbContext.CharacterSync.Set(new CharacterSync
                    {
                        Uid = uid
                    });

                await _dbContext.SaveChangesAsync();
                return new Response.Init
                {
                    Character = _mapper.Map<Protocol.Character>(ch),
                    Items = items.Select(_mapper.Map<Protocol.Item>).ToList(),
                    Spells = spells.Select(_mapper.Map<Protocol.Spell>).ToList(),
                    Achievements = achievements.Select(_mapper.Map<Protocol.Achievement>).ToList(),
                    Quests = quests.Select(_mapper.Map<Protocol.Quest>).ToList(),
                    Option = _mapper.Map<Protocol.Option>(option),
                    Clan = sync.Clan,
                    Group = sync.Group,
                    Mail = await _dbContext.Mail.Unread(uid)
                };
            }
        }

        private T[] Override<T>(IEnumerable<T> request, IEnumerable<T> exists) where T : IModel, IRedisHashKey
        {
            var src = request.ToDictionary(x => $"{x.GetRedisKey()}:{x.GetRedisField()}");
            var dst = exists.ToDictionary(x => $"{x.GetRedisKey()}:{x.GetRedisField()}");

            var deletedKeys = dst.Keys.Except(src.Keys).ToArray();
            if (deletedKeys.Length > 0)
            {
                _logger.LogWarning($"deleted keys : {string.Join(", ", deletedKeys)}");
            }

            foreach (var x in dst.Values)
            {
                x.Deleted = true;
            }

            foreach (var key in src.Keys.ToArray())
            {
                dst[key] = src[key];
            }

            return dst.Values.ToArray();
        }

        [HttpPost("save")]
        public async Task<Response.Save> Save(Request.Save request)
        {
            try
            {
                var exists = await _dbContext.Character.Get(request.Character.Id) ??
                    throw new Exception();

                if (exists.Deleted)
                    throw new Exception();

                var ch = _mapper.Map<Character>(request.Character);
                _dbContext.Character.Set(ch);

                var items = Override(_mapper.Map<Protocol.Item[], Item[]>(request.Items.ToArray()), await _dbContext.Item.Get(request.Character.Id));
                _dbContext.Item.Set(items);

                var spells = Override(_mapper.Map<Protocol.Spell[], Spell[]>(request.Spells.ToArray()), await _dbContext.Spell.Get(request.Character.Id));
                _dbContext.Spell.Set(spells.ToArray());

                var achievements = Override(_mapper.Map<Protocol.Achievement[], Achievement[]>(request.Achievements.ToArray()), await _dbContext.Achievement.Get(request.Character.Id));
                _dbContext.Achievement.Set(achievements.ToArray());

                var quests = Override(_mapper.Map<Protocol.Quest[], Quest[]>(request.Quests.ToArray()), await _dbContext.Quest.Get(request.Character.Id));
                _dbContext.Quest.Set(quests.ToArray());

                await _dbContext.SaveChangesAsync();
                return new Response.Save
                {
                    Success = true
                };
            }
            catch (Exception)
            {
                return new Response.Save
                {
                    Success = false
                };
            }
        }

        [HttpPost("option")]
        public async Task<Response.SetOption> Option(Request.SetOption request)
        {
            try
            {
                var option = await _dbContext.Option.Get(request.User) ??
                    throw new Exception($"option {request.User} not found");

                switch ((Fb.Model.EnumValue.Option)request.Type)
                {
                    case Fb.Model.EnumValue.Option.Whisper:
                        option.Whisper = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.Group:
                        option.Group = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.Roar:
                        option.Roar = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.RoarWorlds:
                        option.RoarWorlds = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.MagicEffect:
                        option.MagicEffect = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.WeatherEffect:
                        option.WeatherEffect = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.FixedMove:
                        option.FixedMove = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.Trade:
                        option.Trade = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.FastMove:
                        option.FastMove = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.EffectSound:
                        option.EffectSound = request.Enabled;
                        break;

                    case Fb.Model.EnumValue.Option.PkProtect:
                        option.PkProtect = request.Enabled;
                        break;

                    default:
                        throw new Exception($"invalid option type : {request.Type}");
                }
                _dbContext.Option.Set(option);

                await _dbContext.SaveChangesAsync();
                return new Response.SetOption
                {
                    Success = true
                };
            }
            catch (Exception)
            {
                return new Response.SetOption
                {
                    Success = false
                };
            }
        }
    }
}