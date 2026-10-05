using AutoMapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Model.Redis;
using Http.Service;
using Http.Util;
using Internal.Services;
using Medallion.Threading.Redis;
using Microsoft.AspNetCore.Mvc;
using System.Text.RegularExpressions;
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
        private readonly FriendService _friendService;
        private readonly RedisService _redisService;
        private static readonly TimeSpan SnapshotTimeTtl = TimeSpan.FromDays(1);

        public InGameController(ILogger<InGameController> logger,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            DbContext dbContext,
            IMapper mapper,
            RedisDistributedLockService distributedLock,
            BanService banService,
            ServerStateService serverStateService,
            LogService logService,
            MaintenanceService maintenanceService,
            FriendService friendService,
            RedisService redisService)
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
            _friendService = friendService;
            _redisService = redisService;
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

                var conf = await _serverStateService.GetHostConfig(request.ProcessWorld, Protocol.Service.Game, request.Host);
                if (conf == null)
                    throw new LogicException(ErrorCode.ServerNotReady);

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
                    throw new LogicException(ErrorCode.Offline);

                if (targetWorld != world && SameCrossHost(session, targetSession) == false)
                    throw new LogicException(ErrorCode.Offline);

                var target = await _dbContext.Character.Get(targetWorld, targetSession.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                var targetOption = await _dbContext.Option.Get(targetWorld, targetSession.Uid) ??
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

        [HttpGet("init/{world}/{uid}")]
        public async Task<Response.Init> Init(uint world, uint uid)
        {
            var ch = await _dbContext.Character.Get(world, uid);
            var items = await _dbContext.Item.Get(world, uid);
            var spells = await _dbContext.Spell.Get(world, uid);
            var matchmakingSkills = await _dbContext.MatchmakingSkill.Get(world, uid);
            var achievements = await _dbContext.Achievement.Get(world, uid);
            var quests = await _dbContext.Quest.Get(world, uid);
            var collectionUnlocks = await _dbContext.CollectionUnlock.Get(world, uid);
            var storageBoxes = await _dbContext.StorageBox.Get(world, uid);
            var marketplacePendings = await _dbContext.MarketplacePending.Get(world, uid);
            var friends = await _friendService.GetEntries(world, uid);
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
            var character = _mapper.Map<Protocol.Character>(ch);
            return new Response.Init
            {
                Character = character,
                Marriage = marriageProtocol,
                Items = items.Select(_mapper.Map<Protocol.Item>).ToList(),
                Spells = spells.Select(_mapper.Map<Protocol.Spell>).ToList(),
                MatchmakingSkills = matchmakingSkills.Select(_mapper.Map<Protocol.MatchmakingSkill>).ToList(),
                Achievements = achievements.Select(_mapper.Map<Protocol.Achievement>).ToList(),
                Quests = quests.Select(_mapper.Map<Protocol.Quest>).ToList(),
                CollectionUnlocks = collectionUnlocks.Select(_mapper.Map<Protocol.CollectionUnlock>).ToList(),
                StorageBoxes = storageBoxes
                    .Where(box => box.ExpiredDate == null || box.ExpiredDate > now)
                    .Select(_mapper.Map<Protocol.StorageBox>)
                    .ToList(),
                MarketplacePendings = marketplacePendings.Select(_mapper.Map<Protocol.MarketplacePending>).ToList(),
                Friends = friends,
                Option = _mapper.Map<Protocol.Option>(option),
                Clan = sync.Clan,
                Group = sync.Group,
                Mail = await _dbContext.Mail.Unread(world, uid),
                SystemMailIds = await _dbContext.Mail.GetSystemMailIds(world, uid)
            };
        }

        private void ReplaceHashEntities<T>(
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

        private void ReplaceMarketplacePendings(
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

                var payloads = await OwnedSavePayloads(request.World, request.Host, new List<Protocol.SavePayload> { request.Payload });
                if (payloads.Count == 0)
                    return new Response.Save { Success = false };

                await SaveNewerSnapshots(request.World, payloads);

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

                var payloads = await OwnedSavePayloads(request.World, request.Host, request.Characters);
                await SaveNewerSnapshots(request.World, payloads);

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

        // A missing session is allowed: the new host registers before it reads, so such a save lands before that read.
        private async Task<List<Protocol.SavePayload>> OwnedSavePayloads(uint world, byte? host, IReadOnlyList<Protocol.SavePayload> payloads)
        {
            if (host == null)
                return payloads.ToList();

            var sessions = await _sessionService.GetMany(world, payloads.Select(x => x.Character.Name).Distinct().ToList());
            var owned = new List<Protocol.SavePayload>();
            foreach (var payload in payloads)
            {
                if (sessions.TryGetValue(payload.Character.Name, out var session) && session != null && session.Host != host.Value)
                {
                    _logger.LogWarning("Save rejected for {Name}: session owned by host {Owner}, requested by host {Host}",
                        payload.Character.Name, session.Host, host.Value);
                }
                else
                {
                    owned.Add(payload);
                }
            }
            return owned;
        }

        private async Task SaveNewerSnapshots(uint world, IReadOnlyList<Protocol.SavePayload> payloads)
        {
            if (payloads.Count == 0)
                return;

            var latest = payloads
                .GroupBy(x => x.Character.Id)
                .Select(x => x.MaxBy(payload => payload.SnapshotTime))
                .OrderBy(x => x.Character.Id)
                .ToList();

            var locks = new List<RedisDistributedLockHandle>();
            try
            {
                foreach (var payload in latest)
                    locks.Add(await _distributedLock.Lock(world, Character.SaveLockKey(payload.Character.Id)));

                var appliedTimes = await Task.WhenAll(latest.Select(x =>
                {
                    var key = Character.SnapshotTimeKey(x.Character.Id);
                    return _redisService.GetShardConnection(world, key).Connection.StringGetAsync(key);
                }));

                var newer = new List<Protocol.SavePayload>();
                for (var i = 0; i < latest.Count; i++)
                {
                    var payload = latest[i];
                    if (appliedTimes[i].HasValue && (long)appliedTimes[i] > payload.SnapshotTime)
                    {
                        _logger.LogWarning("Save dropped for {Name}: snapshot {Snapshot} is older than applied {Applied}",
                            payload.Character.Name, payload.SnapshotTime, (long)appliedTimes[i]);
                    }
                    else
                    {
                        newer.Add(payload);
                    }
                }
                if (newer.Count == 0)
                    return;

                await ReplaceSnapshots(world, newer);
                await _dbContext.SaveChangesAsync();

                await Task.WhenAll(newer.Select(x =>
                {
                    var key = Character.SnapshotTimeKey(x.Character.Id);
                    return _redisService.GetShardConnection(world, key).Connection.StringSetAsync(key, x.SnapshotTime, SnapshotTimeTtl);
                }));
            }
            finally
            {
                foreach (var handle in locks)
                    await handle.DisposeAsync();
            }
        }

        private async Task ReplaceSnapshots(uint world, IReadOnlyList<Protocol.SavePayload> payloads)
        {
            if (payloads == null || payloads.Count == 0)
                return;

            var characterIds = payloads.Select(p => p.Character.Id).Distinct().ToList();
            var charactersTask = _dbContext.Character.GetMany(world, characterIds);
            var itemsTask = _dbContext.Item.GetMany(world, characterIds);
            var spellsTask = _dbContext.Spell.GetMany(world, characterIds);
            var achievementsTask = _dbContext.Achievement.GetMany(world, characterIds);
            var questsTask = _dbContext.Quest.GetMany(world, characterIds);
            var collectionUnlocksTask = _dbContext.CollectionUnlock.GetMany(world, characterIds);
            var marketplacePendingsTask = _dbContext.MarketplacePending.GetMany(world, characterIds);

            await Task.WhenAll(charactersTask, itemsTask, spellsTask, achievementsTask, questsTask,
                collectionUnlocksTask, marketplacePendingsTask);

            var characters = await charactersTask;
            var itemsByOwner = await itemsTask;
            var spellsByOwner = await spellsTask;
            var achievementsByOwner = await achievementsTask;
            var questsByOwner = await questsTask;
            var collectionUnlocksByOwner = await collectionUnlocksTask;
            var marketplacePendingsByOwner = await marketplacePendingsTask;

            foreach (var data in payloads)
            {
                var characterId = data.Character.Id;
                if (!characters.TryGetValue(characterId, out var existingCharacter))
                    throw new Exception($"Character not found: {characterId}");

                ReplaceSnapshot(world, data, existingCharacter,
                    itemsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Item>(),
                    spellsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Spell>(),
                    achievementsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Achievement>(),
                    questsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Quest>(),
                    collectionUnlocksByOwner.GetValueOrDefault(characterId) ?? Array.Empty<CollectionUnlock>(),
                    marketplacePendingsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<MarketplacePending>());
            }
        }

        private void ReplaceSnapshot(
            uint world,
            Protocol.SavePayload data,
            Character existingCharacter,
            IReadOnlyList<Item> existingItems,
            IReadOnlyList<Spell> existingSpells,
            IReadOnlyList<Achievement> existingAchievements,
            IReadOnlyList<Quest> existingQuests,
            IReadOnlyList<CollectionUnlock> existingCollectionUnlocks,
            IReadOnlyList<MarketplacePending> existingMarketplacePendings)
        {
            var characterId = data.Character.Id;

            var ch = _mapper.Map<Character>(data.Character);
            ch.Reputation = existingCharacter.Reputation;
            ch.Evaluation = existingCharacter.Evaluation;
            _dbContext.Character.Set(world, ch);

            var marriage = _mapper.Map<Http.Model.Marriage>(data.Marriage);
            marriage.CharacterId = characterId;
            marriage.UpdatedDate = DateTime.Now;
            _dbContext.Marriage.Set(world, marriage);

            var items = _mapper.Map<Protocol.Item[], Item[]>(data.Items?.ToArray() ?? Array.Empty<Protocol.Item>());
            // Escrow rows move between slots and the inventory in one save, so removal and upsert must land together.
            var itemFields = items.Select(x => x.GetRedisField()).ToHashSet();
            var removedItems = existingItems.Where(x => itemFields.Contains(x.GetRedisField()) == false).ToList();
            if (removedItems.Count > 0)
                _logger.LogWarning($"deleted keys : {string.Join(", ", removedItems.Select(x => $"{x.GetRedisKey()}:{x.GetRedisField()}"))}");
            _dbContext.Item.Replace(world, removedItems, items);

            var spells = _mapper.Map<Protocol.Spell[], Spell[]>(data.Spells?.ToArray() ?? Array.Empty<Protocol.Spell>());
            ReplaceHashEntities(
                spells,
                existingSpells,
                removed => _dbContext.Spell.Delete(world, removed),
                alive => _dbContext.Spell.Set(world, alive));

            var matchmakingSkills = _mapper.Map<Protocol.MatchmakingSkill[], MatchmakingSkill[]>(
                data.MatchmakingSkills?.ToArray() ?? Array.Empty<Protocol.MatchmakingSkill>());
            if (matchmakingSkills.Length > 0)
                _dbContext.MatchmakingSkill.Set(world, matchmakingSkills);

            var achievements = _mapper.Map<Protocol.Achievement[], Achievement[]>(data.Achievements?.ToArray() ?? Array.Empty<Protocol.Achievement>());
            ReplaceHashEntities(
                achievements,
                existingAchievements,
                removed => _dbContext.Achievement.Delete(world, removed),
                alive => _dbContext.Achievement.Set(world, alive));

            var quests = _mapper.Map<Protocol.Quest[], Quest[]>(data.Quests?.ToArray() ?? Array.Empty<Protocol.Quest>());
            ReplaceHashEntities(
                quests,
                existingQuests,
                removed => _dbContext.Quest.Delete(world, removed),
                alive => _dbContext.Quest.Set(world, alive));

            var collectionUnlocks = _mapper.Map<Protocol.CollectionUnlock[], CollectionUnlock[]>(
                data.CollectionUnlocks?.ToArray() ?? Array.Empty<Protocol.CollectionUnlock>());
            ReplaceHashEntities(
                collectionUnlocks,
                existingCollectionUnlocks,
                removed => _dbContext.CollectionUnlock.Delete(world, removed),
                alive => _dbContext.CollectionUnlock.Set(world, alive));

            var marketplacePendings = _mapper.Map<Protocol.MarketplacePending[], MarketplacePending[]>(
                data.MarketplacePendings?.ToArray() ?? Array.Empty<Protocol.MarketplacePending>());
            ReplaceMarketplacePendings(world, characterId, marketplacePendings, existingMarketplacePendings);
        }

        [HttpPost("option")]
        public async Task<Response.SetOption> Option(Request.SetOption request)
        {
            try
            {
                var world = request.World;
                if (request.Changes == null || request.Changes.Count == 0)
                    throw new Exception("option changes is empty");

                var option = await _dbContext.Option.Get(world, request.User) ??
                    throw new Exception($"option {request.User} not found");

                foreach (var change in request.Changes)
                    SetOption(option, change.Type, change.Enabled);

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

                default:
                    throw new Exception($"invalid option type : {type}");
            }
        }

        private static bool SameCrossHost(Session session, Session target)
        {
            if (session.World.HasValue || target.World.HasValue)
                return false;

            if (session.Host != target.Host)
                return false;

            return true;
        }
    }
}
