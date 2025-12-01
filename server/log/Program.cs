using Dapper;
using Http.Service;
using Log.Repository;
using Log.Worker;
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
            var host = Host.CreateDefaultBuilder(args)
                .ConfigureServices(services =>
                {
                    services.AddSingleton<DbContext>();
                    services.AddScoped<LogRepository>();
                    services.AddLogging(builder =>
                    {
                        builder.ClearProviders();
                        builder.AddConsole();
                    });
                    services.AddHostedService<LogConsumerService>();
                })
                .UseConsoleLifetime()
                .Build();

            await host.RunAsync();
        }
    }
}
