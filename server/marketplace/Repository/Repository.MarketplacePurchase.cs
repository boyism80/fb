using Dapper;
using Http.Extension;
using Http.Reepository;
using Http.Service;
using Marketplace.Model;

namespace Marketplace.Reepository
{
    public class MarketplacePurchaseRepository : IRepository
    {
        private readonly DbContext _dbContext;

        public MarketplacePurchaseRepository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        public async Task<ulong> CreatePurchaseAsync(
            ulong purchaseId,
            uint world,
            ulong listingId,
            uint buyerId,
            ushort purchaseCount,
            ulong purchasePrice,
            System.Data.IDbTransaction transaction = null)
        {
            var sql = $"""
                INSERT INTO `marketplace_purchase` (
                    `id`,
                    `world`,
                    `listing_id`,
                    `buyer_id`,
                    `purchase_count`,
                    `purchase_price`,
                    `created_date`)
                VALUES (
                    {purchaseId.Escape()},
                    {world.Escape()},
                    {listingId.Escape()},
                    {buyerId.Escape()},
                    {purchaseCount.Escape()},
                    {purchasePrice.Escape()},
                    NOW())
                """;

            if (transaction != null)
            {
                await transaction.Connection.ExecuteAsync(sql, null, transaction);
            }
            else
            {
                await using var conn = _dbContext.GetUnifiedConnection();
                await conn.ExecuteAsync(sql);
            }

            return purchaseId;
        }

        public async Task<MarketplacePurchase> GetAnyPurchaseByIdForUpdateAsync(ulong purchaseId, System.Data.IDbTransaction transaction)
        {
            var sql = $@"
                SELECT * FROM `marketplace_purchase`
                WHERE `id` = {purchaseId.Escape()}
                FOR UPDATE";

            return await transaction.Connection.QueryFirstOrDefaultAsync<MarketplacePurchase>(sql, null, transaction);
        }

        public async Task CreateAbortedPurchaseAsync(
            ulong purchaseId,
            uint world,
            ulong listingId,
            uint buyerId,
            System.Data.IDbTransaction transaction)
        {
            var sql = $"""
                INSERT INTO `marketplace_purchase` (
                    `id`,
                    `world`,
                    `listing_id`,
                    `buyer_id`,
                    `purchase_count`,
                    `purchase_price`,
                    `aborted`,
                    `created_date`)
                VALUES (
                    {purchaseId.Escape()},
                    {world.Escape()},
                    {listingId.Escape()},
                    {buyerId.Escape()},
                    0,
                    0,
                    1,
                    NOW())
                """;

            await transaction.Connection.ExecuteAsync(sql, null, transaction);
        }

        public async Task<bool> CheckPurchaseIdExistsAsync(ulong purchaseId)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $@"
                SELECT COUNT(*) FROM `marketplace_purchase` 
                WHERE `id` = {purchaseId.Escape()}";

            var count = await conn.QuerySingleAsync<int>(sql);
            return count > 0;
        }

        public Task SaveChangesAsync()
        {
            return Task.CompletedTask;
        }
    }
}