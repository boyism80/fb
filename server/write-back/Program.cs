using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using WriteBack.Service;

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
            var host = Host.CreateDefaultBuilder(args)
                .ConfigureServices(services =>
                {
                    services.AddSingleton<Http.Service.RedisService>();
                    services.AddSingleton<Http.Service.DbContext>();
                    services.AddLogging(builder =>
                    {
                        builder.AddConsole();
                    });
                    services.AddHostedService<WriteBackService>();
                    services.AddHostedService<Http.Service.ShutdownListenerService>();
                })
                .UseConsoleLifetime()
                .Build();

            await host.RunAsync();
        }
    }
}
