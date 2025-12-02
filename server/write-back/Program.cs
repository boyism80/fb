using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Http.Service;

namespace WriteBack
{
    /// <summary>
    /// The main entry point class for the write-back service application.
    /// Configures and runs the background service that processes deferred database operations.
    /// </summary>
    internal class Program
    {
        /// <summary>
        /// The main entry point for the write-back service application.
        /// Sets up dependency injection, configures services, and starts the background processing.
        /// </summary>
        /// <param name="args">Command line arguments passed to the application.</param>
        /// <returns>A task representing the asynchronous execution of the application.</returns>
        static async Task Main(string[] args)
        {
            var builder = WebApplication.CreateBuilder(args);
            
            // Add services
            builder.Services.AddSingleton<Http.Service.RedisService>();
            builder.Services.AddSingleton<Http.Service.DbContext>();
            builder.Services.AddSingleton<Http.Service.HealthCheckService>();
            builder.Logging.AddConsole();
            builder.Services.AddHostedService<WriteBackService>();
            builder.Services.AddHostedService<Http.Service.ShutdownListenerService>();

            // Read HealthApi configuration
            var healthApiEnabled = builder.Configuration.GetValue<bool>("HealthApi:Enabled", false);
            var healthApiPort = builder.Configuration.GetValue<int>("HealthApi:Port", 80);

            if (healthApiEnabled)
            {
                // Configure Kestrel to listen on configured port
                builder.WebHost.ConfigureKestrel(options =>
                {
                    options.ListenAnyIP(healthApiPort);
                });
            }

            var app = builder.Build();

            if (healthApiEnabled)
            {
                // Health check endpoints
                app.MapGet("/health/ready", (Http.Service.HealthCheckService health) =>
                {
                    return health.IsReady ? Results.Ok("Ready") : Results.ServiceUnavailable();
                });

                app.MapGet("/health/live", (Http.Service.HealthCheckService health) =>
                {
                    return health.IsAlive ? Results.Ok("Alive") : Results.ServiceUnavailable();
                });
            }

            await app.RunAsync();
        }
    }
}
