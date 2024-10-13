using fb.protocol._internal;
using http.Service;
using Internal.Model.Redis;
using Internal.Redis;
using Internal.Redis.Key;
using Internal.Service;
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
        private readonly ILogger<TransferController> _logger;
        private readonly RedisService _redisService;
        private readonly Fb.Model.Model _dataSet;
        private readonly RabbitMqService _rabbitMqService;
        private readonly SessionService _sessionService;

        public InGameController(ILogger<TransferController> logger,
            RedisService redisService,
            Fb.Model.Model dataSet,
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
        public async Task<Response.Login> Login(Request.Login request)
        {
            var map = _dataSet.Map[request.Map] ?? throw new KeyNotFoundException($"{request.Map} not found in map data");
            var connection = _redisService.Connection;
            var config = await connection.JsonGetAsync<HostConfig>(new HeartBeatKey { Service = fb.protocol._internal.Service.Game, Id = map.Host }.Key);
            if (config == null)
            {
                return new Response.Login 
                {
                    Success = false, 
                    Logon = false ,
                    Ip = string.Empty,
                    Port = 0
                };
            }

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
                _rabbitMqService.Publish(new Response.KickOut
                {
                    Uid = session.Uid
                }, "amq.direct", $"fb.game.{session.Host}");
                return new Response.Login
                {
                    Success = false,
                    Logon = true,
                    Ip = String.Empty,
                    Port = 0
                };
            }

            return new Response.Login
            {
                Success = success,
                Logon = false,
                Ip = config.IP,
                Port = config.Port
            };
        }

        [HttpPost("logout")]
        public async Task<Response.Logout> Logout(Request.Logout request)
        {
            var connection = _redisService.Connection;
            await connection.HashDeleteAsync(new SessionKey { }.Key, request.Name);

            return new Response.Logout
            {
                Success = true
            };
        }

        [HttpPost("transfer")]
        public async Task<Response.Transfer> Transfer(Request.Transfer request)
        {
            var connection = _redisService.Connection;
            var exists = await connection.StringGetAsync(new HeartBeatKey { Service = request.Service, Id = request.Id }.Key);
            if (exists.IsNull)
            {
                return new Response.Transfer
                {
                    Code = TransferResult.Failed,
                    Ip = "0.0.0.0",
                };
            }

            var config = JsonConvert.DeserializeObject<HostConfig>(exists.ToString());
            return new Response.Transfer
            {
                Code = TransferResult.Success,
                Ip = config.IP,
                Port = config.Port
            };
        }

        [HttpPost("ping")]
        public async Task<Response.Pong> Ping(Request.Ping request)
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
            return new Response.Pong
            { };
        }

        [HttpPost("whisper")]
        public async Task<Response.Whisper> Whisper(Request.Whisper request)
        {
            var you = await _sessionService.Get(request.To);
            if (you == null)
                return new Response.Whisper { Success = false };

            var response = new Response.Whisper
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
