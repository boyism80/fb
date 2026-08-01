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
        private readonly BanService _banService;
        private readonly ServerStateService _serverStateService;
        private readonly LogService _logService;
        private readonly MaintenanceService _maintenanceService;
        public InGameController(ILogger<InGameController> logger,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            DbContext dbContext,
            IMapper mapper,
            RedisDistributedLockService distributedLock,
            BanService banService,
            ServerStateService serverStateService,
            LogService logService,
            MaintenanceService maintenanceService)
        {
            _logger = logger;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
            _dbContext = dbContext;
            _mapper = mapper;
            _distributedLock = distributedLock;
            _banService = banService;
            _serverStateService = serverStateService;
            _logService = logService;
            _maintenanceService = maintenanceService;
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

                var maintenanceInfo = await _maintenanceService.GetMaintenanceInfo(world);
                if (maintenanceInfo != null && maintenanceInfo.IsActive)
                {
                    var characterId = await _dbContext.Character.GetCharacterId(world, request.Name);
                    if (characterId.HasValue)
                    {
                        var character = await _dbContext.Character.Get(world, characterId.Value);
                        if (character != null && character.Role < Fb.Model.EnumValue.Role.Admin)
                        {
                            return new Response.Login
                            {
                                Error = (uint)ErrorCode.Maintenance,
                                MaintenanceMessage = maintenanceInfo.Message,
                                MaintenanceEndTime = maintenanceInfo.EndTime.ToString("yyyy-MM-dd HH:mm:ss")
                            };
                        }
                    }
                }

                var conf = await _serverStateService.GetHostConfig(world, fb.protocol._internal.Service.Game, request.Host);
                if (conf == null)
                    throw new LogicException(ErrorCode.ServerNotReady);

                var success = await _sessionService.Login(world, request.Name, new Session
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
            var world = request.World;
            await _sessionService.Delete(world, request.Name);

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

                    var maintenanceInfo = await _maintenanceService.GetMaintenanceInfo(world);
                    if (maintenanceInfo != null && maintenanceInfo.IsActive)
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
                    throw new LogicException(ErrorCode.ServerNotReady);

                if (request.ForceShutdown && string.IsNullOrEmpty(request.Name) == false)
                {
                    var session = await _sessionService.GetAndDelete(world, request.Name);
                    if (session != null)
                    {
                        await _rabbitMqService.PublishAsync(new Response.KickOut
                        {
                            Uid = session.Uid,
                            Name = request.Name
                        }, "amq.direct", $"fb.{world}.game.{session.Host}");
                        throw new LogicException(ErrorCode.AlreadyLogin);
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

        [HttpPost("whisper")]
        public async Task<Response.Whisper> Whisper(Request.Whisper request)
        {
            try
            {
                var world = request.World;

                var session = await _sessionService.Get(world, request.From) ??
                    throw new LogicException(ErrorCode.Offline);

                var targetSession = await _sessionService.Get(world, request.To);
                if (targetSession == null)
                    throw new LogicException(ErrorCode.Offline);

                var target = await _dbContext.Character.Get(world, targetSession.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var targetOption = await _dbContext.Option.Get(world, targetSession.Uid) ??
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
                await _rabbitMqService.PublishAsync(response, "amq.direct", $"fb.{request.World}.game.{targetSession.Host}");
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

            await _rabbitMqService.PublishAsync(response, "amq.direct", $"fb.{request.World}.global");
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

            await _rabbitMqService.PublishAsync(response, "amq.direct", $"fb.{request.World}.global");
            return response;
        }

        [HttpPost("reload-tables")]
        public async Task<Response.ReloadTables> ReloadTables(Request.ReloadTables request)
        {
            var response = new Response.ReloadTables
            {
                Error = (uint)ErrorCode.None,
                Url = request.Url ?? string.Empty,
                TableNames = request.TableNames ?? new List<string>()
            };

            // Game/login logic hosts
            await _rabbitMqService.PublishAsync(response, "amq.direct", $"fb.{request.World}.global");
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

            await _rabbitMqService.PublishAsync(response, "amq.direct", $"fb.{request.World}.global");
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

            await _rabbitMqService.PublishAsync(response, "amq.direct", $"fb.{request.World}.global");
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

            await _rabbitMqService.PublishAsync(response, "amq.direct", $"fb.{request.World}.global");
            return response;
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

        [HttpGet("init/{world}/{uid}")]
        public async Task<Response.Init> Init(uint world, uint uid)
        {
            var ch = await _dbContext.Character.Get(world, uid);
            var items = await _dbContext.Item.Get(world, uid);
            var spells = await _dbContext.Spell.Get(world, uid);
            var matchmakingSkills = await _dbContext.MatchmakingSkill.Get(world, uid);
            var achievements = await _dbContext.Achievement.Get(world, uid);
            var quests = await _dbContext.Quest.Get(world, uid);
            var storageBoxes = await _dbContext.StorageBox.Get(world, uid);
            var marketplacePendings = await _dbContext.MarketplacePending.Get(world, uid);
            var option = await _dbContext.Option.Get(world, uid) ??
                _dbContext.Option.Set(world, new Option
                {
                    Uid = uid,
                });
            var marriageData = await _dbContext.Marriage.Get(world, uid) ??
                _dbContext.Marriage.Set(world, new Http.Model.Marriage
                {
                    CharacterId = uid,
                });

            var spouseName = string.Empty;
            if (marriageData.SpouseId.HasValue)
                spouseName = await _dbContext.Character.GetName(world, marriageData.SpouseId.Value) ?? string.Empty;
            var marriageProtocol = _mapper.Map<Protocol.Marriage>(marriageData);
            marriageProtocol.SpouseName = spouseName ?? string.Empty;

            await using var _ = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(uid));

            var sync = await _dbContext.CharacterRealtimeState.Get(world, uid) ??
                _dbContext.CharacterRealtimeState.Set(world, new CharacterRealtimeState
                {
                    Uid = uid
                });

            await _dbContext.SaveChangesAsync();
            var now = DateTime.Now;
            return new Response.Init
            {
                Character = _mapper.Map<Protocol.Character>(ch),
                Marriage = marriageProtocol,
                Items = items.Select(_mapper.Map<Protocol.Item>).ToList(),
                Spells = spells.Select(_mapper.Map<Protocol.Spell>).ToList(),
                MatchmakingSkills = matchmakingSkills.Select(_mapper.Map<Protocol.MatchmakingSkill>).ToList(),
                Achievements = achievements.Select(_mapper.Map<Protocol.Achievement>).ToList(),
                Quests = quests.Select(_mapper.Map<Protocol.Quest>).ToList(),
                StorageBoxes = storageBoxes
                    .Where(box => box.ExpiredDate == null || box.ExpiredDate > now)
                    .Select(_mapper.Map<Protocol.StorageBox>)
                    .ToList(),
                MarketplacePendings = marketplacePendings.Select(_mapper.Map<Protocol.MarketplacePending>).ToList(),
                Option = _mapper.Map<Protocol.Option>(option),
                Clan = sync.Clan,
                Group = sync.Group,
                Mail = await _dbContext.Mail.Unread(world, uid),
                SystemMailIds = await _dbContext.Mail.GetSystemMailIds(world, uid)
            };
        }

        private void ApplyHashEntitySnapshot<T>(
            T[] request,
            IReadOnlyList<T> existing,
            Action<IReadOnlyList<T>> deleteMany,
            Action<T[]> setMany) where T : class, IModel, IRedisHashKey
        {
            var src = request.ToDictionary(x => $"{x.GetRedisKey()}:{x.GetRedisField()}");
            var dst = existing.ToDictionary(x => $"{x.GetRedisKey()}:{x.GetRedisField()}");

            var removed = dst.Keys.Except(src.Keys).Select(k => dst[k]).ToList();
            if (removed.Count > 0)
            {
                _logger.LogWarning($"deleted keys : {string.Join(", ", removed.Select(x => $"{x.GetRedisKey()}:{x.GetRedisField()}"))}");
                deleteMany(removed);
            }

            if (request.Length > 0)
                setMany(request);
        }

        private void ApplyMarketplacePendingSnapshot(
            uint world,
            uint userId,
            MarketplacePending[] request,
            IReadOnlyList<MarketplacePending> existing)
        {
            var src = request.ToDictionary(x => x.PendingKey);
            var existingByKey = existing.ToDictionary(x => x.PendingKey);

            var removedKeys = existingByKey.Keys.Except(src.Keys).ToArray();
            if (removedKeys.Length > 0)
                _dbContext.MarketplacePending.Delete(world, userId, removedKeys);

            if (src.Count > 0)
                _dbContext.MarketplacePending.Set(world, src.Values.ToArray());
        }

        [HttpPost("save")]
        public async Task<Response.Save> Save(Request.Save request)
        {
            try
            {
                if (request.Payload == null)
                    throw new Exception("Save request data is null");

                await ApplySavePayload(request.World, new List<Protocol.SavePayload> { request.Payload });
                await _dbContext.SaveChangesAsync();

                await _logService.WriteAsync("character_save", new
                {
                    character_id = request.Payload.Character.Id,
                    character_name = request.Payload.Character.Name,
                    item_count = request.Payload.Items?.Count ?? 0,
                    spell_count = request.Payload.Spells?.Count ?? 0,
                    matchmaking_skill_count = request.Payload.MatchmakingSkills?.Count ?? 0,
                    achievement_count = request.Payload.Achievements?.Count ?? 0,
                    quest_count = request.Payload.Quests?.Count ?? 0
                });

                return new Response.Save
                {
                    Success = true
                };
            }
            catch (Exception e)
            {
                _logger.LogError(e, "Save failed for character {CharacterId}", request.Payload?.Character?.Id);
                return new Response.Save
                {
                    Success = false
                };
            }
        }

        [HttpPost("save-batch")]
        public async Task<Response.BatchSave> SaveBatch(Request.SaveBatch request)
        {
            try
            {
                if (request.Characters == null || request.Characters.Count == 0)
                    return new Response.BatchSave { Success = true };

                await ApplySavePayload(request.World, request.Characters);
                await _dbContext.SaveChangesAsync();

                await _logService.WriteAsync("character_save_batch", new
                {
                    world = request.World,
                    character_count = request.Characters.Count
                });

                return new Response.BatchSave
                {
                    Success = true
                };
            }
            catch (Exception e)
            {
                _logger.LogError(e, "BatchSave failed for world {World} with {Count} characters", request.World, request.Characters?.Count ?? 0);
                return new Response.BatchSave
                {
                    Success = false
                };
            }
        }

        private async Task ApplySavePayload(uint world, IReadOnlyList<Protocol.SavePayload> payloads)
        {
            if (payloads == null || payloads.Count == 0)
                return;

            var characterIds = payloads.Select(p => p.Character.Id).Distinct().ToList();
            var charactersTask = _dbContext.Character.GetMany(world, characterIds);
            var itemsTask = _dbContext.Item.GetMany(world, characterIds);
            var spellsTask = _dbContext.Spell.GetMany(world, characterIds);
            var achievementsTask = _dbContext.Achievement.GetMany(world, characterIds);
            var questsTask = _dbContext.Quest.GetMany(world, characterIds);
            var marketplacePendingsTask = _dbContext.MarketplacePending.GetMany(world, characterIds);

            await Task.WhenAll(charactersTask, itemsTask, spellsTask, achievementsTask, questsTask,
                marketplacePendingsTask);

            var characters = await charactersTask;
            var itemsByOwner = await itemsTask;
            var spellsByOwner = await spellsTask;
            var achievementsByOwner = await achievementsTask;
            var questsByOwner = await questsTask;
            var marketplacePendingsByOwner = await marketplacePendingsTask;

            foreach (var data in payloads)
            {
                var characterId = data.Character.Id;
                if (!characters.TryGetValue(characterId, out var existingCharacter))
                    throw new Exception($"Character not found: {characterId}");

                ApplyOneSavePayload(world, data,
                    itemsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Item>(),
                    spellsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Spell>(),
                    achievementsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Achievement>(),
                    questsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Quest>(),
                    marketplacePendingsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<MarketplacePending>());
            }
        }

        private void ApplyOneSavePayload(
            uint world,
            Protocol.SavePayload data,
            IReadOnlyList<Item> existingItems,
            IReadOnlyList<Spell> existingSpells,
            IReadOnlyList<Achievement> existingAchievements,
            IReadOnlyList<Quest> existingQuests,
            IReadOnlyList<MarketplacePending> existingMarketplacePendings)
        {
            var characterId = data.Character.Id;

            var ch = _mapper.Map<Character>(data.Character);
            _dbContext.Character.Set(world, ch);

            var marriage = _mapper.Map<Http.Model.Marriage>(data.Marriage);
            marriage.CharacterId = characterId;
            marriage.UpdatedDate = DateTime.Now;
            _dbContext.Marriage.Set(world, marriage);

            var items = _mapper.Map<Protocol.Item[], Item[]>(data.Items?.ToArray() ?? Array.Empty<Protocol.Item>());
            ApplyHashEntitySnapshot(
                items,
                existingItems,
                removed => _dbContext.Item.Delete(world, removed),
                alive => _dbContext.Item.Set(world, alive));

            var spells = _mapper.Map<Protocol.Spell[], Spell[]>(data.Spells?.ToArray() ?? Array.Empty<Protocol.Spell>());
            ApplyHashEntitySnapshot(
                spells,
                existingSpells,
                removed => _dbContext.Spell.Delete(world, removed),
                alive => _dbContext.Spell.Set(world, alive));

            var matchmakingSkills = _mapper.Map<Protocol.MatchmakingSkill[], MatchmakingSkill[]>(
                data.MatchmakingSkills?.ToArray() ?? Array.Empty<Protocol.MatchmakingSkill>());
            if (matchmakingSkills.Length > 0)
                _dbContext.MatchmakingSkill.Set(world, matchmakingSkills);

            var achievements = _mapper.Map<Protocol.Achievement[], Achievement[]>(data.Achievements?.ToArray() ?? Array.Empty<Protocol.Achievement>());
            ApplyHashEntitySnapshot(
                achievements,
                existingAchievements,
                removed => _dbContext.Achievement.Delete(world, removed),
                alive => _dbContext.Achievement.Set(world, alive));

            var quests = _mapper.Map<Protocol.Quest[], Quest[]>(data.Quests?.ToArray() ?? Array.Empty<Protocol.Quest>());
            ApplyHashEntitySnapshot(
                quests,
                existingQuests,
                removed => _dbContext.Quest.Delete(world, removed),
                alive => _dbContext.Quest.Set(world, alive));

            var marketplacePendings = _mapper.Map<Protocol.MarketplacePending[], MarketplacePending[]>(
                data.MarketplacePendings?.ToArray() ?? Array.Empty<Protocol.MarketplacePending>());
            ApplyMarketplacePendingSnapshot(world, characterId, marketplacePendings, existingMarketplacePendings);
        }

        [HttpPost("option")]
        public async Task<Response.SetOption> Option(Request.SetOption request)
        {
            try
            {
                var world = request.World;

                var option = await _dbContext.Option.Get(world, request.User) ??
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

                    case Fb.Model.EnumValue.Option.News:
                        option.News = request.Enabled;
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
    }
}
