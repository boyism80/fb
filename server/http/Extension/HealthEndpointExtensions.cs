using Http.Service;
using Microsoft.AspNetCore.Diagnostics.HealthChecks;
using Microsoft.Extensions.Diagnostics.HealthChecks;

namespace Http.Extension
{
    public static class HealthEndpointExtensions
    {
        public static IServiceCollection AddFbHealthChecks(this IServiceCollection services)
        {
            services.AddSingleton<HealthCheckService>();
            services.AddHealthChecks()
                .AddCheck<AliveHealthCheck>("alive", tags: new[] { "live" })
                .AddCheck<ReadyHealthCheck>("ready", tags: new[] { "ready" });
            return services;
        }

        public static WebApplication MapFbHealthChecks(this WebApplication app)
        {
            app.MapHealthChecks("/health/live", new HealthCheckOptions
            {
                Predicate = registration => registration.Tags.Contains("live")
            });
            app.MapHealthChecks("/health/ready", new HealthCheckOptions
            {
                Predicate = registration => registration.Tags.Contains("ready")
            });
            app.MapHealthChecks("/health", new HealthCheckOptions
            {
                Predicate = registration => registration.Tags.Contains("ready")
            });
            return app;
        }
    }
}
