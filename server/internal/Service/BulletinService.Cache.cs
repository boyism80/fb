using System;
using System.Collections.Generic;
using System.Linq;
using Http.Model;
using Http.Service;

namespace Internal.Service
{
    public sealed class BulletinCacheService : GenericCacheService<Bulletin>
    {
        public BulletinCacheService(
            RedisService redisService,
            ILogger<BulletinCacheService> logger) : base(redisService, logger)
        {
        }

        protected override TimeSpan DefaultTtl => TimeSpan.FromHours(1);

        protected override string GenerateCacheKey(params object[] parameters)
        {
            if (parameters == null || parameters.Length < 2)
                throw new ArgumentException("Bulletin cache key requires section and id parameters", nameof(parameters));

            var section = Convert.ToUInt32(parameters[0]);
            var id = Convert.ToUInt32(parameters[1]);

            return $"cache:bulletin:{section}:{id}";
        }

        protected override uint? GetHashForSharding(string cacheKey, params object[] parameters)
        {
            if (parameters == null || parameters.Length < 1)
                return null;

            return Convert.ToUInt32(parameters[0]);
        }

        public Task<Bulletin> GetArticleAsync(
            uint section,
            uint id,
            Func<Task<Bulletin>> dbQueryFunc,
            TimeSpan? ttl = null)
        {
            return GetAsync(dbQueryFunc, ttl, section, id);
        }

        public Task SetArticleAsync(
            uint section,
            uint id,
            Bulletin article,
            TimeSpan? ttl = null)
        {
            return SetAsync(article, ttl, section, id);
        }

        public Task DeleteArticleAsync(uint section, uint id)
        {
            return DeleteAsync(section, id);
        }

        public Task SetArticlesBatchAsync(
            Dictionary<(uint section, uint id), Bulletin> articles,
            TimeSpan? ttl = null)
        {
            if (articles == null || articles.Count == 0)
                return Task.CompletedTask;

            var items = articles.ToDictionary(
                kvp => new object[] { kvp.Key.section, kvp.Key.id },
                kvp => kvp.Value);

            return SetBatchAsync(items, ttl);
        }

        public Task DeleteArticlesBatchAsync(IEnumerable<(uint section, uint id)> articleKeys)
        {
            if (articleKeys == null)
                return Task.CompletedTask;

            var parametersList = articleKeys.Select(k => new object[] { k.section, k.id });
            return DeleteBatchAsync(parametersList);
        }
    }
}

