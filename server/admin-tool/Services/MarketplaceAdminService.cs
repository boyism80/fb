using Dapper;
using Microsoft.Extensions.Configuration;
using MySqlConnector;

namespace AdminTool.Services
{
    public class MarketplaceAdminService
    {
        private readonly IConfiguration _configuration;
        private readonly ILogger<MarketplaceAdminService> _logger;

        public MarketplaceAdminService(IConfiguration configuration, ILogger<MarketplaceAdminService> logger)
        {
            _configuration = configuration;
            _logger = logger;
        }

        public async Task<MarketplaceListResult> GetListingsAsync(
            uint? world,
            int page,
            int pageSize,
            string sellerIdText = null,
            byte? status = null,
            string sortOrder = "newest")
        {
            await using var conn = GetUnifiedConnection();
            var where = new List<string> { "1=1" };
            var parameters = new DynamicParameters();
            parameters.Add("pageSize", pageSize);
            parameters.Add("offset", (page - 1) * pageSize);

            if (world.HasValue && world.Value > 0)
            {
                where.Add("`world` = @world");
                parameters.Add("world", world.Value);
            }

            if (status.HasValue)
            {
                where.Add("`status` = @status");
                parameters.Add("status", status.Value);
            }

            if (!string.IsNullOrWhiteSpace(sellerIdText) && uint.TryParse(sellerIdText.Trim(), out var sellerId))
            {
                where.Add("`seller_id` = @sellerId");
                parameters.Add("sellerId", sellerId);
            }

            var whereClause = string.Join(" AND ", where);

            var orderBy = sortOrder switch
            {
                "oldest" => "`created_date` ASC",
                "price_desc" => "`price` DESC",
                "price_asc" => "`price` ASC",
                _ => "`created_date` DESC"
            };

            try
            {
                var countSql = $"SELECT COUNT(*) FROM `marketplace_listing` WHERE {whereClause}";
                var totalCount = await conn.QuerySingleAsync<int>(countSql, parameters);

                var sql = $"""
                    SELECT
                        `id` AS Id,
                        `world` AS World,
                        `seller_id` AS SellerId,
                        `item_model` AS ItemModel,
                        `remaining_count` AS RemainingCount,
                        `price` AS Price,
                        `status` AS Status,
                        `expire_date` AS ExpireDate,
                        `created_date` AS CreatedDate
                    FROM `marketplace_listing`
                    WHERE {whereClause}
                    ORDER BY {orderBy}
                    LIMIT @pageSize OFFSET @offset
                    """;

                var listings = (await conn.QueryAsync<MarketplaceListingItem>(sql, parameters)).ToList();

                return new MarketplaceListResult
                {
                    Listings = listings,
                    TotalCount = totalCount,
                    Page = page,
                    PageSize = pageSize,
                    TotalPages = (int)Math.Ceiling(totalCount / (double)pageSize)
                };
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to load marketplace listings");
                throw;
            }
        }

        public async Task<MarketplaceStats> GetStatsAsync(uint? world)
        {
            await using var conn = GetUnifiedConnection();
            var where = new List<string> { "1=1" };
            var parameters = new DynamicParameters();

            if (world.HasValue && world.Value > 0)
            {
                where.Add("`world` = @world");
                parameters.Add("world", world.Value);
            }

            var whereClause = string.Join(" AND ", where);
            var todayStart = DateTime.Today;

            try
            {
                var todayParams = new DynamicParameters(parameters);
                todayParams.Add("todayStart", todayStart);

                var stats = new MarketplaceStats
                {
                    ActiveCount = await conn.QuerySingleAsync<int>(
                        $"SELECT COUNT(*) FROM `marketplace_listing` WHERE {whereClause} AND `status` = 0",
                        parameters),
                    TodayCompletedCount = await conn.QuerySingleAsync<int>(
                        $"SELECT COUNT(*) FROM `marketplace_listing` WHERE {whereClause} AND `status` = 1 AND `created_date` >= @todayStart",
                        todayParams),
                    AveragePrice = await conn.QuerySingleOrDefaultAsync<uint?>(
                        $"SELECT AVG(`price`) FROM `marketplace_listing` WHERE {whereClause} AND `status` = 0",
                        parameters) ?? 0
                };
                return stats;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to load marketplace stats");
                throw;
            }
        }

        private MySqlConnection GetUnifiedConnection()
        {
            var connectionString = _configuration.GetValue<string>("ConnectionStrings:MySql:unified");
            if (string.IsNullOrEmpty(connectionString))
                throw new InvalidOperationException("Unified connection string not found");
            return new MySqlConnection(connectionString);
        }
    }

    public class MarketplaceListingItem
    {
        public string Id { get; set; } = string.Empty;
        public uint World { get; set; }
        public uint SellerId { get; set; }
        public uint ItemModel { get; set; }
        public ushort RemainingCount { get; set; }
        public uint Price { get; set; }
        public byte Status { get; set; }
        public DateTime ExpireDate { get; set; }
        public DateTime CreatedDate { get; set; }
    }

    public class MarketplaceListResult
    {
        public List<MarketplaceListingItem> Listings { get; set; } = new();
        public int TotalCount { get; set; }
        public int Page { get; set; }
        public int PageSize { get; set; }
        public int TotalPages { get; set; }
    }

    public class MarketplaceStats
    {
        public int ActiveCount { get; set; }
        public int TodayCompletedCount { get; set; }
        public uint AveragePrice { get; set; }
    }
}
