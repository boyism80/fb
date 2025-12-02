namespace Http.Service
{
    /// <summary>
    /// Provides health check status for background services.
    /// Tracks whether all data has been processed for graceful shutdown.
    /// </summary>
    public class HealthCheckService
    {
        private volatile bool _isProcessing = true;
        private volatile bool _isShuttingDown = false;

        /// <summary>
        /// Gets whether the service is currently processing data.
        /// </summary>
        public bool IsProcessing => _isProcessing;

        /// <summary>
        /// Gets whether the service is in graceful shutdown mode.
        /// </summary>
        public bool IsShuttingDown => _isShuttingDown;

        /// <summary>
        /// Sets the processing status.
        /// </summary>
        /// <param name="isProcessing">True if processing, false if idle.</param>
        public void SetProcessing(bool isProcessing)
        {
            _isProcessing = isProcessing;
        }

        /// <summary>
        /// Marks the service as entering graceful shutdown mode.
        /// </summary>
        public void BeginShutdown()
        {
            _isShuttingDown = true;
        }

        /// <summary>
        /// Gets whether the service is ready (not shutting down and processing normally).
        /// </summary>
        public bool IsReady => !_isShuttingDown;

        /// <summary>
        /// Gets whether the service is alive (always true unless critical error).
        /// </summary>
        public bool IsAlive => true;
    }
}

