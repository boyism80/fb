namespace Http.Service
{
    public class HealthCheckService
    {
        private volatile bool _isProcessing = true;
        private volatile bool _isShuttingDown = false;

        public bool IsProcessing => _isProcessing;

        public bool IsShuttingDown => _isShuttingDown;

        public void SetProcessing(bool isProcessing)
        {
            _isProcessing = isProcessing;
        }

        public void BeginShutdown()
        {
            _isShuttingDown = true;
        }

        public bool IsReady => !_isShuttingDown;

        public bool IsAlive => true;
    }
}
