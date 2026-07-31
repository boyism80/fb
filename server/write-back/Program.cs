using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Http.Service.Amqp;
using WriteBack.Service;

namespace WriteBack
{
    internal class Program
    {
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
                    services.AddAmqpListener<ShutdownHandler>();
                })
                .UseConsoleLifetime()
                .Build();

            await host.RunAsync();
        }
    }
}