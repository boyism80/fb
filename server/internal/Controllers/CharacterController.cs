using AutoMapper;
using Http.Model;
using Http.Service;
using Internal.Services;
using Medallion.Threading.Redis;
using Microsoft.AspNetCore.Mvc;
using Option = Http.Model.Option;
using Protocol = fb.protocol.@internal;
using Request = fb.protocol.@internal.request;
using Response = fb.protocol.@internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("in-game")]
    public class CharacterController : ControllerBase
    {
        private readonly ILogger<CharacterController> _logger;
        private readonly SessionService _sessionService;
        private readonly DbContext _dbContext;
        private readonly IMapper _mapper;
        private readonly RedisDistributedLockService _distributedLock;
        private readonly LogService _logService;
        private readonly FriendService _friendService;

        public CharacterController(ILogger<CharacterController> logger,
            SessionService sessionService,
            DbContext dbContext,
            IMapper mapper,
            RedisDistributedLockService distributedLock,
            LogService logService,
            FriendService friendService)
        {
            _logger = logger;
            _sessionService = sessionService;
            _dbContext = dbContext;
            _mapper = mapper;
            _distributedLock = distributedLock;
            _logService = logService;
            _friendService = friendService;
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
            var storageBoxes = (await _dbContext.StorageBox.Get(world, uid)).ToList();
            var takenStorageBoxes = storageBoxes
                .Where(x => x.SystemStorageBoxId != null)
                .Select(x => x.SystemStorageBoxId.Value)
                .ToHashSet();
            var archivedStorageBoxes = await _dbContext.SystemStorageBox.GetArchived(world, takenStorageBoxes);
            if (takenStorageBoxes.Count > 0)
            {
                await _dbContext.SystemStorageBox.Archive(world,
                    (await _dbContext.SystemStorageBox.GetPersonal(world, uid)).Where(x => takenStorageBoxes.Contains(x.Id)).ToList());
            }
            var unsettledRequests = (await _dbContext.UnsettledRequest.Get(world, uid)).ToList();
            var escrows = await _dbContext.Escrow.Get(world, uid);
            var friends = await _friendService.GetEntries(world, uid);
            var friendRevision = await _dbContext.FriendRevision.Get(world, uid);
            var activeSystemMailIds = (await _dbContext.SystemMail.GetActive(world)).Select(x => x.Id).ToHashSet();
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
            {
                spouseName = await _dbContext.Character.GetName(world, marriageData.SpouseId.Value) ?? string.Empty;
            }

            var marriageProtocol = _mapper.Map<Protocol.Marriage>(marriageData);
            marriageProtocol.SpouseName = spouseName ?? string.Empty;

            await using var _ = await _distributedLock.Lock(world, CharacterRealtimeState.DistributedLockKey(uid));

            var sync = await _dbContext.CharacterRealtimeState.Get(world, uid) ??
                _dbContext.CharacterRealtimeState.Set(world, new CharacterRealtimeState
                {
                    Uid = uid
                });
            await _dbContext.CharacterRealtimeState.ClearStaleMembership(world, sync);

            await _dbContext.SaveChangesAsync();
            var character = _mapper.Map<Protocol.Character>(ch);
            character.Pw = string.Empty;
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
                StorageBoxes = storageBoxes.Select(_mapper.Map<Protocol.StorageBox>).ToList(),
                UnsettledRequests = unsettledRequests.Select(_mapper.Map<Protocol.UnsettledRequest>).ToList(),
                Escrows = escrows.Select(_mapper.Map<Protocol.Escrow>).ToList(),
                Friends = friends,
                FriendsRevision = friendRevision?.Revision ?? 0,
                ArchivedStorageBoxes = archivedStorageBoxes,
                Option = _mapper.Map<Protocol.Option>(option),
                Clan = sync.Clan,
                Group = sync.Group,
                Mail = await _dbContext.Mail.Unread(world, uid),
                SystemMailIds = (await _dbContext.Mail.GetSystemMailIds(world, uid))
                    .Where(activeSystemMailIds.Contains)
                    .ToList()
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
            {
                setMany(request);
            }
        }

        [HttpPost("save")]
        public async Task<Response.Save> Save(Request.Save request)
        {
            try
            {
                if (request.Payload == null)
                {
                    throw new Exception("Save request data is null");
                }

                var payloads = await OwnedSavePayloads(request.World, request.Host, new List<Protocol.SavePayload> { request.Payload });
                if (payloads.Count == 0)
                {
                    return new Response.Save { Success = false };
                }

                if (await SaveNewerSnapshots(request.World, payloads) == false)
                {
                    return new Response.Save { Success = false };
                }

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
                {
                    return new Response.BatchSave { Success = true };
                }

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
            {
                return payloads.ToList();
            }

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

        private async Task<bool> SaveNewerSnapshots(uint world, IReadOnlyList<Protocol.SavePayload> payloads)
        {
            if (payloads.Count == 0)
            {
                return true;
            }

            var latest = payloads
                .GroupBy(x => x.Character.Id)
                .Select(x => x.MaxBy(payload => payload.SnapshotTime))
                .OrderBy(x => x.Character.Id)
                .ToList();

            var locks = new List<RedisDistributedLockHandle>();
            try
            {
                foreach (var payload in latest)
                {
                    locks.Add(await _distributedLock.Lock(world, Character.SaveLockKey(payload.Character.Id)));
                }

                var characters = await _dbContext.Character.GetMany(world, latest.Select(x => x.Character.Id).ToList());
                var newer = new List<Protocol.SavePayload>();
                foreach (var payload in latest)
                {
                    var applied = characters.GetValueOrDefault(payload.Character.Id)?.SnapshotTime;
                    if (applied.HasValue && applied.Value > payload.SnapshotTime)
                    {
                        _logger.LogWarning("Save dropped for {Name}: snapshot {Snapshot} is older than applied {Applied}",
                            payload.Character.Name, payload.SnapshotTime, applied.Value);
                    }
                    else
                    {
                        newer.Add(payload);
                    }
                }
                if (newer.Count > 0)
                {
                    var previous = await _dbContext.StorageBox.GetMany(world, newer.Select(x => x.Character.Id).ToList());
                    await ReplaceSnapshots(world, newer);
                    await _dbContext.SaveChangesAsync();
                    await ArchiveTakenStorageBoxes(world, newer, previous);
                }
                return newer.Count == latest.Count;
            }
            finally
            {
                foreach (var handle in locks)
                {
                    await handle.DisposeAsync();
                }
            }
        }

        private async Task ArchiveTakenStorageBoxes(uint world,
            IReadOnlyList<Protocol.SavePayload> payloads,
            IReadOnlyDictionary<uint, IReadOnlyList<StorageBox>> previous)
        {
            foreach (var payload in payloads)
            {
                var saved = (previous.GetValueOrDefault(payload.Character.Id) ?? Array.Empty<StorageBox>())
                    .Where(x => x.SystemStorageBoxId != null)
                    .Select(x => x.SystemStorageBoxId.Value)
                    .ToHashSet();
                var taken = (payload.StorageBoxes ?? new List<Protocol.StorageBox>())
                    .Where(x => x.SystemStorageBoxId != 0 && saved.Contains(x.SystemStorageBoxId) == false)
                    .Select(x => x.SystemStorageBoxId)
                    .ToHashSet();
                if (taken.Count == 0)
                {
                    continue;
                }

                try
                {
                    var finished = (await _dbContext.SystemStorageBox.GetPersonal(world, payload.Character.Id))
                        .Where(x => taken.Contains(x.Id))
                        .ToList();
                    await _dbContext.SystemStorageBox.Archive(world, finished);
                }
                catch (Exception e)
                {
                    _logger.LogWarning(e, "Archiving taken storage boxes failed for {Name}", payload.Character.Name);
                }
            }
        }

        private async Task ReplaceSnapshots(uint world, IReadOnlyList<Protocol.SavePayload> payloads)
        {
            if (payloads == null || payloads.Count == 0)
            {
                return;
            }

            var characterIds = payloads.Select(p => p.Character.Id).Distinct().ToList();
            var charactersTask = _dbContext.Character.GetMany(world, characterIds);
            var itemsTask = _dbContext.Item.GetMany(world, characterIds);
            var spellsTask = _dbContext.Spell.GetMany(world, characterIds);
            var achievementsTask = _dbContext.Achievement.GetMany(world, characterIds);
            var questsTask = _dbContext.Quest.GetMany(world, characterIds);
            var collectionUnlocksTask = _dbContext.CollectionUnlock.GetMany(world, characterIds);
            var unsettledTask = _dbContext.UnsettledRequest.GetMany(world, characterIds);
            var escrowTask = _dbContext.Escrow.GetMany(world, characterIds);
            var storageBoxTask = _dbContext.StorageBox.GetMany(world, characterIds);
            var marriageCreatedTask = _dbContext.Marriage.GetCreatedDates(world, characterIds);

            await Task.WhenAll(charactersTask, itemsTask, spellsTask, achievementsTask, questsTask,
                collectionUnlocksTask, unsettledTask, escrowTask, storageBoxTask, marriageCreatedTask);

            var characters = await charactersTask;
            var itemsByOwner = await itemsTask;
            var spellsByOwner = await spellsTask;
            var achievementsByOwner = await achievementsTask;
            var questsByOwner = await questsTask;
            var collectionUnlocksByOwner = await collectionUnlocksTask;
            var unsettledByOwner = await unsettledTask;
            var escrowsByOwner = await escrowTask;
            var storageBoxesByOwner = await storageBoxTask;
            var marriageCreated = await marriageCreatedTask;

            foreach (var data in payloads)
            {
                var characterId = data.Character.Id;
                if (!characters.TryGetValue(characterId, out var existingCharacter))
                {
                    throw new Exception($"Character not found: {characterId}");
                }

                DateTime? createdDate = marriageCreated.TryGetValue(characterId, out var existingMarriageCreated)
                    ? existingMarriageCreated
                    : null;
                ReplaceSnapshot(world, data, existingCharacter, createdDate,
                    itemsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Item>(),
                    spellsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Spell>(),
                    achievementsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Achievement>(),
                    questsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Quest>(),
                    collectionUnlocksByOwner.GetValueOrDefault(characterId) ?? Array.Empty<CollectionUnlock>(),
                    unsettledByOwner.GetValueOrDefault(characterId) ?? Array.Empty<UnsettledRequest>(),
                    escrowsByOwner.GetValueOrDefault(characterId) ?? Array.Empty<Escrow>(),
                    storageBoxesByOwner.GetValueOrDefault(characterId) ?? Array.Empty<StorageBox>());
            }
        }

        private void ReplaceSnapshot(
            uint world,
            Protocol.SavePayload data,
            Character existingCharacter,
            DateTime? marriageCreatedDate,
            IReadOnlyList<Item> existingItems,
            IReadOnlyList<Spell> existingSpells,
            IReadOnlyList<Achievement> existingAchievements,
            IReadOnlyList<Quest> existingQuests,
            IReadOnlyList<CollectionUnlock> existingCollectionUnlocks,
            IReadOnlyList<UnsettledRequest> existingUnsettledRequests,
            IReadOnlyList<Escrow> existingEscrows,
            IReadOnlyList<StorageBox> existingStorageBoxes)
        {
            var characterId = data.Character.Id;

            var ch = _mapper.Map<Character>(data.Character);
            ch.Pw = existingCharacter.Pw;
            ch.SnapshotTime = data.SnapshotTime;
            _dbContext.Character.Set(world, ch);

            var marriage = _mapper.Map<Http.Model.Marriage>(data.Marriage);
            marriage.CharacterId = characterId;
            if (marriageCreatedDate.HasValue)
            {
                marriage.CreatedDate = marriageCreatedDate.Value;
            }

            marriage.UpdatedDate = DateTime.UtcNow;
            _dbContext.Marriage.Set(world, marriage);

            var items = _mapper.Map<Protocol.Item[], Item[]>(data.Items?.ToArray() ?? Array.Empty<Protocol.Item>());
            // Escrow rows move between slots and the inventory in one save, so removal and upsert must land together.
            var itemFields = items.Select(x => x.GetRedisField()).ToHashSet();
            var removedItems = existingItems.Where(x => itemFields.Contains(x.GetRedisField()) == false).ToList();
            if (removedItems.Count > 0)
            {
                _logger.LogWarning($"deleted keys : {string.Join(", ", removedItems.Select(x => $"{x.GetRedisKey()}:{x.GetRedisField()}"))}");
            }

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
            {
                _dbContext.MatchmakingSkill.Set(world, matchmakingSkills);
            }

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

            var unsettledRequests = _mapper.Map<Protocol.UnsettledRequest[], UnsettledRequest[]>(
                data.UnsettledRequests?.ToArray() ?? Array.Empty<Protocol.UnsettledRequest>());
            ReplaceHashEntities(
                unsettledRequests,
                existingUnsettledRequests,
                removed => _dbContext.UnsettledRequest.Delete(world, removed),
                alive => _dbContext.UnsettledRequest.Set(world, alive));

            var escrows = _mapper.Map<Protocol.Escrow[], Escrow[]>(
                data.Escrows?.ToArray() ?? Array.Empty<Protocol.Escrow>());
            ReplaceHashEntities(
                escrows,
                existingEscrows,
                removed => _dbContext.Escrow.Delete(world, removed),
                alive => _dbContext.Escrow.Set(world, alive));

            var storageBoxes = _mapper.Map<Protocol.StorageBox[], StorageBox[]>(
                data.StorageBoxes?.ToArray() ?? Array.Empty<Protocol.StorageBox>());
            foreach (var box in storageBoxes)
            {
                box.User = characterId;
            }

            ReplaceHashEntities(
                storageBoxes,
                existingStorageBoxes,
                removed => _dbContext.StorageBox.Delete(world, removed),
                alive => _dbContext.StorageBox.Set(world, alive));
        }
    }
}
