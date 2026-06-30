using Http.Migration;
using Http.Service;
using Log.Repository;
using Log.Worker;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

namespace Log
{
    internal class Program
    {
        static async Task Main(string[] args)
        {
            Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
            var host = Host.CreateDefaultBuilder(args)
                .ConfigureServices((context, services) =>
                {
                    services.AddDatabaseMigrations(context.Configuration);
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

            await DatabaseMigrationHost.RunAsync(host.Services, MigrationProfile.Log);
            await host.RunAsync();
        }
    }
}