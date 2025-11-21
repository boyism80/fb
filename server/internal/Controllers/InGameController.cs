using AutoMapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Model.Redis;
using Http.Redis;
using Http.Redis.Key;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using StackExchange.Redis;
using Option = Http.Model.Option;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    /// <summary>
    /// Provides in-game operations and session management for the internal API.
    /// Handles player login/logout, server transfers, whisper messaging, and broadcasting.
    /// </summary>
    [ApiController]
    [Route("in-game")]
    public class InGameController : ControllerBase
    {
        private readonly ILogger<InGameController> _logger;
        private readonly RedisService _redisService;
        private readonly Fb.Model.Model _dataSet;
        private readonly RabbitMqService _rabbitMqService;
        private readonly SessionService _sessionService;
        private readonly DbContext _dbContext;
        private readonly IMapper _mapper;
        private readonly RedisDistributedLockService _distributedLock;

        /// <summary>
        /// Initializes a new instance of the <see cref="InGameController"/> class.
        /// </summary>
        /// <param name="logger">The logger for recording in-game operations.</param>
        /// <param name="redisService">The Redis service for session and cache management.</param>
        /// <param name="dataSet">The game data model containing configuration data.</param>
        /// <param name="rabbitMqService">The RabbitMQ service for inter-server messaging.</param>
        /// <param name="sessionService">The session service for managing user sessions.</param>
        /// <param name="dbContext">The database context for data operations.</param>
        /// <param name="mapper">The AutoMapper instance for object mapping.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        public InGameController(ILogger<InGameController> logger,
            RedisService redisService,
            Fb.Model.Model dataSet,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            DbContext dbContext,
            IMapper mapper,
            RedisDistributedLockService distributedLock)
        {
            _logger = logger;
            _redisService = redisService;
            _dataSet = dataSet;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
            _dbContext = dbContext;
            _mapper = mapper;
            _distributedLock = distributedLock;
        }

        /// <summary>
        /// Handles player login requests and manages session creation.
        /// Validates server availability, manages session conflicts, and returns connection information.
        /// </summary>
        /// <param name="request">The login request containing player credentials and target host.</param>
        /// <returns>A login response with connection details or error information.</returns>
        [HttpPost("login")]
        public async Task<Response.Login> Login(Request.Login request)
        {
            try
            {
                var redis = _redisService.Redis(-1);
                var conf = await redis.Connection.JsonGetAsync<HostConfig>(new HeartBeatKey { Service = fb.protocol._internal.Service.Game, Id = request.Host }.Key);
                if (conf == null)
                    throw new LogicException(ErrorCode.ServerNotReady);

                var redisResult = await redis.ScriptEvaluateAsync("login.lua", new
                {
                    key = new RedisKey(new SessionKey { }.Key),
                    name = request.Name,
                    session = JsonConvert.SerializeObject(new Session
                    {
                        Uid = request.Uid,
                        Host = request.Host
                    }),
                });

                var success = (bool)redisResult[0];
                if (success == false)
                {
                    var session = JsonConvert.DeserializeObject<Session>(redisResult[1].ToString());
                    _rabbitMqService.Publish(new Response.KickOut
                    {
                        Uid = session.Uid,
                        Name = request.Name
                    }, "amq.direct", $"fb.game.{session.Host}");
                    throw new LogicException(ErrorCode.AlreadyLogin);
                }

                return new Response.Login
                {
                    Logon = false,
                    Ip = conf.IP,
                    Port = conf.Port
                };
            }
            catch (LogicException e)
            {
                return new Response.Login
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
            {
                _logger.LogError(e.Message);
                return new Response.Login
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Handles player logout requests and cleans up session data.
        /// Removes the player's session from Redis storage.
        /// </summary>
        /// <param name="request">The logout request containing the player's name.</param>
        /// <returns>A logout response indicating success.</returns>
        [HttpPost("logout")]
        public async Task<Response.Logout> Logout(Request.Logout request)
        {
            var conn = _redisService.Redis(-1).Connection;
            await conn.HashDeleteAsync(new SessionKey { }.Key, request.Name);

            return new Response.Logout
            {
                Success = true
            };
        }

        /// <summary>
        /// Handles server transfer requests for moving players between game servers.
        /// Validates target server availability and handles forced disconnections if needed.
        /// </summary>
        /// <param name="request">The transfer request containing target service and player information.</param>
        /// <returns>A transfer response with target server connection details or error information.</returns>
        [HttpPost("transfer")]
        public async Task<Response.Transfer> Transfer(Request.Transfer request)
        {
            try
            {
                var conn = _redisService.Redis(-1).Connection;
                var gameConf = await conn.StringGetAsync(new HeartBeatKey { Service = request.Service, Id = request.Id }.Key);
                if (gameConf.IsNull)
                    throw new LogicException(ErrorCode.ServerNotReady);

                if (request.ForceShutdown && string.IsNullOrEmpty(request.Name) == false)
                {
                    // TODO: 루아스크립트
                    var session = await conn.JsonHashGetAsync<Session>(new SessionKey().Key, new RedisValue(request.Name));
                    if (session != null)
                    {
                        await conn.HashDeleteAsync(new SessionKey().Key, new RedisValue(request.Name));
                        _rabbitMqService.Publish(new Response.KickOut
                        {
                            Uid = session.Uid,
                            Name = request.Name
                        }, "amq.direct", $"fb.game.{session.Host}");
                        throw new LogicException(ErrorCode.AlreadyLogin);
                    }
                }

                var config = JsonConvert.DeserializeObject<HostConfig>(gameConf.ToString());
                return new Response.Transfer
                {
                    Ip = config.IP,
                    Port = config.Port
                };
            }
            catch (LogicException e)
            {
                return new Response.Transfer
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
            {
                _logger.LogError(e.Message);
                return new Response.Transfer
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Handles whisper message requests between players.
        /// Validates both sender and recipient sessions, checks whisper permissions, and routes the message.
        /// </summary>
        /// <param name="request">The whisper request containing sender, recipient, and message information.</param>
        /// <returns>A whisper response with routing information or error details.</returns>
        [HttpPost("whisper")]
        public async Task<Response.Whisper> Whisper(Request.Whisper request)
        {
            try
            {
                var session = await _sessionService.Get(request.From) ??
                    throw new LogicException(ErrorCode.Offline);

                var targetSession = await _sessionService.Get(request.To);
                if (targetSession == null)
                    throw new LogicException(ErrorCode.Offline);

                var target = await _dbContext.Character.Get(targetSession.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var targetOption = await _dbContext.Option.Get(targetSession.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundOption);

                if (!targetOption.Whisper)
                    throw new LogicException(ErrorCode.DisabledWhisperTarget);

                var response = new Response.Whisper
                {
                    Host = session.Host,
                    From = request.From,
                    To = target.Name,
                    Message = request.Message
                };
                _rabbitMqService.Publish(response, "amq.direct", $"fb.game.{targetSession.Host}");
                return response;
            }
            catch (LogicException e)
            {
                return new Response.Whisper
                {
                    From = request.From,
                    To = request.To,
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
            {
                _logger.LogError(e.Message);
                return new Response.Whisper
                {
                    From = request.From,
                    To = request.To,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Handles global broadcast message requests.
        /// Publishes messages to all connected game servers via RabbitMQ.
        /// </summary>
        /// <param name="request">The broadcast request containing message content and type.</param>
        /// <returns>A broadcast response confirming the message was sent.</returns>
        [HttpPost("broadcast")]
        public Task<Response.Broadcast> Broadcast(Request.Broadcast request)
        {
            var response = new Response.Broadcast
            {
                Message = request.Message,
                Type = request.Type,
                Host = request.Host,
                Error = (uint)ErrorCode.None
            };

            _rabbitMqService.Publish(response, "amq.direct", $"fb.global");
            return Task.FromResult(response);
        }

        [HttpPost("update-friends")]
        public Task<Response.UpdateFriends> UpdateFriends(Request.UpdateFriends request)
        {
            var response = new Response.UpdateFriends
            {
                Error = (uint)ErrorCode.None
            };

            return Task.FromResult(response);
        }

        /// <summary>
        /// Loads game initialization data for a character entering the game.
        /// Retrieves character data, items, spells, achievements, quests, options, clan, group, and mail.
        /// </summary>
        /// <param name="uid">The unique identifier of the character.</param>
        /// <returns>An initialization response containing all character game data.</returns>
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

        /// <summary>
        /// Merges request data with existing data, marking deleted items.
        /// Used for synchronizing game data between client and server.
        /// </summary>
        /// <typeparam name="T">The model type that implements IModel and IRedisHashKey.</typeparam>
        /// <param name="request">The request data from the client.</param>
        /// <param name="exists">The existing data from the database.</param>
        /// <returns>An array of merged data with deleted items marked.</returns>
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

        /// <summary>
        /// Saves character game data including character stats, items, spells, achievements, and quests.
        /// Merges the request data with existing data and persists changes to the database.
        /// </summary>
        /// <param name="request">The save request containing character and game data.</param>
        /// <returns>A save response indicating success or failure.</returns>
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

        /// <summary>
        /// Updates a user's game option settings.
        /// Modifies specific option flags such as whisper, group, roar, and visual/audio effects.
        /// </summary>
        /// <param name="request">The option update request containing the option type and enabled state.</param>
        /// <returns>An option response indicating success or failure.</returns>
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
