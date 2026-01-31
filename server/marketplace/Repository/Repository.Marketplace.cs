using Dapper;
using fb.protocol.marketplace;
using Http.Extension;
using Http.Reepository;
using Marketplace.Model;
using Marketplace.Service;

namespace Marketplace.Reepository
{
    /// <summary>
    /// Provides repository functionality for marketplace system data management.
    /// Implements direct database operations for marketplace-related functionality.
    /// </summary>
    public class MarketplaceRepository : IRepository
    {
        private readonly DbContext _dbContext;

        /// <summary>
        /// Initializes a new instance of the <see cref="MarketplaceRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        public MarketplaceRepository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        /// <summary>
        /// Retrieves a marketplace listing by its unique identifier.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <returns>The marketplace listing if found; otherwise, null.</returns>
        public async Task<MarketplaceListing> GetListingByIdAsync(string listingId)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $@"
                SELECT * FROM `marketplace_listing` 
                WHERE `id` = {listingId.Escape()} AND `status` != {ListingState.EXPIRED.Escape()}";

            return await conn.QueryFirstOrDefaultAsync<MarketplaceListing>(sql);
        }

        /// <summary>
        /// Retrieves a marketplace listing by its unique identifier with row lock for update.
        /// Used for purchase operations to prevent concurrent purchases.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="transaction">Optional database transaction.</param>
        /// <returns>The marketplace listing if found; otherwise, null.</returns>
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

        /// <summary>
        /// Retrieves multiple marketplace listings by their unique identifiers.
        /// </summary>
        /// <param name="listingIds">List of unique identifiers of the listings (UUID strings).</param>
        /// <returns>List of marketplace listings found; empty list if none found.</returns>
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

        /// <summary>
        /// Retrieves purchase records for specific listings and buyer.
        /// </summary>
        /// <param name="listingIds">List of listing IDs to search purchases for.</param>
        /// <param name="buyerId">Buyer ID to filter purchases.</param>
        /// <returns>Dictionary mapping listing IDs to purchase records; empty dictionary if none found.</returns>
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


        /// <summary>
        /// Creates a new marketplace listing.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="world">The world identifier (e.g., 1, 2).</param>
        /// <param name="sellerId">The unique identifier of the seller.</param>
        /// <param name="itemModel">The item model identifier.</param>
        /// <param name="remainingCount">The number of items available for purchase.</param>
        /// <param name="itemDurability">The durability of the item (nullable).</param>
        /// <param name="itemCustomName">The custom name of the item (nullable).</param>
        /// <param name="price">The per unit price set by the seller.</param>
        /// <param name="expireDate">The expiration date and time for the listing.</param>
        /// <returns>The unique identifier of the created listing (UUID string).</returns>
        public async Task<string> CreateListingAsync(
            string listingId,
            uint world,
            uint sellerId,
            uint itemModel,
            ushort remainingCount,
            uint? itemDurability,
            string itemCustomName,
            uint price,
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

        /// <summary>
        /// Updates the status of a marketplace listing.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="status">The new status.</param>
        /// <returns>True if the update was successful; otherwise, false.</returns>
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

        /// <summary>
        /// Updates a marketplace listing with purchase information.
        /// Decrements remaining_count and updates status/sold_date conditionally.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="purchaseCount">The number of items purchased (to be deducted from remaining_count).</param>
        /// <param name="transaction">Optional database transaction.</param>
        /// <returns>Tuple containing (success: bool, new_remaining_count: ushort, is_sold: bool).</returns>
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

        /// <summary>
        /// Checks if a listing ID already exists in the database.
        /// Used for ID duplication validation.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <returns>True if the listing ID exists; otherwise, false.</returns>
        public async Task<bool> CheckListingIdExistsAsync(string listingId)
        {
            await using var conn = _dbContext.GetUnifiedConnection();
            var sql = $@"
                SELECT COUNT(*) FROM `marketplace_listing` 
                WHERE `id` = {listingId.Escape()}";

            var count = await conn.QuerySingleAsync<int>(sql);
            return count > 0;
        }

        /// <summary>
        /// Searches marketplace listings based on specified criteria.
        /// Item model filtering is done in the WHERE clause.
        /// </summary>
        /// <param name="itemModelIds">List of item model IDs to filter by (nullable).</param>
        /// <param name="minPrice">Minimum price filter (nullable).</param>
        /// <param name="maxPrice">Maximum price filter (nullable).</param>
        /// <param name="sellerId">Seller ID filter (nullable).</param>
        /// <param name="sortBy">Sort order (price_asc, price_desc, created_desc).</param>
        /// <param name="page">Page number (1-based).</param>
        /// <param name="pageSize">Number of items per page.</param>
        /// <param name="transaction">Optional database transaction for atomic operations.</param>
        /// <returns>A list of marketplace listings matching the criteria.</returns>
        public async Task<List<MarketplaceListing>> SearchListingsAsync(
            List<uint> itemModelIds,
            uint? minPrice,
            uint? maxPrice,
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

        /// <summary>
        /// Counts marketplace listings matching the specified criteria.
        /// Item model filtering is done in the WHERE clause.
        /// </summary>
        /// <param name="itemModelIds">List of item model IDs to filter by (nullable).</param>
        /// <param name="minPrice">Minimum price filter (nullable).</param>
        /// <param name="maxPrice">Maximum price filter (nullable).</param>
        /// <param name="sellerId">Seller ID filter (nullable).</param>
        /// <param name="transaction">Optional database transaction for atomic operations.</param>
        /// <returns>The total count of listings matching the criteria.</returns>
        public async Task<int> CountListingsAsync(
            List<uint> itemModelIds,
            uint? minPrice,
            uint? maxPrice,
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

        /// <summary>
        /// Counts active marketplace listings for a specific seller.
        /// Active listings are those with status = ACTIVE and expire_date > NOW().
        /// </summary>
        /// <param name="sellerId">The unique identifier of the seller.</param>
        /// <returns>The count of active listings for the seller.</returns>
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

        /// <summary>
        /// Saves any pending changes to the underlying data store.
        /// This implementation returns a completed task as marketplace operations are immediately persisted.
        /// </summary>
        /// <returns>A completed task representing the save operation.</returns>
        public Task SaveChangesAsync()
        {
            return Task.CompletedTask;
        }
    }
}

