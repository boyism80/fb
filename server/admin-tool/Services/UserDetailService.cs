using Dapper;
using Http.Model;
using Http.Reepository;
using Http.Service;

namespace AdminTool.Services
{
    public class UserDetailService
    {
        private readonly DbContext _dbContext;
        private readonly UserService _userService;
        private readonly MarketplaceAdminService _marketplaceAdminService;
        private readonly ILogger<UserDetailService> _logger;

        public UserDetailService(
            DbContext dbContext,
            UserService userService,
            MarketplaceAdminService marketplaceAdminService,
            ILogger<UserDetailService> logger)
        {
            _dbContext = dbContext;
            _userService = userService;
            _marketplaceAdminService = marketplaceAdminService;
            _logger = logger;
        }

        public async Task<UserDetailCore> LoadCoreAsync(uint world, uint userId)
        {
            var character = await _dbContext.Character.Get(world, userId);
            if (character == null)
                return null;

            var userDetail = await _userService.GetUserByName(world, character.Name);
            if (userDetail == null)
                return null;

            var isOnline = await _userService.IsOnline(world, character.Name);
            CharacterRealtimeState realtimeState = null;

            try
            {
                realtimeState = await _dbContext.CharacterRealtimeState.Get(world, userId);
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to load realtime state for user {UserId}", userId);
            }

            return new UserDetailCore
            {
                Character = character,
                UserDetail = userDetail,
                IsOnline = isOnline,
                RealtimeState = realtimeState
            };
        }

        public async Task<UserDetailItemsData> LoadItemsAsync(uint world, uint userId)
        {
            var allItems = (await _dbContext.Item.Get(world, userId)).ToList();
            return new UserDetailItemsData
            {
                Equipped = allItems.Where(i => i.Stored == -1 && i.Parts != 0).ToList(),
                Inventory = allItems.Where(i => i.Stored == -1 && i.Parts == 0).ToList(),
                Storage = allItems.Where(i => i.Stored != -1).ToList()
            };
        }

        public async Task<UserDetailProgressData> LoadProgressAsync(uint world, uint userId)
        {
            var quests = (await _dbContext.Quest.Get(world, userId))
                .OrderBy(q => q.Completed)
                .ThenBy(q => q.Id)
                .ToList();

            var spells = (await _dbContext.Spell.Get(world, userId))
                .OrderBy(s => s.Slot)
                .ToList();

            var achievements = (await _dbContext.Achievement.Get(world, userId))
                .OrderBy(a => a.Id)
                .ToList();

            return new UserDetailProgressData
            {
                Quests = quests,
                Spells = spells,
                Achievements = achievements
            };
        }

