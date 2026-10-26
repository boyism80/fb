using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Model.Redis;
using Http.Service;
using Http.Util;
using Internal.Services;
using Microsoft.AspNetCore.Mvc;
using System.Text.RegularExpressions;
using Option = Http.Model.Option;
using Protocol = fb.protocol.@internal;
using Request = fb.protocol.@internal.request;
using Response = fb.protocol.@internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("in-game")]
    public class InGameController : ControllerBase
    {
        private readonly ILogger<InGameController> _logger;
        private readonly RabbitMqService _rabbitMqService;
        private readonly SessionService _sessionService;
        private readonly DbContext _dbContext;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly BanService _banService;
        private readonly ServerStateService _serverStateService;
        private readonly LogService _logService;
        private readonly MaintenanceService _maintenanceService;
        private readonly FriendService _friendService;

        public InGameController(ILogger<InGameController> logger,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            DbContext dbContext,
            RedisDistributedLockService distributedLock,
            BanService banService,
            ServerStateService serverStateService,
            LogService logService,
            MaintenanceService maintenanceService,
            FriendService friendService)
        {
            _logger = logger;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
            _dbContext = dbContext;
            _distributedLock = distributedLock;
            _banService = banService;
            _serverStateService = serverStateService;
            _logService = logService;
            _maintenanceService = maintenanceService;
            _friendService = friendService;
        }

        [HttpPost("login")]
        public async Task<Response.Login> Login(Request.Login request)
        {
            try
            {
                var world = request.World;

                var banCheck = await _banService.IsBanned(world, request.Name);
                if (banCheck != null && banCheck.IsBanned)
                {
                    return new Response.Login
                    {
                        Error = (uint)ErrorCode.Banned,
                        BanReason = banCheck.Reason,
                        BanExpireDate = banCheck.ExpireDate?.ToString("yyyy-MM-dd HH:mm:ss")
                    };
                }

                var conf = await _serverStateService.GetHostConfig(request.ProcessWorld, Protocol.Service.Game, request.Host);
                if (conf == null)
                {
                    throw new LogicException(ErrorCode.ServerNotReady);
                }

                var success = await _sessionService.Login(world, request.Name, new Session
                {
                    Uid = request.Uid,
                    Host = request.Host,
                    World = request.ProcessWorld
                }, request.Force);

                if (!success)
                {
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
            try
            {
                await _sessionService.Delete(request.World, request.Name, request.Host);
                return new Response.Logout { Success = true };
            }
            catch (Exception e)
            {
                _logger.LogError(e, "Logout failed for {Name}", request.Name);
                return new Response.Logout { Success = false, Reason = e.Message };
            }
        }

        [HttpPost("transfer")]
        public async Task<Response.Transfer> Transfer(Request.Transfer request)
        {
            try
            {
                var world = request.World;

                if (!string.IsNullOrEmpty(request.Name))
                {
                    var banCheck = await _banService.IsBanned(world, request.Name);
                    if (banCheck != null && banCheck.IsBanned)
                    {
                        return new Response.Transfer
                        {
                            Error = (uint)ErrorCode.Banned,
                            BanReason = banCheck.Reason,
                            BanExpireDate = banCheck.ExpireDate?.ToString("yyyy-MM-dd HH:mm:ss")
                        };
                    }

                    var maintenanceInfo = await _maintenanceService.FindActiveMaintenance(MaintenanceLabel.World(world));
                    if (maintenanceInfo != null)
                    {
                        var characterId = await _dbContext.Character.GetCharacterId(world, request.Name);
                        if (characterId.HasValue)
                        {
                            var character = await _dbContext.Character.Get(world, characterId.Value);
                            if (character != null && character.Role < Fb.Model.EnumValue.Role.Admin)
                            {
                                return new Response.Transfer
                                {
                                    Error = (uint)ErrorCode.Maintenance,
                                    MaintenanceMessage = maintenanceInfo.Message,
                                    MaintenanceEndTime = maintenanceInfo.EndTime.ToString("yyyy-MM-dd HH:mm:ss")
                                };
                            }
                        }
                    }
                }

                var config = await _serverStateService.GetHostConfig(world, request.Service, request.Id);
                if (config == null)
                {
                    throw new LogicException(ErrorCode.ServerNotReady);
                }

                if (request.ForceShutdown && string.IsNullOrEmpty(request.Name) == false)
                {
                    // A live owner deletes the session itself after its disconnect save; deleting it here would let
                    // the next login read the character before that save lands.
                    var session = await _sessionService.Get(world, request.Name);
                    if (session != null)
                    {
                        await _rabbitMqService.PublishAsync(new Response.KickOut
                        {
                            Uid = session.Uid,
                            Name = request.Name,
                            World = world
                        }, AmqpRoute.Exchange, AmqpRoute.Unicast(session));

                        var owner = await _serverStateService.GetHostConfig(session.World, Protocol.Service.Game, (byte)session.Host);
                        if (owner != null)
                        {
                            throw new LogicException(ErrorCode.AlreadyLogin);
                        }
                        else
                        {
                            await _sessionService.Delete(world, request.Name, (byte)session.Host);
                        }
                    }
                }

                if (!string.IsNullOrEmpty(request.Name))
                {
                    var uid = await _dbContext.Character.GetCharacterId(world, request.Name);
                    if (uid.HasValue)
                    {
                        await _logService.WriteAsync("game_server_entry", new
                        {
                            account_name = request.Name,
                            uid = uid.Value,
                            game_server_ip = config.IP,
                            game_server_port = config.Port
                        });
                    }
                }

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

        [HttpPost("match-transfer")]
        public async Task<Response.MatchTransfer> MatchTransfer(Request.MatchTransfer request)
        {
            try
            {
                // The matchmaking server pinned the cross host into the match id when the match formed.
                var hostId = SnowflakeId.HostId(request.MatchId);
                var host = await _serverStateService.GetHostConfig(null, Protocol.Service.Game, hostId) ??
                    throw new LogicException(ErrorCode.ServerNotReady);

                return new Response.MatchTransfer
                {
                    Ip = host.IP,
                    Port = host.Port,
                    Id = hostId
                };
            }
            catch (LogicException e)
            {
                return new Response.MatchTransfer
                {
                    Error = (uint)e.Error
                };
            }
            catch (Exception e)
            {
                _logger.LogError(e.Message);
                return new Response.MatchTransfer
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost("whisper")]
        public async Task<Response.Whisper> Whisper(Request.Whisper request)
        {
            try
            {
                var world = request.World;

                var session = await _sessionService.Get(world, request.From) ??
                    throw new LogicException(ErrorCode.Offline);

                var targetRef = await _dbContext.Character.GetCharacterRef(request.To) ??
                    throw new LogicException(ErrorCode.Offline);
                var targetWorld = targetRef.World;
                var targetSession = await _sessionService.Get(targetWorld, request.To);
                if (targetSession == null)
                {
                    throw new LogicException(ErrorCode.Offline);
                }

                if (targetWorld != world && SameCrossHost(session, targetSession) == false)
                {
                    throw new LogicException(ErrorCode.Offline);
                }

                var target = await _dbContext.Character.Get(targetWorld, targetSession.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var sender = await _dbContext.Character.Get(world, session.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var targetOption = await _dbContext.Option.Get(targetWorld, targetSession.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundOption);

                if (targetOption.SuperHide && target.Role > sender.Role)
                {
                    throw new LogicException(ErrorCode.Offline);
                }

                if (!targetOption.Whisper)
                {
                    throw new LogicException(ErrorCode.DisabledWhisperTarget);
                }

                var response = new Response.Whisper
                {
                    Host = session.Host,
                    From = request.From,
                    To = target.Name,
                    Message = request.Message
                };
                await _rabbitMqService.PublishAsync(response, AmqpRoute.Exchange, AmqpRoute.Unicast(targetSession));
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

        [HttpPost("broadcast")]
        public async Task<Response.Broadcast> Broadcast(Request.Broadcast request)
        {
            var response = new Response.Broadcast
            {
                Message = request.Message,
                Type = request.Type,
                Host = request.Host,
                Error = (uint)ErrorCode.None
            };

            await _rabbitMqService.PublishAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("global", request.World));
            return response;
        }

        [HttpPost("set-exp-multiplier")]
        public async Task<Response.SetExpMultiplier> SetExpMultiplier(Request.SetExpMultiplier request)
        {
            var response = new Response.SetExpMultiplier
            {
                Value = request.Value,
                Error = (uint)ErrorCode.None
            };

            await _rabbitMqService.PublishAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("global", request.World));
            return response;
        }

        [HttpPost("reload-tables")]
        public async Task<Response.ReloadTables> ReloadTables(Request.ReloadTables request)
        {
            // Every host writes json/{stem}.json from these names; reject anything that could leave json/.
            var invalidName = (request.TableNames ?? new List<string>())
                .Select(x => x.Trim())
                .Select(x => x.EndsWith(".json", StringComparison.OrdinalIgnoreCase) ? x[..^5] : x)
                .FirstOrDefault(x => !Regex.IsMatch(x, @"^[a-z0-9_]+\z", RegexOptions.CultureInvariant));
            if (invalidName != null)
            {
                _logger.LogWarning("ReloadTables rejected invalid table name {Name}", invalidName);
                return new Response.ReloadTables
                {
                    Error = (uint)ErrorCode.Unhandled,
                    Url = string.Empty,
                    TableNames = new List<string>()
                };
            }

            var response = new Response.ReloadTables
            {
                Error = (uint)ErrorCode.None,
                Url = request.Url ?? string.Empty,
                TableNames = request.TableNames ?? new List<string>()
            };

            // Game/login logic hosts
            await _rabbitMqService.PublishAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("global", request.World));
            // C# hosts (AmqpListener on fb.global)
            await _rabbitMqService.PublishAsync(response, "amq.direct", "fb.global");
            return response;
        }

        [HttpPost("reload-scripts")]
        public async Task<Response.ReloadScripts> ReloadScripts(Request.ReloadScripts request)
        {
            var response = new Response.ReloadScripts
            {
                Error = (uint)ErrorCode.None,
                Url = request.Url ?? string.Empty,
                ScriptPaths = request.ScriptPaths ?? new List<string>()
            };

            await _rabbitMqService.PublishAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("global", request.World));
            return response;
        }

        [HttpPost("set-drop-rate-multiplier")]
        public async Task<Response.SetDropRateMultiplier> SetDropRateMultiplier(Request.SetDropRateMultiplier request)
        {
            var response = new Response.SetDropRateMultiplier
            {
                Value = request.Value,
                Error = (uint)ErrorCode.None
            };

            await _rabbitMqService.PublishAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("global", request.World));
            return response;
        }

        [HttpPost("set-datetime")]
        public async Task<Response.SetDateTime> SetDateTime(Request.SetDateTime request)
        {
            var response = new Response.SetDateTime
            {
                Datetime = request.Datetime,
                Reset = request.Reset,
                Error = (uint)ErrorCode.None
            };

            await _rabbitMqService.PublishAsync(response, AmqpRoute.Exchange, AmqpRoute.Home("global", request.World));
            return response;
        }

        [HttpPost("update-friends")]
        public async Task<Response.UpdateFriends> UpdateFriends(Request.UpdateFriends request)
        {
            return await _friendService.Update(request);
        }

        [HttpPost("friend-broadcast")]
        public async Task<Response.FriendBroadcast> FriendBroadcast(Request.FriendBroadcast request)
        {
            return await _friendService.Broadcast(request);
        }

        [HttpPost("option")]
        public async Task<Response.SetOption> Option(Request.SetOption request)
        {
            try
            {
                var world = request.World;
                if (request.Changes == null || request.Changes.Count == 0)
                {
                    throw new Exception("option changes is empty");
                }

                // The row is read and written whole, so concurrent changes must not interleave.
                await using var optionLock = await _distributedLock.Lock(world, Http.Model.Option.DistributedLockKey(request.User));
                var option = await _dbContext.Option.Get(world, request.User) ??
                    throw new Exception($"option {request.User} not found");

                foreach (var change in request.Changes)
                {
                    SetOption(option, change.Type, change.Enabled);
                }

                _dbContext.Option.Set(world, option);

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

        private static void SetOption(Option option, byte type, bool enabled)
        {
            switch ((Fb.Model.EnumValue.Option)type)
            {
                case Fb.Model.EnumValue.Option.Whisper:
                    option.Whisper = enabled;
                    break;

                case Fb.Model.EnumValue.Option.Group:
                    option.Group = enabled;
                    break;

                case Fb.Model.EnumValue.Option.Roar:
                    option.Roar = enabled;
                    break;

                case Fb.Model.EnumValue.Option.News:
                    option.News = enabled;
                    break;

                case Fb.Model.EnumValue.Option.MagicEffect:
                    option.MagicEffect = enabled;
                    break;

                case Fb.Model.EnumValue.Option.WeatherEffect:
                    option.WeatherEffect = enabled;
                    break;

                case Fb.Model.EnumValue.Option.FixedMove:
                    option.FixedMove = enabled;
                    break;

                case Fb.Model.EnumValue.Option.Trade:
                    option.Trade = enabled;
                    break;

                case Fb.Model.EnumValue.Option.FastMove:
                    option.FastMove = enabled;
                    break;

                case Fb.Model.EnumValue.Option.EffectSound:
                    option.EffectSound = enabled;
                    break;

                case Fb.Model.EnumValue.Option.PkProtect:
                    option.PkProtect = enabled;
                    break;

                case Fb.Model.EnumValue.Option.VisibleHelmet:
                    option.VisibleHelmet = enabled;
                    break;

                case Fb.Model.EnumValue.Option.SuperHide:
                    option.SuperHide = enabled;
                    break;

                default:
                    throw new Exception($"invalid option type : {type}");
            }
        }

        private static bool SameCrossHost(Session session, Session target)
        {
            if (session.World.HasValue || target.World.HasValue)
            {
                return false;
            }

            if (session.Host != target.Host)
            {
                return false;
            }

            return true;
        }
    }
}
