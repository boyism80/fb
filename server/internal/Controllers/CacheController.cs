using Http.Service;
using Microsoft.AspNetCore.Mvc;

namespace Internal.Controllers
{
    /// <summary>
    /// Provides cache management operations for the internal API.
    /// Handles Redis cache clearing and maintenance tasks.
    /// </summary>
    [ApiController]
    [Route("cache")]
    public class CacheController : ControllerBase
    {
        private readonly CacheService _cacheService;

        /// <summary>
        /// Initializes a new instance of the <see cref="CacheController"/> class.
        /// </summary>
        /// <param name="cacheService">The cache service for cache operations.</param>
        public CacheController(CacheService cacheService)
        {
            _cacheService = cacheService;
        }

        /// <summary>
        /// Clears all cache data across all Redis shards.
        /// Removes cache entries, reference counts, and buffer keys from all shards.
        /// </summary>
        /// <returns>A task representing the asynchronous cache clearing operation.</returns>
        [HttpPost("clear")]
        public async Task<int> Clear()
        {
            return await _cacheService.ClearCache();
        }
    }
}