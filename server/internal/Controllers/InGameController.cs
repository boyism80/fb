using Fb.Model.EnumValue;
using Http;
using Http.Model.Redis;
using Http.Redis;
using Http.Redis.Key;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using StackExchange.Redis;
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

        /// <summary>
        /// Initializes a new instance of the <see cref="InGameController"/> class.
        /// </summary>
        /// <param name="logger">The logger for recording in-game operations.</param>
        /// <param name="redisService">The Redis service for session and cache management.</param>
        /// <param name="dataSet">The game data model containing configuration data.</param>
        /// <param name="rabbitMqService">The RabbitMQ service for inter-server messaging.</param>
        /// <param name="sessionService">The session service for managing user sessions.</param>
        /// <param name="dbContext">The database context for data operations.</param>
        public InGameController(ILogger<InGameController> logger,
            RedisService redisService,
            Fb.Model.Model dataSet,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            DbContext dbContext)
        {
            _logger = logger;
            _redisService = redisService;
            _dataSet = dataSet;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
            _dbContext = dbContext;
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
    }
}
