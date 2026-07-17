using Dapper;
using fb.protocol.marketplace;
using Http.Extension;
using Http.Reepository;
using Marketplace.Model;
using Marketplace.Service;

namespace Marketplace.Reepository
{
    public class MarketplaceRepository : IRepository
    {
        private readonly DbContext _dbContext;

        public MarketplaceRepository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        public async Task<MarketplaceListing> GetListingByIdAsync(string listingId)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $@"
                SELECT * FROM `marketplace_listing` 
                WHERE `id` = {listingId.Escape()} AND `status` != {ListingState.EXPIRED.Escape()}";

            return await conn.QueryFirstOrDefaultAsync<MarketplaceListing>(sql);
        }

        public async Task<MarketplaceListing> GetListingByIdForUpdateAsync(string listingId, System.Data.IDbTransaction transaction = null)
        {
            var sql = $@"
                SELECT * FROM `marketplace_listing` 
                WHERE `id` = {listingId.Escape()} AND `status` = {ListingState.ACTIVE.Escape()} AND `expire_date` > NOW()
                FOR UPDATE";

            if (transaction != null)
            {
                return await transaction.Connection.QueryFirstOrDefaultAsync<MarketplaceListing>(sql, null, transaction);
            }
            else
            {
                await using var conn = _dbContext.GetUnifiedConnection();
                return await conn.QueryFirstOrDefaultAsync<MarketplaceListing>(sql);
            }
        }

        public async Task<List<MarketplaceListing>> GetListingsByIdsAsync(List<string> listingIds)
        {
            if (listingIds == null || listingIds.Count == 0)
            {
                return new List<MarketplaceListing>();
            }

            await using var conn = _dbContext.GetUnifiedConnection();
            var parameters = new DynamicParameters();
            parameters.Add("ListingIds", listingIds);

            var sql = $@"
                SELECT * FROM `marketplace_listing` 
                WHERE `id` IN @ListingIds AND `status` != {ListingState.EXPIRED.Escape()}";

            return (await conn.QueryAsync<MarketplaceListing>(sql, parameters)).ToList();
        }

        public async Task<Dictionary<string, MarketplacePurchase>> GetPurchasesByListingIdsAndBuyerAsync(
            List<string> listingIds,
            uint buyerId)
        {
            if (listingIds == null || listingIds.Count == 0)
            {
                return new Dictionary<string, MarketplacePurchase>();
            }

            await using var conn = _dbContext.GetUnifiedConnection();
            var parameters = new DynamicParameters();
            parameters.Add("ListingIds", listingIds);
            parameters.Add("BuyerId", buyerId);

            var sql = @"
                SELECT * FROM `marketplace_purchase` 
                WHERE `listing_id` IN @ListingIds AND `buyer_id` = @BuyerId";

            var purchases = await conn.QueryAsync<MarketplacePurchase>(sql, parameters);
            return purchases.ToDictionary(p => p.ListingId, p => p);
        }


        public async Task<string> CreateListingAsync(
            string listingId,
            uint world,
            uint sellerId,
            uint itemModel,
            ushort remainingCount,
            uint? itemDurability,
            string itemCustomName,
            ulong price,
            DateTime expireDate)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $"""
                INSERT INTO `marketplace_listing` (
                    `id`,
                    `world`,
                    `seller_id`,
                    `item_model`,
                    `remaining_count`,
                    `item_durability`,
                    `item_custom_name`,
                    `price`,
                    `status`,
                    `expire_date`)
                VALUES (
                    {listingId.Escape()},
                    {world.Escape()},
                    {sellerId.Escape()},
                    {itemModel.Escape()},
                    {remainingCount.Escape()},
                    {(itemDurability.HasValue ? itemDurability.Value.Escape() : "NULL")},
                    {(itemCustomName != null ? itemCustomName.Escape() : "NULL")},
                    {price.Escape()},
                    {ListingState.ACTIVE.Escape()},
                    {expireDate.Escape()})
                """;

