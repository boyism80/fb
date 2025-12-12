using Dapper;
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
            var sql = @"
                SELECT * FROM marketplace_listing 
                WHERE id = @ListingId AND status != 3";

            return await conn.QueryFirstOrDefaultAsync<MarketplaceListing>(sql, new { ListingId = listingId });
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
        /// <param name="listingFee">The fee paid when listing (refundable on cancel).</param>
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
            uint listingFee,
            uint transactionFee,
            DateTime expireDate)
        {
            await using var conn = _dbContext.Connection(-1);
            var sql = @"
                INSERT INTO marketplace_listing 
                (id, seller_id, item_model, item_count, item_durability, item_custom_name, 
                 price, listing_fee, transaction_fee, status, expire_date)
                VALUES 
                (@ListingId, @SellerId, @ItemModel, @ItemCount, @ItemDurability, @ItemCustomName,
                 @Price, @ListingFee, @TransactionFee, 0, @ExpireDate);";

            await conn.ExecuteAsync(sql, new
            {
                ListingId = listingId,
                SellerId = sellerId,
                ItemModel = itemModel,
                ItemCount = itemCount,
                ItemDurability = itemDurability,
                ItemCustomName = itemCustomName,
                Price = price,
                ListingFee = listingFee,
                TransactionFee = transactionFee,
                ExpireDate = expireDate
            });
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
            var sql = @"
                UPDATE marketplace_listing 
                SET status = @Status,
                    updated_date = NOW()
                WHERE id = @Id";

            var rowsAffected = await conn.ExecuteAsync(sql, new { Id = listingId, Status = status });
            return rowsAffected > 0;
        }

        /// <summary>
        /// Updates a marketplace listing with sale information.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="status">The new status (typically 1 for Sold).</param>
        /// <param name="soldDate">The date and time when the item was sold (nullable).</param>
        /// <param name="buyerId">The unique identifier of the buyer (nullable).</param>
        /// <returns>True if the update was successful; otherwise, false.</returns>
        public async Task<bool> UpdateListingAsync(string listingId, byte status, DateTime? soldDate, uint? buyerId)
        {
            await using var conn = _dbContext.Connection(-1);
            var sql = @"
                UPDATE marketplace_listing 
                SET status = @Status,
                    sold_date = @SoldDate,
                    buyer_id = @BuyerId,
                    updated_date = NOW()
                WHERE id = @Id";

            var rowsAffected = await conn.ExecuteAsync(sql, new
            {
                Id = listingId,
                Status = status,
                SoldDate = soldDate,
                BuyerId = buyerId
            });
            return rowsAffected > 0;
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

            // Query single database
            var sql = $@"
                SELECT * FROM marketplace_listing 
                WHERE {whereClause}";

            var allResults = (await conn.QueryAsync<MarketplaceListing>(sql, parameters)).ToList();

            // Sort results in memory
            switch (sortBy)
            {
                case "price_asc":
                    allResults = allResults.OrderBy(x => x.Price).ToList();
                    break;
                case "price_desc":
                    allResults = allResults.OrderByDescending(x => x.Price).ToList();
                    break;
                case "created_desc":
                default:
                    allResults = allResults.OrderByDescending(x => x.CreatedDate).ToList();
                    break;
            }

            // Apply pagination
            var offset = (page - 1) * pageSize;
            return allResults.Skip(offset).Take(pageSize).ToList();
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
        /// Creates a new marketplace transaction record.
        /// </summary>
        /// <param name="listingId">The unique identifier of the listing (UUID string).</param>
        /// <param name="sellerId">The unique identifier of the seller.</param>
        /// <param name="buyerId">The unique identifier of the buyer.</param>
        /// <param name="itemModel">The item model identifier.</param>
        /// <param name="itemCount">The number of items sold.</param>
        /// <param name="price">The transaction price.</param>
        /// <param name="listingFee">The listing fee paid.</param>
        /// <param name="transactionFee">The transaction fee deducted.</param>
        /// <param name="sellerRevenue">The revenue received by the seller after fees.</param>
        /// <returns>The unique identifier of the created transaction.</returns>
        public async Task<ulong> CreateTransactionAsync(
            string listingId,
            uint sellerId,
            uint buyerId,
            uint itemModel,
            ushort itemCount,
            uint price,
            uint listingFee,
            uint transactionFee,
            uint sellerRevenue)
        {
            await using var conn = _dbContext.Connection(-1);
            var sql = @"
                INSERT INTO marketplace_transaction 
                (listing_id, seller_id, buyer_id, item_model, item_count, price, 
                 listing_fee, transaction_fee, seller_revenue)
                VALUES 
                (@ListingId, @SellerId, @BuyerId, @ItemModel, @ItemCount, @Price,
                 @ListingFee, @TransactionFee, @SellerRevenue);
                SELECT LAST_INSERT_ID();";

            var id = await conn.QuerySingleAsync<ulong>(sql, new
            {
                ListingId = listingId,
                SellerId = sellerId,
                BuyerId = buyerId,
                ItemModel = itemModel,
                ItemCount = itemCount,
                Price = price,
                ListingFee = listingFee,
                TransactionFee = transactionFee,
                SellerRevenue = sellerRevenue
            });
            return id;
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

