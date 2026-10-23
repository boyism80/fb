using Microsoft.Extensions.Diagnostics.HealthChecks;

namespace Http.Service
{
    public sealed class AliveHealthCheck : IHealthCheck
    {
        private readonly HealthCheckService _health;

        public AliveHealthCheck(HealthCheckService health)
        {
            _health = health;
        }

        public Task<HealthCheckResult> CheckHealthAsync(HealthCheckContext context, CancellationToken cancellationToken = default)
        {
            if (_health.IsAlive)
            {
                return Task.FromResult(HealthCheckResult.Healthy());
            }

            return Task.FromResult(HealthCheckResult.Unhealthy("process is not alive"));
        }
    }
}
