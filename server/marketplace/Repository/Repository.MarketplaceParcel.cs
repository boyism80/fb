using Dapper;
using Http.Extension;
using Http.Reepository;
using Marketplace.Model;
using Marketplace.Service;

namespace Marketplace.Reepository
{
    public class MarketplaceParcelRepository : IRepository
    {
        private readonly DbContext _dbContext;

        public MarketplaceParcelRepository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        public async Task CreateAsync(MarketplaceParcel parcel, System.Data.IDbTransaction transaction)
        {
            var sql = $"""
                INSERT INTO `marketplace_parcel` (
                    `external_ref`,
                    `world`,
                    `user`,
                    `title`,
                    `message`,
                    `attachments`,
                    `created_date`)
                VALUES (
                    {parcel.ExternalRef.Escape()},
                    {parcel.World.Escape()},
                    {parcel.User.Escape()},
                    {parcel.Title.Escape()},
                    {parcel.Message.Escape()},
                    {parcel.Attachments.Escape()},
                    UTC_TIMESTAMP(3))
                """;

            await transaction.Connection.ExecuteAsync(sql, null, transaction);
        }

        public async Task<List<MarketplaceParcel>> GetPendingAsync(TimeSpan minAge, int limit)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $"""
                SELECT * FROM `marketplace_parcel`
                WHERE `sent_date` IS NULL
                  AND `created_date` < UTC_TIMESTAMP(3) - INTERVAL {(int)minAge.TotalSeconds} SECOND
                ORDER BY `id`
                LIMIT {limit}
                """;

            return (await conn.QueryAsync<MarketplaceParcel>(sql)).ToList();
        }

        public async Task<bool> ExistsAsync(string externalRef)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            return await conn.ExecuteScalarAsync<int>($"""
                SELECT COUNT(*) FROM `marketplace_parcel`
                WHERE `external_ref` = {externalRef.Escape()}
                """) > 0;
        }

        public async Task MarkSentAsync(string externalRef)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            await conn.ExecuteAsync($"""
                UPDATE `marketplace_parcel`
                SET `sent_date` = UTC_TIMESTAMP(3)
                WHERE `external_ref` = {externalRef.Escape()} AND `sent_date` IS NULL
                """);
        }

        // A sent row is never read again: its storage box already holds the same external_ref.
        public async Task<int> DeleteSentAsync(TimeSpan retention, int limit)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            return await conn.ExecuteAsync($"""
                DELETE FROM `marketplace_parcel`
                WHERE `sent_date` IS NOT NULL
                  AND `sent_date` < UTC_TIMESTAMP(3) - INTERVAL {(long)retention.TotalSeconds} SECOND
                ORDER BY `id`
                LIMIT {limit}
                """);
        }

        public async Task MarkFailedAsync(string externalRef, string error)
        {
            var truncated = error?.Length > 512 ? error[..512] : error;
            await using var conn = _dbContext.GetUnifiedConnection();
            await conn.ExecuteAsync($"""
                UPDATE `marketplace_parcel`
                SET `attempts` = `attempts` + 1, `last_error` = {truncated.Escape()}
                WHERE `external_ref` = {externalRef.Escape()} AND `sent_date` IS NULL
                """);
        }

        public Task SaveChangesAsync()
        {
            return Task.CompletedTask;
        }
    }
}
