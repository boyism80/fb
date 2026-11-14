namespace Http.Service
{
    public class HealthCheckService
    {
        private readonly TaskCompletionSource _ready = new(TaskCreationOptions.RunContinuationsAsynchronously);
        private volatile bool _shuttingDown;

        public bool IsAlive => true;

        public bool IsReady => _ready.Task.IsCompleted && !_shuttingDown;

        public void MarkReady()
        {
            _ready.TrySetResult();
        }

        public Task WhenReady(CancellationToken cancellationToken)
        {
            return _ready.Task.WaitAsync(cancellationToken);
        }

        public void BeginShutdown()
        {
            _shuttingDown = true;
        }
    }
}
