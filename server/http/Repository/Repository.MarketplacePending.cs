using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;
using StackExchange.Redis;
namespace Http.Reepository
{
    public class MarketplacePendingRepository : RedisHashRepository<MarketplacePending, MarketplacePendingKey>
    {
        public MarketplacePendingRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }

        public Task<IEnumerable<MarketplacePending>> Get(uint world, uint user)
        {
            return base.GetAll(world, new MarketplacePendingKey
            {
                User = user,
                PendingKey = string.Empty
            });
        }

        protected override string OnSelect(MarketplacePendingKey key)
        {
            return $"""
                SELECT * FROM `marketplace_pending`
                WHERE `user` = {key.User} AND `pending_key` = {key.PendingKey.Escape()}
                LIMIT 1;
                """;
        }

        protected override string OnSelectBulk(MarketplacePendingKey key)
        {
            return $"""
                SELECT * FROM `marketplace_pending`
                WHERE `user` = {key.User};
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<MarketplacePendingKey> keys)
        {
            return $"SELECT * FROM `marketplace_pending` WHERE `user` IN ({string.Join(",", keys.Select(k => k.User))});";
        }

        protected override MarketplacePendingKey GetKeyFromRow(MarketplacePending row)
        {
            return new MarketplacePendingKey { User = row.User, PendingKey = row.PendingKey };
        }

        public async Task<IReadOnlyDictionary<uint, IReadOnlyList<MarketplacePending>>> GetMany(uint world, IReadOnlyList<uint> ownerIds)
        {
            if (ownerIds == null || ownerIds.Count == 0)
                return new Dictionary<uint, IReadOnlyList<MarketplacePending>>();

            var keys = ownerIds.Distinct().Select(uid => new MarketplacePendingKey { User = uid, PendingKey = string.Empty }).ToList();
            var list = await base.GetMany(world, keys);
            var dict = keys.Distinct().ToDictionary(k => k.User, _ => (IList<MarketplacePending>)new List<MarketplacePending>());
            foreach (var row in list)
            {
                dict[row.User].Add(row);
            }

            return dict.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<MarketplacePending>)kv.Value);
        }

        public void DeleteMany(uint world, uint user, IReadOnlyList<string> pendingKeys)
        {
            if (pendingKeys == null || pendingKeys.Count == 0)
                return;

            var key = new MarketplacePendingKey
            {
                User = user,
                PendingKey = string.Empty
            };
            var fields = pendingKeys.Select(k => (RedisValue)k).ToList();
            var sql = $"""
                DELETE FROM `marketplace_pending`
                WHERE `user` = {user} AND `pending_key` IN ({string.Join(",", pendingKeys.Select(k => k.Escape()))});
                """;

            DeleteFields(world, key, fields, sql);
        }

        protected override string OnUpsert(MarketplacePending value)
        {
            var attachmentsJson = JsonConvert.SerializeObject(value.Attachments ?? new List<Fb.Model.Dsl>());

            return $"""
                INSERT INTO `marketplace_pending` (
                    `user`,
                    `pending_key`,
                    `type`,
                    `purchase_id`,
                    `listing_id`,
                    `attachments`,
                    `expected_purchase_count`,
                    `expected_total_price`,
                    `character_id`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.User.Escape()},
                    {value.PendingKey.Escape()},
                    {value.Type.Escape()},
                    {value.PurchaseId.Escape()},
                    {value.ListingId.Escape()},
                    {attachmentsJson.Escape()},
                    {value.ExpectedPurchaseCount.Escape()},
                    {value.ExpectedTotalPrice.Escape()},
                    {value.CharacterId.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE
                    `type`=VALUES(`type`),
                    `purchase_id`=VALUES(`purchase_id`),
                    `listing_id`=VALUES(`listing_id`),
                    `attachments`=VALUES(`attachments`),
                    `expected_purchase_count`=VALUES(`expected_purchase_count`),
                    `expected_total_price`=VALUES(`expected_total_price`),
                    `character_id`=VALUES(`character_id`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }

        protected override string OnUpsert(MarketplacePending[] values)
        {
            if (values.Length == 0)
                return string.Empty;

            var args = values.Select(item =>
            {
                var attachmentsJson = JsonConvert.SerializeObject(item.Attachments ?? new List<Fb.Model.Dsl>());
                return $"""
                        ({item.User.Escape()},
                         {item.PendingKey.Escape()},
                         {item.Type.Escape()},
                         {item.PurchaseId.Escape()},
                         {item.ListingId.Escape()},
                         {attachmentsJson.Escape()},
                         {item.ExpectedPurchaseCount.Escape()},
                         {item.ExpectedTotalPrice.Escape()},
                         {item.CharacterId.Escape()},
                         {item.CreatedDate.Escape()},
                         {item.UpdatedDate.Escape()})
                        """;
            });

            return $"""
                INSERT INTO `marketplace_pending` (
                    `user`,
                    `pending_key`,
                    `type`,
                    `purchase_id`,
                    `listing_id`,
                    `attachments`,
                    `expected_purchase_count`,
                    `expected_total_price`,
                    `character_id`,
                    `created_date`,
                    `updated_date`)
                VALUES {string.Join(',', args)}
                ON DUPLICATE KEY UPDATE
                    `type`=VALUES(`type`),
                    `purchase_id`=VALUES(`purchase_id`),
                    `listing_id`=VALUES(`listing_id`),
                    `attachments`=VALUES(`attachments`),
                    `expected_purchase_count`=VALUES(`expected_purchase_count`),
                    `expected_total_price`=VALUES(`expected_total_price`),
                    `character_id`=VALUES(`character_id`),
                    `updated_date`=VALUES(`updated_date`);
                """;
        }
    }
}
