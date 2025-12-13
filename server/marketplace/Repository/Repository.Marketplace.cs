using Dapper;
using Http.Extension;
using Http.Service;
using Marketplace.Model;

namespace Http.Reepository
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
            await using var conn = _dbContext.Connection(-1);
            var sql = $@"
                SELECT * FROM marketplace_listing 
                WHERE id = {listingId.Escape()} AND status != 3";

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
                SELECT * FROM marketplace_listing 
                WHERE id = {listingId.Escape()} AND status = 0 AND expire_date > NOW()
                FOR UPDATE";

            if (transaction != null)
            {
                return await transaction.Connection.QueryFirstOrDefaultAsync<MarketplaceListing>(sql, null, transaction);
            }
            else
            {
                await using var conn = _dbContext.Connection(-1);
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

            await using var conn = _dbContext.Connection(-1);
            var escapedIds = string.Join(", ", listingIds.Select(id => id.Escape()));

            var sql = $@"
                SELECT * FROM marketplace_listing 
                WHERE id IN ({escapedIds}) AND status != 3";

            return (await conn.QueryAsync<MarketplaceListing>(sql)).ToList();
        }


        /// <summary>
        /// Creates a new marketplace listing.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="sellerId">The unique identifier of the seller.</param>
        /// <param name="itemModel">The item model identifier.</param>
        /// <param name="itemCount">The number of items being listed.</param>
        /// <param name="itemDurability">The durability of the item (nullable).</param>
        /// <param name="itemCustomName">The custom name of the item (nullable).</param>
        /// <param name="price">The listing price set by the seller.</param>
        /// <param name="transactionFee">The fee deducted from seller on sale.</param>
        /// <param name="expireDate">The expiration date and time for the listing.</param>
        /// <returns>The unique identifier of the created listing (UUID string).</returns>
        public async Task<string> CreateListingAsync(
            string listingId,
            uint sellerId,
            uint itemModel,
            ushort itemCount,
            uint? itemDurability,
            string itemCustomName,
            uint price,
            uint transactionFee,
            DateTime expireDate)
        {
            await using var conn = _dbContext.Connection(-1);
            var sql = $"""
                INSERT INTO marketplace_listing (
                    `id`,
                    `seller_id`,
                    `item_model`,
                    `item_count`,
                    `item_durability`,
                    `item_custom_name`,
                    `price`,
                    `transaction_fee`,
                    `status`,
                    `expire_date`)
                VALUES (
                    {listingId.Escape()},
                    {sellerId.Escape()},
                    {itemModel.Escape()},
                    {itemCount.Escape()},
                    {(itemDurability.HasValue ? itemDurability.Value.Escape() : "NULL")},
                    {(itemCustomName != null ? itemCustomName.Escape() : "NULL")},
                    {price.Escape()},
                    {transactionFee.Escape()},
                    {0.Escape()},
                    {expireDate.Escape()})
                """;

            await conn.ExecuteAsync(sql);
            return listingId;
        }

        /// <summary>
        /// Updates the status of a marketplace listing.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="status">The new status (0=Active, 1=Sold, 2=Cancelled, 3=Expired).</param>
        /// <returns>True if the update was successful; otherwise, false.</returns>
        public async Task<bool> UpdateListingStatusAsync(string listingId, byte status)
        {
            await using var conn = _dbContext.Connection(-1);
            var sql = $@"
                UPDATE marketplace_listing (
                    `id`,
                    `status`,
                    `updated_date`)
                VALUES (
                    {listingId.Escape()},
                    {status.Escape()},
                    NOW())
                WHERE id = {listingId.Escape()}";

            var rowsAffected = await conn.ExecuteAsync(sql);
            return rowsAffected > 0;
        }

        /// <summary>
        /// Updates a marketplace listing with sale information.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="status">The new status (typically 1 for Sold).</param>
        /// <param name="soldDate">The date and time when the item was sold (nullable).</param>
        /// <param name="buyerId">The unique identifier of the buyer (nullable).</param>
        /// <param name="transaction">Optional database transaction.</param>
        /// <returns>True if the update was successful; otherwise, false.</returns>
        public async Task<bool> UpdateListingAsync(string listingId, byte status, DateTime? soldDate, uint? buyerId, System.Data.IDbTransaction transaction = null)
        {
            var sql = $@"
                UPDATE marketplace_listing 
                SET `status` = {status.Escape()},
                    `sold_date` = {(soldDate.HasValue ? soldDate.Value.Escape() : "NULL")},
                    `buyer_id` = {(buyerId.HasValue ? buyerId.Value.Escape() : "NULL")},
                    `updated_date` = NOW()
                WHERE `id` = {listingId.Escape()} AND `status` = 0";

            if (transaction != null)
            {
                var rowsAffected = await transaction.Connection.ExecuteAsync(sql, null, transaction);
                return rowsAffected > 0;
            }
            else
            {
                await using var conn = _dbContext.Connection(-1);
                var rowsAffected = await conn.ExecuteAsync(sql);
                return rowsAffected > 0;
            }
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
        /// <returns>A list of marketplace listings matching the criteria.</returns>
        public async Task<List<MarketplaceListing>> SearchListingsAsync(
            List<uint> itemModelIds,
            uint? minPrice,
            uint? maxPrice,
            uint? sellerId,
            string sortBy,
            int page,
            int pageSize)
        {
            await using var conn = _dbContext.Connection(-1);

            // Build WHERE conditions
            var whereConditions = new List<string> { "status = 0", "expire_date > NOW()" };
            var parameters = new DynamicParameters();

            if (itemModelIds != null && itemModelIds.Count > 0)
            {
                whereConditions.Add("item_model IN @ItemModelIds");
                parameters.Add("ItemModelIds", itemModelIds);
            }

            if (minPrice.HasValue)
            {
                whereConditions.Add("price >= @MinPrice");
                parameters.Add("MinPrice", minPrice.Value);
            }

            if (maxPrice.HasValue)
            {
                whereConditions.Add("price <= @MaxPrice");
                parameters.Add("MaxPrice", maxPrice.Value);
            }

            if (sellerId.HasValue)
            {
                whereConditions.Add("seller_id = @SellerId");
                parameters.Add("SellerId", sellerId.Value);
            }

            var whereClause = string.Join(" AND ", whereConditions);

            // Build ORDER BY clause
            var orderByClause = sortBy switch
            {
                "price_asc" => "ORDER BY price ASC",
                "price_desc" => "ORDER BY price DESC",
                "name_price_asc" => "ORDER BY item_model ASC, price ASC",
                "created_desc" => "ORDER BY created_date DESC",
                _ => "ORDER BY created_date DESC"
            };

            // Query single database with sorting
            var sql = $@"
                SELECT * FROM marketplace_listing 
                WHERE {whereClause}
                {orderByClause}
                LIMIT {pageSize} OFFSET {(page - 1) * pageSize}";

            return (await conn.QueryAsync<MarketplaceListing>(sql, parameters)).ToList();
        }

        /// <summary>
        /// Counts marketplace listings matching the specified criteria.
        /// Item model filtering is done in the WHERE clause.
        /// </summary>
        /// <param name="itemModelIds">List of item model IDs to filter by (nullable).</param>
        /// <param name="minPrice">Minimum price filter (nullable).</param>
        /// <param name="maxPrice">Maximum price filter (nullable).</param>
        /// <param name="sellerId">Seller ID filter (nullable).</param>
        /// <returns>The total count of listings matching the criteria.</returns>
        public async Task<int> CountListingsAsync(
            List<uint> itemModelIds,
            uint? minPrice,
            uint? maxPrice,
            uint? sellerId)
        {
            await using var conn = _dbContext.Connection(-1);

            // Build WHERE conditions
            var whereConditions = new List<string> { "status = 0", "expire_date > NOW()" };
            var parameters = new DynamicParameters();

            if (itemModelIds != null && itemModelIds.Count > 0)
            {
                whereConditions.Add("item_model IN @ItemModelIds");
                parameters.Add("ItemModelIds", itemModelIds);
            }

            if (minPrice.HasValue)
            {
                whereConditions.Add("price >= @MinPrice");
                parameters.Add("MinPrice", minPrice.Value);
            }

            if (maxPrice.HasValue)
            {
                whereConditions.Add("price <= @MaxPrice");
                parameters.Add("MaxPrice", maxPrice.Value);
            }

            if (sellerId.HasValue)
            {
                whereConditions.Add("seller_id = @SellerId");
                parameters.Add("SellerId", sellerId.Value);
            }

            var whereClause = string.Join(" AND ", whereConditions);

            // Query single database
            var sql = $"SELECT COUNT(*) FROM marketplace_listing WHERE {whereClause}";
            return await conn.QuerySingleAsync<int>(sql, parameters);
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

