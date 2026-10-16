namespace Http.Service
{
    public class HealthCheckService
    {
        private volatile bool _ready;
        private volatile bool _shuttingDown;

        public bool IsAlive => true;

        public bool IsReady => _ready && !_shuttingDown;

        public void MarkReady()
        {
            _ready = true;
        }

        public void BeginShutdown()
        {
            _shuttingDown = true;
        }
    }
}
