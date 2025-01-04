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

        [HttpPost("ping")]
        public async Task<Response.Pong> Ping(Request.Ping request)
        {
            var conn = _redisService.Redis(-1).Connection;
            var config = new HostConfig
            {
                Name = request.Name,
                IP = request.Ip,
                Port = request.Port
            };
            await conn.StringSetAsync(new HeartBeatKey
            {
                Service = request.Service,
                Id = request.Id,
            }.Key, JsonConvert.SerializeObject(config), TimeSpan.FromSeconds(30));
            return new Response.Pong
            { };
        }

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
    }
}
