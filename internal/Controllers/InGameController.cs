using http.fb.protocol;
using http.Service;
using Internal.Model.Redis;
using Internal.Redis;
using Internal.Redis.Key;
using Internal.Service;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using StackExchange.Redis;
using System.Runtime.CompilerServices;
using request = fb.protocol.inter.request;
using response = fb.protocol.inter.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("in-game")]
    public class InGameController : ControllerBase
    {
        private readonly ILogger<TransferController> _logger;
        private readonly RedisService _redisService;
        private readonly http.Model.Model _dataSet;
        private readonly RabbitMqService _rabbitMqService;
        private readonly SessionService _sessionService;

        public InGameController(ILogger<TransferController> logger,
            RedisService redisService,
            http.Model.Model dataSet,
            RabbitMqService rabbitMqService,
            SessionService sessionService)
        {
            _logger = logger;
            _redisService = redisService;
            _dataSet = dataSet;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
        }

        [HttpPost("login")]
        public async Task<http.fb.protocol.@internal.Login> Login(http.fb.protocol.@internal.Login request)
        {
            var map = _dataSet.Map[request.Map] ?? throw new KeyNotFoundException($"{request.Map} not found in map data");
            var connection = _redisService.Connection;
            var config = await connection.JsonGetAsync<HostConfig>(new HeartBeatKey { Service = http.fb.protocol.Service.Game, Id = map.Host }.Key);
            if (config == null)
                return new response.Login { Success = false, Logon = false };

            var redisResult = await connection.ScriptEvaluateAsync("login.lua", new
            {
                key = new RedisKey(new SessionKey { }.Key),
                name = request.Name,
                session = JsonConvert.SerializeObject(new Session
                {
                    Uid = request.Uid,
                    Host = map.Host
                }),
            });

            var success = (bool)redisResult[0];
            if (success == false)
            {
                var session = JsonConvert.DeserializeObject<Session>(redisResult[1].ToString());
                _rabbitMqService.Publish(new response.KickOut
                {
                    Uid = session.Uid
                }, "amq.direct", $"fb.game.{session.Host}");
                return new response.Login { Success = false, Logon = true };
            }

            return new response.Login
            {
                Success = success,
                Logon = false,
                Ip = config.IP,
                Port = config.Port
            };
        }

        [HttpPost("logout")]
        public async Task<http.fb.protocol.@internal.Logout> Logout(http.fb.protocol.@internal.Logout request)
        {
            var connection = _redisService.Connection;
            await connection.HashDeleteAsync(new SessionKey { }.Key, $"{request.Uid}");

            return new response.Logout
            {
                Uid = request.Uid
            };
        }

        [HttpPost("transfer")]
        public async Task<http.fb.protocol.@internal.Transfer> Transfer(http.fb.protocol.@internal.Transfer request)
        {
            var connection = _redisService.Connection;
            var exists = await connection.StringGetAsync(new HeartBeatKey { Service = request.Service, Id = request.Id }.Key);
            if (exists.IsNull)
            {
                return new response.Transfer
                {
                    Code = TransferResult.Failed,
                    Ip = "0.0.0.0",
                };
            }

            var config = JsonConvert.DeserializeObject<HostConfig>(exists.ToString());
            return new response.Transfer
            {
                Code = TransferResult.Success,
                Ip = config.IP,
                Port = config.Port
            };
        }

        [HttpPost("ping")]
        public async Task<http.fb.protocol.@internal.Pong> Ping(http.fb.protocol.@internal.Ping request)
        {
            var connection = _redisService.Connection;
            var config = new HostConfig
            {
                Name = request.Name,
                IP = request.Ip,
                Port = request.Port
            };
            await connection.StringSetAsync(new HeartBeatKey
            {
                Service = request.Service,
                Id = request.Id,
            }.Key, JsonConvert.SerializeObject(config), TimeSpan.FromSeconds(5));
            return new response.Pong
            { };
        }

        [HttpPost("whisper")]
        public async Task<http.fb.protocol.@internal.Whisper> Whisper(http.fb.protocol.@internal.Whisper request)
        {
            var you = await _sessionService.Get(request.To);
            if (you == null)
                return new response.Whisper { Success = false };

            var response = new response.Whisper
            {
                Success = true,
                From = request.From,
                To = you.Uid,
                Message = request.Message
            };
            _rabbitMqService.Publish(response, "amq.direct", $"fb.game.{you.Host}");
            return response;
        }
    }
}
