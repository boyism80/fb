using Dapper;
using Http.Extension;
using Http.Reepository;
using Marketplace.Model;
using Marketplace.Service;

namespace Marketplace.Reepository
{
    public class MarketplaceDeliveryRepository : IRepository
    {
        private readonly DbContext _dbContext;

        public MarketplaceDeliveryRepository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        public async Task CreateAsync(MarketplaceDelivery delivery, System.Data.IDbTransaction transaction)
        {
            var sql = $"""
                INSERT INTO `marketplace_delivery` (
                    `external_ref`,
                    `world`,
                    `user`,
                    `title`,
                    `message`,
                    `attachments`,
                    `created_date`)
                VALUES (
                    {delivery.ExternalRef.Escape()},
                    {delivery.World.Escape()},
                    {delivery.User.Escape()},
                    {delivery.Title.Escape()},
                    {delivery.Message.Escape()},
                    {delivery.Attachments.Escape()},
                    NOW())
                """;

            await transaction.Connection.ExecuteAsync(sql, null, transaction);
        }

        public async Task<List<MarketplaceDelivery>> GetPendingAsync(TimeSpan minAge, int limit)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $"""
                SELECT * FROM `marketplace_delivery`
                WHERE `delivered_date` IS NULL
                  AND `created_date` < NOW() - INTERVAL {(int)minAge.TotalSeconds} SECOND
                ORDER BY `id`
                LIMIT {limit}
                """;

            return (await conn.QueryAsync<MarketplaceDelivery>(sql)).ToList();
        }

        public async Task MarkDeliveredAsync(string externalRef)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            await conn.ExecuteAsync($"""
                UPDATE `marketplace_delivery`
                SET `delivered_date` = NOW()
                WHERE `external_ref` = {externalRef.Escape()} AND `delivered_date` IS NULL
                """);
        }

        public async Task MarkFailedAsync(string externalRef, string error)
        {
            var truncated = error?.Length > 512 ? error[..512] : error;
            await using var conn = _dbContext.GetUnifiedConnection();
            await conn.ExecuteAsync($"""
                UPDATE `marketplace_delivery`
                SET `attempts` = `attempts` + 1, `last_error` = {truncated.Escape()}
                WHERE `external_ref` = {externalRef.Escape()} AND `delivered_date` IS NULL
                """);
        }

        public Task SaveChangesAsync()
        {
            return Task.CompletedTask;
        }
    }
}
