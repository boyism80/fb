using Http.Service;
using Microsoft.AspNetCore.Mvc;

namespace Internal.Controllers
{
    [ApiController]
    [Route("cache")]
    public class CacheController : ControllerBase
    {
        private readonly CacheService _cacheService;
        public CacheController(CacheService cacheService)
        {
            _cacheService = cacheService;
        }
        [HttpPost("clear")]
        public async Task<int> Clear()
        {
            return await _cacheService.ClearCache();
        }
    }
}