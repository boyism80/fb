using AutoMapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Model.Redis;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Option = Http.Model.Option;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

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
        private readonly IMapper _mapper;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly StorageService _storageService;
        private readonly BanService _banService;
        private readonly ServerStateService _serverStateService;
        private readonly LogService _logService;
        public InGameController(ILogger<InGameController> logger,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            DbContext dbContext,
            IMapper mapper,
            RedisDistributedLockService distributedLock,
            StorageService storageService,
            BanService banService,
            ServerStateService serverStateService,
            LogService logService)
        {
            _logger = logger;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
            _dbContext = dbContext;
            _mapper = mapper;
            _distributedLock = distributedLock;
            _storageService = storageService;
            _banService = banService;
            _serverStateService = serverStateService;
            _logService = logService;
        }
        [HttpPost("login")]
        public async Task<Response.Login> Login(Request.Login request)
        {
            try
            {
                var section = request.Section;

                // Check if user is banned
                var banCheck = await _banService.IsBanned(section, request.Name);
                if (banCheck != null && banCheck.IsBanned)
                {
                    return new Response.Login
                    {
                        Error = (uint)ErrorCode.Banned,
                        BanReason = banCheck.Reason,
                        BanExpireDate = banCheck.ExpireDate?.ToString("yyyy-MM-dd HH:mm:ss")
                    };
                }

                var conf = await _serverStateService.GetHostConfig(section, fb.protocol._internal.Service.Game, request.Host);
                if (conf == null)
                    throw new LogicException(ErrorCode.ServerNotReady);

                var success = await _sessionService.Login(section, request.Name, new Session
                {
                    Uid = request.Uid,
                    Host = request.Host
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
            var section = request.Section;
            await _sessionService.Delete(section, request.Name);

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
                var section = request.Section;

                // Check if user is banned (only if name is provided)
                if (!string.IsNullOrEmpty(request.Name))
                {
                    var banCheck = await _banService.IsBanned(section, request.Name);
                    if (banCheck != null && banCheck.IsBanned)
                    {
                        return new Response.Transfer
                        {
                            Error = (uint)ErrorCode.Banned,
                            BanReason = banCheck.Reason,
                            BanExpireDate = banCheck.ExpireDate?.ToString("yyyy-MM-dd HH:mm:ss")
                        };
                    }
                }

                var config = await _serverStateService.GetHostConfig(section, request.Service, request.Id);
                if (config == null)
                    throw new LogicException(ErrorCode.ServerNotReady);

                if (request.ForceShutdown && string.IsNullOrEmpty(request.Name) == false)
                {
                    var session = await _sessionService.GetAndDelete(section, request.Name);
                    if (session != null)
                    {
                        _rabbitMqService.Publish(section, new Response.KickOut
                        {
                            Uid = session.Uid,
                            Name = request.Name
                        }, "amq.direct", $"fb.game.{session.Host}");
                        throw new LogicException(ErrorCode.AlreadyLogin);
                    }
                }

                // Log game server entry event (only if name is provided)
                if (!string.IsNullOrEmpty(request.Name))
                {
                    var uid = await _dbContext.Character.GetCharacterId(section, request.Name);
                    if (uid.HasValue)
                    {
                        _logService.Write("game_server_entry", new
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
        [HttpPost("whisper")]
        public async Task<Response.Whisper> Whisper(Request.Whisper request)
        {
            try
            {
                var section = request.Section;

                var session = await _sessionService.Get(section, request.From) ??
                    throw new LogicException(ErrorCode.Offline);

                var targetSession = await _sessionService.Get(section, request.To);
                if (targetSession == null)
                    throw new LogicException(ErrorCode.Offline);

                var target = await _dbContext.Character.Get(section, targetSession.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var targetOption = await _dbContext.Option.Get(section, targetSession.Uid) ??
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
                _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.game.{targetSession.Host}");
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
        public Task<Response.Broadcast> Broadcast(Request.Broadcast request)
        {
            var response = new Response.Broadcast
            {
                Message = request.Message,
                Type = request.Type,
                Host = request.Host,
                Error = (uint)ErrorCode.None
            };

            _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.global");
            return Task.FromResult(response);
        }

        [HttpPost("set-exp-multiplier")]
        public Task<Response.SetExpMultiplier> SetExpMultiplier(Request.SetExpMultiplier request)
        {
            var response = new Response.SetExpMultiplier
            {
                Value = request.Value,
                Error = (uint)ErrorCode.None
            };

            _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.global");
            return Task.FromResult(response);
        }

        [HttpPost("set-drop-rate-multiplier")]
        public Task<Response.SetDropRateMultiplier> SetDropRateMultiplier(Request.SetDropRateMultiplier request)
        {
            var response = new Response.SetDropRateMultiplier
            {
                Value = request.Value,
                Error = (uint)ErrorCode.None
            };

            _rabbitMqService.Publish(request.Section, response, "amq.direct", $"fb.global");
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
        [HttpGet("init/{section}/{uid}")]
        public async Task<Response.Init> Init(string section, uint uid)
        {
            var ch = await _dbContext.Character.Get(section, uid);
            var items = await _dbContext.Item.Get(section, uid);
            var spells = await _dbContext.Spell.Get(section, uid);
            var achievements = await _dbContext.Achievement.Get(section, uid);
            var quests = await _dbContext.Quest.Get(section, uid);
            var storageBoxes = await _dbContext.StorageBox.Get(section, uid);
            var storageRewardMarks = await _dbContext.StorageRewardMark.Get(section, uid);
            var option = await _dbContext.Option.Get(section, uid) ??
                _dbContext.Option.Set(section, new Option
                {
                    Uid = uid,
                });
            var storagePending = await _storageService.GetPendingForUserAsync(section, uid);

            await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(uid)))
            {
                var sync = await _dbContext.CharacterSync.Get(section, uid) ??
                    _dbContext.CharacterSync.Set(section, new CharacterSync
                    {
                        Uid = uid
                    });

                await _dbContext.SaveChangesAsync();
                var now = DateTime.Now;
                var receivedSystemMails = await _dbContext.SystemMailUser.Get(section, uid);
                return new Response.Init
                {
                    Character = _mapper.Map<Protocol.Character>(ch),
                    Items = items.Select(_mapper.Map<Protocol.Item>).ToList(),
                    Spells = spells.Select(_mapper.Map<Protocol.Spell>).ToList(),
                    Achievements = achievements.Select(_mapper.Map<Protocol.Achievement>).ToList(),
                    Quests = quests.Select(_mapper.Map<Protocol.Quest>).ToList(),
                    ReceivedSystemMails = receivedSystemMails.Where(smu => !smu.Deleted).Select(_mapper.Map<Protocol.SystemMailUser>).ToList(),
                    StorageBoxes = storageBoxes
                        .Where(box => box.ExpiredDate == null || box.ExpiredDate > now)
                        .Select(_mapper.Map<Protocol.StorageBox>)
                        .ToList(),
                    StorageRewardMarks = storageRewardMarks
                        .Where(mark => !mark.Deleted)
                        .Select(_mapper.Map<Protocol.StorageRewardMark>)
                        .ToList(),
                    StoragePending = storagePending
                        .Select(_mapper.Map<Protocol.StoragePendingBox>)
                        .ToList(),
                    Option = _mapper.Map<Protocol.Option>(option),
                    Clan = sync.Clan,
                    Group = sync.Group,
                    Mail = await _dbContext.Mail.Unread(section, uid)
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
                var section = request.Section;

                var exists = await _dbContext.Character.Get(section, request.Character.Id) ??
                    throw new Exception();

                if (exists.Deleted)
                    throw new Exception();

                var ch = _mapper.Map<Character>(request.Character);
                _dbContext.Character.Set(section, ch);

                var items = Override(_mapper.Map<Protocol.Item[], Item[]>(request.Items.ToArray()), await _dbContext.Item.Get(section, request.Character.Id));
                _dbContext.Item.Set(section, items);

                var spells = Override(_mapper.Map<Protocol.Spell[], Spell[]>(request.Spells.ToArray()), await _dbContext.Spell.Get(section, request.Character.Id));
                _dbContext.Spell.Set(section, spells.ToArray());

                var achievements = Override(_mapper.Map<Protocol.Achievement[], Achievement[]>(request.Achievements.ToArray()), await _dbContext.Achievement.Get(section, request.Character.Id));
                _dbContext.Achievement.Set(section, achievements.ToArray());

                var quests = Override(_mapper.Map<Protocol.Quest[], Quest[]>(request.Quests.ToArray()), await _dbContext.Quest.Get(section, request.Character.Id));
                _dbContext.Quest.Set(section, quests.ToArray());

                var receivedSystemMails = Override(_mapper.Map<Protocol.SystemMailUser[], SystemMailUser[]>(request.ReceivedSystemMails.ToArray()), await _dbContext.SystemMailUser.Get(section, request.Character.Id));
                _dbContext.SystemMailUser.Set(section, receivedSystemMails.ToArray());

                var storageBoxes = Override(_mapper.Map<Protocol.StorageBox[], StorageBox[]>(request.StorageBoxes?.ToArray() ?? Array.Empty<Protocol.StorageBox>()), await _dbContext.StorageBox.Get(section, request.Character.Id));
                _dbContext.StorageBox.Set(section, storageBoxes);

                var storageRewardMarks = Override(_mapper.Map<Protocol.StorageRewardMark[], StorageRewardMark[]>(request.StorageRewardMarks?.ToArray() ?? Array.Empty<Protocol.StorageRewardMark>()), await _dbContext.StorageRewardMark.Get(section, request.Character.Id));
                _dbContext.StorageRewardMark.Set(section, storageRewardMarks.ToArray());

                var personalPendingIds = request.StorageRewardMarks?.Select(mark => mark.PendingId).ToHashSet() ?? new HashSet<string>();
                if (personalPendingIds.Count > 0)
                {
                    var pendingBoxes = (await _dbContext.StoragePendingBox.Get(section, request.Character.Id)).Where(x => personalPendingIds.Contains(x.Id)).ToArray();
                    if (pendingBoxes.Length > 0)
                    {
                        foreach (var pendingBox in pendingBoxes)
                        {
                            pendingBox.Deleted = true;
                        }
                        _dbContext.StoragePendingBox.Set(section, pendingBoxes);
                    }
                }

                await _dbContext.SaveChangesAsync();

                // Log character save event
                _logService.Write("character_save", new
                {
                    character_id = request.Character.Id,
                    character_name = request.Character.Name,
                    item_count = request.Items?.Count ?? 0,
                    spell_count = request.Spells?.Count ?? 0,
                    achievement_count = request.Achievements?.Count ?? 0,
                    quest_count = request.Quests?.Count ?? 0,
                    storage_box_count = request.StorageBoxes?.Count ?? 0
                });

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
                var section = request.Section;

                var option = await _dbContext.Option.Get(section, request.User) ??
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
                _dbContext.Option.Set(section, option);

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
