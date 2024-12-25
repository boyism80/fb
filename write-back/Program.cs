using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using WriteBack.Service;

namespace WriteBack
{
    internal class Program
    {
        static async Task Main(string[] args)
        {
            var configuration = new ConfigurationBuilder()
                .SetBasePath(Directory.GetCurrentDirectory())
                .AddJsonFile("appsettings.json", false)
                .Build() as IConfiguration;


            var collection = new ServiceCollection();
            collection.AddSingleton<Http.Service.RedisService>();
            collection.AddSingleton(configuration);
            collection.AddSingleton<Http.Service.DbContext>();
            collection.AddSingleton<WriteBackService>();
            collection.AddLogging(builder =>
            {
                builder.AddConsole();
            });

            var sp = collection.BuildServiceProvider();
            var wbs = sp.GetService<WriteBackService>();

            var stoppingToken = new CancellationToken();
            await wbs.ExecuteAsync(stoppingToken);
        }
    }
}
