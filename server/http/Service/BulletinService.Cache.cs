using Http.Model;

namespace Http.Service
{
    public sealed class BulletinCacheService : GenericCacheService<Bulletin>
    {
        public BulletinCacheService(
            RedisService redisService,
            ILogger<BulletinCacheService> logger) : base(redisService, logger)
        {
        }

        protected override TimeSpan DefaultTtl => TimeSpan.FromMinutes(5);

        protected override string GenerateCacheKey(params object[] parameters)
        {
            if (parameters == null || parameters.Length < 2)
                throw new ArgumentException("Bulletin cache key requires bulletinSection and id parameters", nameof(parameters));

            var bulletinSection = Convert.ToUInt32(parameters[0]);
            var id = Convert.ToUInt32(parameters[1]);

            return $"cache:bulletin:{bulletinSection}:{id}";
        }

        protected override uint? GetHashForSharding(string cacheKey, params object[] parameters)
        {
            if (parameters == null || parameters.Length < 1)
                return null;

            return Convert.ToUInt32(parameters[0]);
        }

        public Task<Bulletin> GetArticleAsync(
            uint world,
            uint bulletinSection,
            uint id,
            Func<Task<Bulletin>> dbQueryFunc,
            TimeSpan? ttl = null)
        {
            return GetAsync(world, dbQueryFunc, ttl, bulletinSection, id);
        }

        public Task SetArticleAsync(
            uint world,
            uint bulletinSection,
            uint id,
            Bulletin article,
            TimeSpan? ttl = null)
        {
            return SetAsync(world, article, ttl, bulletinSection, id);
        }

        public Task DeleteArticleAsync(uint world, uint bulletinSection, uint id)
        {
            return DeleteAsync(world, bulletinSection, id);
        }

        public Task SetArticlesBatchAsync(
            uint world,
            Dictionary<(uint bulletinSection, uint id), Bulletin> articles,
            TimeSpan? ttl = null)
        {
            if (articles == null || articles.Count == 0)
                return Task.CompletedTask;

            var items = articles.ToDictionary(
                kvp => new object[] { kvp.Key.bulletinSection, kvp.Key.id },
                kvp => kvp.Value);

            return SetBatchAsync(world, items, ttl);
        }

        public Task DeleteArticlesBatchAsync(uint world, IEnumerable<(uint bulletinSection, uint id)> articleKeys)
        {
            if (articleKeys == null)
                return Task.CompletedTask;

            var parametersList = articleKeys.Select(k => new object[] { k.bulletinSection, k.id });
            return DeleteBatchAsync(world, parametersList);
        }
    }
}

