using Http.Service;
using StackExchange.Redis;

namespace Marketplace.Services
{
    /// <summary>
    /// Provides a background service that periodically archives marketplace listings.
    /// Uses Redis TTL with SET NX EX for atomic distributed locking to prevent concurrent execution across multiple instances.
    /// </summary>
    public class MarketplaceArchiveBackgroundService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly RedisService _redisService;
        private readonly ILogger<MarketplaceArchiveBackgroundService> _logger;

        private static readonly TimeSpan ProcessingInterval = TimeSpan.FromMinutes(1);
        private static readonly TimeSpan RetryInterval = TimeSpan.FromSeconds(10);
        private const string LockKey = "marketplace:archive:lock";

        /// <summary>
        /// Initializes a new instance of the <see cref="MarketplaceArchiveBackgroundService"/> class.
        /// </summary>
        public MarketplaceArchiveBackgroundService(
            IServiceScopeFactory scopeFactory,
            RedisService redisService,
            ILogger<MarketplaceArchiveBackgroundService> logger)
        {
            _scopeFactory = scopeFactory;
            _redisService = redisService;
            _logger = logger;
        }

        /// <summary>
        /// Executes the background service that periodically archives marketplace listings.
        /// </summary>
        /// <param name="stoppingToken">The cancellation token for stopping the service.</param>
        /// <returns>A task representing the asynchronous execution of the background service.</returns>
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    // Use Lua script to atomically check and acquire lock
                    var redis = _redisService.Redis(LockKey);
                    var acquireLockScript = @"
                        if redis.call('exists', KEYS[1]) == 0 then
                            redis.call('set', KEYS[1], '1')
                            redis.call('expire', KEYS[1], ARGV[1])
                            return 1
                        else
                            return 0
                        end";
                    var script = LuaScript.Prepare(acquireLockScript);
                    var loadedScript = script.Load(redis.GetServer());
                    var result = await redis.Connection.ScriptEvaluateAsync(
                        loadedScript.Hash,
                        new RedisKey[] { LockKey },
                        new RedisValue[] { (int)ProcessingInterval.TotalSeconds });

                    var lockAcquired = (int)result == 1;

                    if (lockAcquired)
                    {
                        try
                        {
                            // Create scope for service resolution
                            using var scope = _scopeFactory.CreateScope();
                            var archiveService = scope.ServiceProvider.GetRequiredService<MarketplaceArchiveService>();

                            // Process archive
                            await archiveService.ArchiveListingsAsync(stoppingToken);
                        }
                        catch (Exception e)
                        {
                            _logger.LogError(e, $"Error processing marketplace archive: {e.Message}");
                        }
                        // Lock will expire automatically via TTL

                        // Wait longer after successful processing
                        await Task.Delay(ProcessingInterval, stoppingToken);
                    }
                    else
                    {
                        // Wait shorter when lock acquisition fails
                        await Task.Delay(RetryInterval, stoppingToken);
                    }
                }
                catch (Exception e)
                {
                    _logger.LogError(e, $"Error in marketplace archive background service: {e.Message}");
                    
                    // Wait on error as well
                    await Task.Delay(RetryInterval, stoppingToken);
                }
            }
        }
    }
}

