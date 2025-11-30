using System.Text.Json;
using Log.Service;
using Microsoft.AspNetCore.Mvc;

namespace Log.Controllers
{
    /// <summary>
    /// Controller for handling log ingestion requests.
    /// Accepts compressed binary log data and stores it in Redis with sharding.
    /// </summary>
    [ApiController]
    [Route("log")]
    public class LogController : ControllerBase
    {
        private readonly ILogger<LogController> _logger;
        private readonly LogService _logService;

        /// <summary>
        /// Initializes a new instance of the <see cref="LogController"/> class.
        /// </summary>
        /// <param name="logger">The logger instance.</param>
        /// <param name="logService">The log service for storing logs.</param>
        public LogController(ILogger<LogController> logger, LogService logService)
        {
            _logger = logger;
            _logService = logService;
        }

        /// <summary>
        /// Ingests log data from compressed binary format.
        /// Accepts gzip-compressed JSON array and stores each log entry in Redis with sharding.
        /// </summary>
        /// <param name="logs">The array of JSON log entries.</param>
        /// <returns>An HTTP 200 OK response if successful.</returns>
        [HttpPost("ingest")]
        public async Task<IActionResult> Ingest([FromBody] JsonElement[] logs)
        {
            try
            {
                if (logs == null || logs.Length == 0)
                {
                    return BadRequest("No log entries provided");
                }

                await _logService.StoreLogsAsync(logs);

                return Ok(new { count = logs.Length });
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to ingest logs");
                return StatusCode(500, "Internal server error");
            }
        }
    }
}

