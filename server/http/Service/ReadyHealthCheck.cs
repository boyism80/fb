using Microsoft.Extensions.Diagnostics.HealthChecks;

namespace Http.Service
{
    public sealed class ReadyHealthCheck : IHealthCheck
    {
        private readonly HealthCheckService _health;

        public ReadyHealthCheck(HealthCheckService health)
        {
            _health = health;
        }

        public Task<HealthCheckResult> CheckHealthAsync(HealthCheckContext context, CancellationToken cancellationToken = default)
        {
            if (_health.IsReady)
            {
                return Task.FromResult(HealthCheckResult.Healthy());
            }

            return Task.FromResult(HealthCheckResult.Unhealthy("startup incomplete"));
        }
    }
}