        public async Task<UserDetailSocialData> LoadSocialAsync(uint world, uint userId, CharacterRealtimeState realtimeState)
        {
            var social = new UserDetailSocialData();

            uint? clanId = realtimeState?.Clan;
            if (!clanId.HasValue)
            {
                clanId = await TryFindClanIdAsync(world, userId);
            }

            if (clanId.HasValue)
            {
                try
                {
                    var clan = await _dbContext.Clan.Get(world, clanId.Value);
                    var member = await _dbContext.ClanMember.Get(world, clanId.Value, userId);
                    if (clan != null)
                    {
                        social.Clan = new UserClanInfo
                        {
                            Id = clan.Id,
                            Name = clan.Name,
                            Role = member?.Role ?? 0,
                            JoinedDate = member?.CreatedDate
                        };
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to load clan for user {UserId}", userId);
                }
            }

            try
            {
                social.Party = await LoadPartyAsync(world, userId, realtimeState);
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to load party for user {UserId}", userId);
            }

            try
            {
                var marriage = await _dbContext.Marriage.Get(world, userId);
                if (marriage != null)
                    social.Marriage = marriage;
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to load marriage for user {UserId}", userId);
            }

            try
            {
                var listings = await _marketplaceAdminService.GetListingsAsync(
                    world,
                    page: 1,
                    pageSize: 50,
                    sellerIdText: userId.ToString(),
                    status: 0);

                social.MarketListings = listings.Listings;
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to load marketplace listings for user {UserId}", userId);
            }

            return social;
        }

        public async Task<UserDetailMailData> LoadMailPageAsync(
            uint world,
            uint userId,
            int page,
            int pageSize,
            string filter)
        {
            var mailFilter = ParseMailFilter(filter);
            var safePage = Math.Max(1, page);
            var safePageSize = Math.Clamp(pageSize, 1, 100);
            var offset = (safePage - 1) * safePageSize;

            var totalCount = await _dbContext.Mail.CountByUser(world, userId, mailFilter);
            var mails = await _dbContext.Mail.GetAdminList(world, userId, offset, safePageSize, mailFilter);
            var unreadCount = await _dbContext.Mail.CountByUser(world, userId, MailRepository.AdminMailFilter.Unread);

            var senderIds = mails
                .Where(m => m.Sender != 0)
                .Select(m => m.Sender)
                .Distinct()
                .ToList();

            var senderNames = senderIds.Count > 0
                ? await ResolveCharacterNamesAsync(world, senderIds)
                : new Dictionary<uint, string>();

            return new UserDetailMailData
            {
                Mails = mails,
                SenderNames = senderNames,
                TotalCount = totalCount,
                Page = safePage,
                PageSize = safePageSize,
                Filter = filter,
                UnreadCount = unreadCount
            };
        }

        private static MailRepository.AdminMailFilter ParseMailFilter(string filter)
        {
            return filter switch
            {
                "unread" => MailRepository.AdminMailFilter.Unread,
                "system" => MailRepository.AdminMailFilter.System,
                _ => MailRepository.AdminMailFilter.All
            };
        }

        public async Task<Option> LoadOptionsAsync(uint world, uint userId)
        {
            try
            {
                var option = await _dbContext.Option.Get(world, userId);
                return option;
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to load options for user {UserId}", userId);
                return null;
            }
        }

        public async Task<string> ResolveCharacterNameAsync(uint world, uint userId)
        {
            try
            {
                var character = await _dbContext.Character.Get(world, userId);
                return character?.Name;
            }
            catch
            {
                return null;
            }
        }

        public async Task<Dictionary<uint, string>> ResolveCharacterNamesAsync(uint world, IReadOnlyList<uint> userIds)
        {
            if (userIds == null || userIds.Count == 0)
                return new Dictionary<uint, string>();

            var characters = await _dbContext.Character.GetMany(world, userIds);
            return characters.ToDictionary(kv => kv.Key, kv => kv.Value.Name);
        }

        private async Task<uint?> TryFindClanIdAsync(uint world, uint userId)
        {
            try
            {
                await using var conn = _dbContext.GetShardConnection(world, userId);
                return await conn.QueryFirstOrDefaultAsync<uint?>(
                    "SELECT `clan` FROM `clan_member` WHERE `user` = @userId AND `deleted` = 0 LIMIT 1",
                    new { userId });
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to query clan membership for user {UserId}", userId);
                return null;
            }
        }

        private async Task<UserPartyInfo> LoadPartyAsync(uint world, uint userId, CharacterRealtimeState realtimeState)
        {
            uint? masterId = realtimeState?.Group;
            Group group = null;

            if (masterId.HasValue)
            {
                group = await _dbContext.Group.Get(world, masterId.Value);
            }
            else
            {
                group = await _dbContext.Group.Get(world, userId);
                if (group != null)
                    masterId = userId;
            }

            if (group == null || !masterId.HasValue)
                return null;

            var members = group.Members ?? new List<uint>();
            var allMemberIds = members
                .Append(masterId.Value)
                .Distinct()
                .ToList();

            var names = await _dbContext.Character.GetMany(world, allMemberIds);
            var masterName = names.TryGetValue(masterId.Value, out var masterChar)
                ? masterChar.Name
                : $"UID {masterId.Value}";

            return new UserPartyInfo
            {
                MasterId = masterId.Value,
                MasterName = masterName,
                IsMaster = userId == masterId.Value,
                MemberCount = allMemberIds.Count
            };
        }
    }

    public class UserDetailCore
    {
        public Character Character { get; set; } = null!;

        public UserDetail UserDetail { get; set; } = null!;

        public bool IsOnline { get; set; }

        public CharacterRealtimeState RealtimeState { get; set; }
    }

    public class UserDetailItemsData
    {
        public List<Http.Model.Item> Equipped { get; set; } = new();

        public List<Http.Model.Item> Inventory { get; set; } = new();

        public List<Http.Model.Item> Storage { get; set; } = new();
    }

    public class UserDetailProgressData
    {
        public List<Quest> Quests { get; set; } = new();

        public List<Spell> Spells { get; set; } = new();

        public List<Achievement> Achievements { get; set; } = new();
    }

    public class UserDetailSocialData
    {
        public UserClanInfo Clan { get; set; }

        public UserPartyInfo Party { get; set; }

        public Marriage Marriage { get; set; }

        public List<MarketplaceListingItem> MarketListings { get; set; } = new();
    }

    public class UserClanInfo
    {
        public uint Id { get; set; }

        public string Name { get; set; } = string.Empty;

        public uint Role { get; set; }

        public DateTime? JoinedDate { get; set; }
    }

    public class UserPartyInfo
    {
        public uint MasterId { get; set; }

        public string MasterName { get; set; } = string.Empty;

        public bool IsMaster { get; set; }

        public int MemberCount { get; set; }
    }

    public class UserDetailMailData
    {
        public List<Mail> Mails { get; set; } = new();

        public Dictionary<uint, string> SenderNames { get; set; } = new();

        public int TotalCount { get; set; }

        public int Page { get; set; }

        public int PageSize { get; set; }

        public string Filter { get; set; } = "all";

        public int UnreadCount { get; set; }

        public int TotalPages => PageSize > 0
            ? (int)Math.Ceiling(TotalCount / (double)PageSize)
            : 0;
    }
}