            await conn.ExecuteAsync(sql);
            return listingId;
        }

        public async Task<bool> UpdateListingStatusAsync(string listingId, ListingState status)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $@"
                UPDATE `marketplace_listing` 
                SET `status` = {status.Escape()},
                    `updated_date` = NOW()
                WHERE `id` = {listingId.Escape()}";

            var rowsAffected = await conn.ExecuteAsync(sql);
            return rowsAffected > 0;
        }

        public async Task<(bool Success, ushort NewRemainingCount, bool IsSold)> UpdateListingAsync(
            string listingId,
            ushort purchaseCount,
            System.Data.IDbTransaction transaction = null)
        {
            // First, get current remaining_count with row lock
            var listing = await GetListingByIdForUpdateAsync(listingId, transaction);
            if (listing == null)
            {
                return (false, 0, false);
            }

            var currentRemaining = listing.RemainingCount;
            if (currentRemaining < purchaseCount)
            {
                // Not enough items available
                return (false, currentRemaining, false);
            }

            var newRemaining = (ushort)(currentRemaining - purchaseCount);
            var isSold = newRemaining == 0;
            var newStatus = isSold ? ListingState.SOLD : ListingState.ACTIVE;

            var sql = $@"
                UPDATE `marketplace_listing` 
                SET `remaining_count` = {newRemaining.Escape()},
                    `status` = {newStatus.Escape()},
                    `sold_date` = {(isSold ? "NOW()" : "NULL")},
                    `updated_date` = NOW()
                WHERE `id` = {listingId.Escape()} 
                    AND `status` = {ListingState.ACTIVE.Escape()} 
                    AND `remaining_count` >= {purchaseCount.Escape()}";

            int rowsAffected;
            if (transaction != null)
            {
                rowsAffected = await transaction.Connection.ExecuteAsync(sql, null, transaction);
            }
            else
            {
                await using var conn = _dbContext.GetUnifiedConnection();
                rowsAffected = await conn.ExecuteAsync(sql);
            }

            return (rowsAffected > 0, newRemaining, isSold);
        }

        public async Task<bool> CheckListingIdExistsAsync(string listingId)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $@"
                SELECT COUNT(*) FROM `marketplace_listing` 
                WHERE `id` = {listingId.Escape()}";

            var count = await conn.QuerySingleAsync<int>(sql);
            return count > 0;
        }

        public async Task<List<MarketplaceListing>> SearchListingsAsync(
            List<uint> itemModelIds,
            ulong? minPrice,
            ulong? maxPrice,
            uint? sellerId,
            string sortBy,
            int page,
            int pageSize,
            System.Data.IDbTransaction transaction = null)
        {
            // Build WHERE conditions
            var whereConditions = new List<string> { $"`status` = {ListingState.ACTIVE.Escape()}", "`expire_date` > NOW()" };
            var parameters = new DynamicParameters();

            if (itemModelIds != null && itemModelIds.Count > 0)
            {
                var escapedIds = string.Join(", ", itemModelIds.Select(id => id.Escape()));
                whereConditions.Add($"`item_model` IN ({escapedIds})");
            }

            if (minPrice.HasValue)
            {
                whereConditions.Add("`price` >= @MinPrice");
                parameters.Add("MinPrice", minPrice.Value);
            }

            if (maxPrice.HasValue)
            {
                whereConditions.Add("`price` <= @MaxPrice");
                parameters.Add("MaxPrice", maxPrice.Value);
            }

            if (sellerId.HasValue)
            {
                whereConditions.Add("`seller_id` = @SellerId");
                parameters.Add("SellerId", sellerId.Value);
            }

            var whereClause = string.Join(" AND ", whereConditions);

            // Build ORDER BY clause
            var orderByClause = sortBy switch
            {
                "price_asc" => "ORDER BY `price` ASC",
                "price_desc" => "ORDER BY `price` DESC",
                "name_price_asc" => "ORDER BY `item_model` ASC, `price` ASC",
                "created_desc" => "ORDER BY `created_date` DESC",
                _ => "ORDER BY `created_date` DESC"
            };

            // Query single database with sorting
            var sql = $@"
                SELECT * FROM `marketplace_listing` 
                WHERE {whereClause}
                {orderByClause}
                LIMIT {pageSize} OFFSET {(page - 1) * pageSize}";

            if (transaction != null)
            {
                return (await transaction.Connection.QueryAsync<MarketplaceListing>(sql, parameters, transaction)).ToList();
            }
            else
            {
                await using var conn = _dbContext.GetUnifiedConnection();
                return (await conn.QueryAsync<MarketplaceListing>(sql, parameters)).ToList();
            }
        }

        public async Task<int> CountListingsAsync(
            List<uint> itemModelIds,
            ulong? minPrice,
            ulong? maxPrice,
            uint? sellerId,
            System.Data.IDbTransaction transaction = null)
        {
            // Build WHERE conditions
            var whereConditions = new List<string> { $"`status` = {ListingState.ACTIVE.Escape()}", "`expire_date` > NOW()" };
            var parameters = new DynamicParameters();

            if (itemModelIds != null && itemModelIds.Count > 0)
            {
                var escapedIds = string.Join(", ", itemModelIds.Select(id => id.Escape()));
                whereConditions.Add($"`item_model` IN ({escapedIds})");
            }

            if (minPrice.HasValue)
            {
                whereConditions.Add("`price` >= @MinPrice");
                parameters.Add("MinPrice", minPrice.Value);
            }

            if (maxPrice.HasValue)
            {
                whereConditions.Add("`price` <= @MaxPrice");
                parameters.Add("MaxPrice", maxPrice.Value);
            }

            if (sellerId.HasValue)
            {
                whereConditions.Add("`seller_id` = @SellerId");
                parameters.Add("SellerId", sellerId.Value);
            }

            var whereClause = string.Join(" AND ", whereConditions);

            // Query single database
            var sql = $"SELECT COUNT(*) FROM `marketplace_listing` WHERE {whereClause}";

            if (transaction != null)
            {
                return await transaction.Connection.QuerySingleAsync<int>(sql, parameters, transaction);
            }
            else
            {
                await using var conn = _dbContext.GetUnifiedConnection();
                return await conn.QuerySingleAsync<int>(sql, parameters);
            }
        }

        public async Task<int> CountActiveListingsBySellerAsync(uint sellerId)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $@"
                SELECT COUNT(*) FROM `marketplace_listing` 
                WHERE `seller_id` = {sellerId.Escape()} 
                    AND `status` = {ListingState.ACTIVE.Escape()} 
                    AND `expire_date` > NOW()";

            return await conn.QuerySingleAsync<int>(sql);
        }

        public Task SaveChangesAsync()
        {
            return Task.CompletedTask;
        }
    }
}
