using Http.Model;
using StackExchange.Redis;

namespace Http.Service
{
    // Article keys carry the section's renumber generation: after a renumber nobody looks up the older keys,
    // so a late cache write or a failed delete can never put an older article under a renumbered id.
    public sealed class BulletinCacheService : GenericCacheService<Bulletin>
    {
        private static readonly TimeSpan GenerationTtl = TimeSpan.FromSeconds(30);

        public BulletinCacheService(
            RedisService redisService,
            ILogger<BulletinCacheService> logger) : base(redisService, logger)
        {
        }

        protected override TimeSpan DefaultTtl => TimeSpan.FromMinutes(5);

        protected override string GenerateCacheKey(params object[] parameters)
        {
            if (parameters == null || parameters.Length < 3)
                throw new ArgumentException("Bulletin cache key requires bulletinSection, generation and id parameters", nameof(parameters));

            var bulletinSection = Convert.ToUInt32(parameters[0]);
            var generation = Convert.ToUInt32(parameters[1]);
            var id = Convert.ToUInt32(parameters[2]);

            return $"fb:cache:bulletin:{bulletinSection}:{generation}:{id}";
        }

        protected override uint? GetHashForSharding(string cacheKey, params object[] parameters)
        {
            if (parameters == null || parameters.Length < 1)
                return null;

            return Convert.ToUInt32(parameters[0]);
        }

        private static string GenerationKey(uint bulletinSection) => $"fb:cache:bulletin:{bulletinSection}:generation";

        // A miss stores the database value only if nothing is there yet, so a read racing a renumber
        // cannot overwrite the new generation; a lost renumber update lasts at most GenerationTtl.
        public async Task<uint> GetGenerationAsync(uint world, uint bulletinSection, Func<Task<uint>> dbQueryFunc)
        {
            try
            {
                var redis = RedisService.GetShardConnection(world, bulletinSection);
                if (redis == null)
                    return await dbQueryFunc();

                var cached = await redis.Connection.StringGetAsync(GenerationKey(bulletinSection));
                if (cached.HasValue && uint.TryParse(cached.ToString(), out var generation))
                    return generation;

                var dbValue = await dbQueryFunc();
                await redis.Connection.StringSetAsync(GenerationKey(bulletinSection), dbValue.ToString(), GenerationTtl, When.NotExists);
                return dbValue;
            }
            catch (Exception ex)
            {
                Logger.LogError(ex, $"Error retrieving bulletin generation for section {bulletinSection}");
                return await dbQueryFunc();
            }
        }

        public async Task SetGenerationAsync(uint world, uint bulletinSection, uint generation)
        {
            try
            {
                var redis = RedisService.GetShardConnection(world, bulletinSection);
                if (redis == null)
                    return;

                await redis.Connection.StringSetAsync(GenerationKey(bulletinSection), generation.ToString(), GenerationTtl);
            }
            catch (Exception ex)
            {
                Logger.LogError(ex, $"Error setting bulletin generation for section {bulletinSection}");
            }
        }

        public Task<Bulletin> GetArticleAsync(
            uint world,
            uint bulletinSection,
            uint generation,
            uint id,
            Func<Task<Bulletin>> dbQueryFunc,
            TimeSpan? ttl = null)
        {
            return GetAsync(world, dbQueryFunc, ttl, bulletinSection, generation, id);
        }

        public Task SetArticleAsync(
            uint world,
            uint bulletinSection,
            uint generation,
            uint id,
            Bulletin article,
            TimeSpan? ttl = null)
        {
            return SetAsync(world, article, ttl, bulletinSection, generation, id);
        }

        public Task DeleteArticleAsync(uint world, uint bulletinSection, uint generation, uint id)
        {
            return DeleteAsync(world, bulletinSection, generation, id);
        }

        public Task SetArticlesBatchAsync(
            uint world,
            uint generation,
            Dictionary<(uint bulletinSection, uint id), Bulletin> articles,
            TimeSpan? ttl = null)
        {
            if (articles == null || articles.Count == 0)
                return Task.CompletedTask;

            var items = articles.ToDictionary(
                kvp => new object[] { kvp.Key.bulletinSection, generation, kvp.Key.id },
                kvp => kvp.Value);

            return SetBatchAsync(world, items, ttl);
        }

        public Task DeleteArticlesBatchAsync(uint world, uint generation, IEnumerable<(uint bulletinSection, uint id)> articleKeys)
        {
            if (articleKeys == null)
                return Task.CompletedTask;

            var parametersList = articleKeys.Select(k => new object[] { k.bulletinSection, generation, k.id });
            return DeleteBatchAsync(world, parametersList);
        }
    }
}
