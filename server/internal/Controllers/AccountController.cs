using AutoMapper;
using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using System.Data;
using Request = fb.protocol.@internal.request;
using Response = fb.protocol.@internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("account")]
    public class AccountController : ControllerBase
    {
        private const int NameReservationThresholdSeconds = 120;

        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly ILogger<AccountController> _logger;
        private readonly LogService _logService;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly PasswordAttemptService _passwordAttempts;

        public AccountController(IMapper mapper,
            DbContext dbContext,
            ILogger<AccountController> logger,
            LogService logService,
            RedisDistributedLockService distributedLock,
            PasswordAttemptService passwordAttempts)
        {
            _mapper = mapper;
            _dbContext = dbContext;
            _logger = logger;
            _logService = logService;
            _distributedLock = distributedLock;
            _passwordAttempts = passwordAttempts;
        }

        [HttpGet("{world}/uid/{name}")]
        public async Task<Response.GetUid> Uid(uint world, string name)
        {
            try
            {
                var row = await _dbContext.Character.GetCharacterRef(name) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                if (row.World != world)
                {
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                }

                return new Response.GetUid
                {
                    Uid = row.Id,
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
        [HttpPost("authenticate")]
        public async Task<Response.Authenticate> Authenticate(Request.Authenticate request)
        {
            var world = request.World;
            var ch = await _dbContext.Character.Get(world, request.Uid);
            if (ch == null)
            {
                return new Response.Authenticate
                {
                    ErrorCode = 1
                };
            }

            var attempt = await _passwordAttempts.Begin(world, request.Uid, request.Ip);
            if (attempt == PasswordAttempt.Locked)
            {
                return new Response.Authenticate
                {
                    ErrorCode = (uint)ErrorCode.AccountLocked
                };
            }

            if (PasswordHash.Matches(ch.Pw, request.Pw, out var updatedPw) == false)
            {
                return new Response.Authenticate
                {
                    ErrorCode = 2
                };
            }

            await _passwordAttempts.Succeed(world, request.Uid, request.Ip, attempt);
            if (updatedPw != null)
            {
                await using var _ = await _distributedLock.Lock(world, Character.SaveLockKey(request.Uid));
                var current = await _dbContext.Character.Get(world, request.Uid);
                if (current != null && current.Pw == ch.Pw)
                {
                    current.Pw = updatedPw;
                    _dbContext.Character.Set(world, current);
                    await _dbContext.SaveChangesAsync();
                }
            }
            return new Response.Authenticate
            {
                ErrorCode = 0,
                Map = ch.Map
            };
        }
        [HttpPost("reserve")]
        public async Task<Response.ReserveName> ReserveName(Request.ReserveName request)
        {
            try
            {
                await using var conn = _dbContext.GetUnifiedConnection();
                await conn.OpenAsync();
                await using var transaction = await conn.BeginTransactionAsync();
                var entry = await conn.QuerySingleOrDefaultAsync<NameRegistryEntry>(
                    """
                    SELECT `id` AS Id, `world` AS World,
                           CAST(`reserved_at` IS NULL AS UNSIGNED) AS Confirmed,
                           CAST(IFNULL(`reserved_at` <= UTC_TIMESTAMP(3) - INTERVAL @threshold SECOND, 0) AS UNSIGNED) AS Expired
                    FROM `name_registry` WHERE `name` = @name
                    FOR UPDATE
                    """,
                    new { name = request.Name, threshold = NameReservationThresholdSeconds },
                    transaction);

                if (entry != null)
                {
                    if (entry.Confirmed == 1 || entry.Expired == 0)
                    {
                        await transaction.RollbackAsync();
                        return new Response.ReserveName { Success = false, Uid = 0 };
                    }

                    _dbContext.Character.Delete(entry.World, new CharacterKey { Id = entry.Id });
                    await _dbContext.SaveChangesAsync();
                    await conn.ExecuteAsync("DELETE FROM `name_registry` WHERE `id` = @id", new { id = entry.Id }, transaction);
                }

                var uid = await conn.ExecuteScalarAsync<ulong>(
                    """
                    INSERT INTO `name_registry` (`name`, `world`, `reserved_at`) VALUES (@name, @world, UTC_TIMESTAMP(3));
                    SELECT LAST_INSERT_ID();
                    """,
                    new { name = request.Name, world = request.World },
                    transaction);
                await transaction.CommitAsync();
                return new Response.ReserveName { Success = true, Uid = Convert.ToUInt32(uid) };
            }
            catch (Exception)
            {
                return new Response.ReserveName { Success = false, Uid = 0 };
            }
        }

        private sealed class NameRegistryEntry
        {
            public uint Id { get; set; }
            public uint World { get; set; }
            public ulong Confirmed { get; set; }
            public ulong Expired { get; set; }
        }

        [HttpPost("name-keepalive")]
        public async Task<Response.ReserveName> NameKeepAlive(Request.ReserveName request)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var refreshed = await conn.QueryFirstOrDefaultAsync<long>("USP_NAME_KEEPALIVE",
                new { in_id = request.Uid },
                commandType: CommandType.StoredProcedure);
            return new Response.ReserveName { Success = refreshed == 1, Uid = request.Uid };
        }
        [HttpPost("init")]
        public async Task<Response.InitCharacter> InitCharacter(Request.InitCharacter request)
        {
            var world = request.World;
            await using var conn = _dbContext.GetUnifiedConnection();
            await conn.OpenAsync();
            await using var transaction = await conn.BeginTransactionAsync();
            var held = await conn.ExecuteAsync(
                """
                UPDATE `name_registry` SET `reserved_at` = UTC_TIMESTAMP(3)
                WHERE `id` = @uid AND `world` = @world AND `name` = @name AND `reserved_at` IS NOT NULL
                """,
                new { uid = request.Uid, world, name = request.Name },
                transaction);
            if (held == 0)
            {
                await transaction.RollbackAsync();
                return new Response.InitCharacter
                {
                    Success = false
                };
            }

            var ch = new Character
            {
                Id = request.Uid,
                World = world,
                Name = request.Name,
                Pw = PasswordHash.Compute(request.Pw),
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
            _dbContext.Character.Set(world, ch);
            await _dbContext.SaveChangesAsync();
            await transaction.CommitAsync();

            await _logService.WriteAsync("account_create", new
            {
                account_name = request.Name,
                uid = request.Uid
            });

            return new Response.InitCharacter
            {
                Success = true
            };
        }
        [HttpPost("make")]
        public async Task<Response.MakeCharacter> MakeCharacter(Request.MakeCharacter request)
        {
            try
            {
                if (request.Nation != (byte)Nation.Goguryeo && request.Nation != (byte)Nation.Buyeo)
                {
                    return new Response.MakeCharacter
                    {
                        Success = false
                    };
                }

                if (request.Gender != (byte)Gender.Male && request.Gender != (byte)Gender.Female)
                {
                    return new Response.MakeCharacter
                    {
                        Success = false
                    };
                }

                if (request.DivineBeast != (byte)DivineBeast.VermilionBird && request.DivineBeast != (byte)DivineBeast.WhiteTiger &&
                    request.DivineBeast != (byte)DivineBeast.BlackTortoise && request.DivineBeast != (byte)DivineBeast.AzureDragon)
                {
                    return new Response.MakeCharacter
                    {
                        Success = false
                    };
                }

                var world = request.World;
                await using var conn = _dbContext.GetUnifiedConnection();
                await conn.OpenAsync();
                await using var transaction = await conn.BeginTransactionAsync();
                var pending = await conn.QuerySingleOrDefaultAsync<long?>(
                    """
                    SELECT `reserved_at` IS NOT NULL FROM `name_registry`
                    WHERE `id` = @uid AND `world` = @world
                    FOR UPDATE
                    """,
                    new { uid = request.Uid, world },
                    transaction);
                if (pending == null)
                {
                    await transaction.RollbackAsync();
                    _dbContext.Character.Delete(world, new CharacterKey { Id = request.Uid });
                    await _dbContext.SaveChangesAsync();
                    return new Response.MakeCharacter
                    {
                        Success = false
                    };
                }

                if (pending == 0)
                {
                    await transaction.RollbackAsync();
                    return new Response.MakeCharacter
                    {
                        Success = true
                    };
                }

                var ch = await _dbContext.Character.Get(world, request.Uid) ??
                    throw new Exception($"user {request.Uid} not found");

                ch.Hair = request.Hair;
                ch.Face = request.Face;
                ch.Gender = request.Gender;
                ch.Nation = request.Nation;
                ch.DivineBeast = request.DivineBeast;
                _dbContext.Character.Set(world, ch);

                await _dbContext.SaveChangesAsync();

                var confirmed = await conn.QueryFirstOrDefaultAsync<long>("USP_NAME_CONFIRM",
                    new { in_id = request.Uid },
                    transaction,
                    commandType: CommandType.StoredProcedure);
                if (confirmed != 1)
                {
                    await transaction.RollbackAsync();
                    return new Response.MakeCharacter
                    {
                        Success = false
                    };
                }

                await transaction.CommitAsync();
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
        [HttpPost("change-pw")]
        public async Task<Response.ChangePw> ChangePassword(Request.ChangePw request)
        {
            try
            {
                var world = request.World;
                await using var saveLock = await _distributedLock.Lock(world, Character.SaveLockKey(request.Uid));
                var ch = await _dbContext.Character.Get(world, request.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var attempt = await _passwordAttempts.Begin(world, request.Uid, request.Ip);
                if (attempt == PasswordAttempt.Locked)
                {
                    throw new LogicException(ErrorCode.AccountLocked);
                }

                if (PasswordHash.Matches(ch.Pw, request.Before, out _) == false)
                {
                    throw new LogicException(ErrorCode.PasswordNotMatched);
                }

                if (ch.Birth != request.Birthday)
                {
                    throw new LogicException(ErrorCode.BirthdayNotMatched);
                }

                await _passwordAttempts.Succeed(world, request.Uid, request.Ip, attempt);
                ch.Pw = PasswordHash.Compute(request.After);
                _dbContext.Character.Set(world, ch);

                await _dbContext.SaveChangesAsync();

                // Log password change event
                await _logService.WriteAsync("password_change", new
                {
                    account_name = ch.Name,
                    uid = request.Uid
                });

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


