using Dapper;
using Http.Service;
using Log.Repository;
using Log.Worker;
using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

namespace Log
{
    /// <summary>
    /// The main entry point class for the log service application.
    /// Configures and runs the background service that consumes logs from RabbitMQ and stores them in the database.
    /// </summary>
    internal class Program
    {
        /// <summary>
        /// The main entry point for the log service application.
        /// Sets up dependency injection, configures services, and starts the background processing.
        /// </summary>
        /// <param name="args">Command line arguments passed to the application.</param>
        /// <returns>A task representing the asynchronous execution of the application.</returns>
        static async Task Main(string[] args)
        {
            Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;

            var builder = WebApplication.CreateBuilder(args);

            // Add services
            builder.Services.AddSingleton<DbContext>();
            builder.Services.AddSingleton<Http.Service.HealthCheckService>();
            builder.Services.AddScoped<LogRepository>();
            builder.Logging.ClearProviders();
            builder.Logging.AddConsole();
            builder.Services.AddHostedService<LogConsumerService>();

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
