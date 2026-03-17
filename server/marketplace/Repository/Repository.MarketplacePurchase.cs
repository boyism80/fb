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

        public async Task<string> CreatePurchaseAsync(
            string purchaseId,
            uint world,
            string listingId,
            uint buyerId,
            ushort purchaseCount,
            uint purchasePrice,
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

        public async Task<Dictionary<string, MarketplacePurchase>> GetPurchasesByIdsAsync(List<string> purchaseIds)
        {
            if (purchaseIds == null || purchaseIds.Count == 0)
            {
                return new Dictionary<string, MarketplacePurchase>();
            }

            await using var conn = _dbContext.GetUnifiedConnection();
            var parameters = new DynamicParameters();
            parameters.Add("PurchaseIds", purchaseIds);

            var sql = @"
                SELECT * FROM `marketplace_purchase` 
                WHERE `id` IN @PurchaseIds";

            var purchases = await conn.QueryAsync<MarketplacePurchase>(sql, parameters);
            return purchases.ToDictionary(p => p.Id, p => p);
        }

        public async Task<List<MarketplacePurchase>> GetPurchasesByListingIdAsync(string listingId)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $@"
                SELECT * FROM `marketplace_purchase` 
                WHERE `listing_id` = {listingId.Escape()}
                ORDER BY `created_date` DESC";

            return (await conn.QueryAsync<MarketplacePurchase>(sql)).ToList();
        }

        public async Task<bool> CheckPurchaseIdExistsAsync(string purchaseId)
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