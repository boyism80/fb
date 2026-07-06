using AutoMapper;
using Http.Service;
using Http.Worker;
using Matchmaking.Core;
using Matchmaking.Formatter;
using Matchmaking.Mapping;
using Matchmaking.Model;
using Matchmaking.Options;
using Matchmaking.Services;
using Microsoft.Extensions.Logging.Abstractions;

namespace Matchmaking;

public class Program
{
    public static async Task Main(string[] args)
    {
        var builder = WebApplication.CreateBuilder(args);
        builder.Logging.ClearProviders();
        builder.Logging.AddConsole();

        builder.Services.Configure<MatchmakingOptions>(builder.Configuration.GetSection("Matchmaking"));
        builder.Services.AddControllers();
        builder.Services.AddMvc(opt =>
        {
            opt.InputFormatters.Insert(0, new FlatBufferInputFormatter());
            opt.OutputFormatters.Insert(0, new FlatBufferOutputFormatter());
        });

        builder.Services.AddSingleton<IMapper>(_ =>
        {
            var config = new MapperConfiguration(
                cfg => cfg.AddProfile<MatchmakingMappingProfile>(),
                NullLoggerFactory.Instance);
            return new Mapper(config);
        });
        builder.Services.AddSingleton<RabbitMqService>();
        builder.Services.AddSingleton<CharacterMatchMaker>();
        builder.Services.AddSingleton<MatchMaker<CharacterRegistryEntry>>(sp => sp.GetRequiredService<CharacterMatchMaker>());
        builder.Services.AddHostedService<MatchmakingBackgroundService>();
        builder.Services.AddHostedService<MatchConfirmationBackgroundService>();
        builder.Services.AddHealthChecks();

        var app = builder.Build();

        var logger = app.Services.GetRequiredService<ILogger<DataTableLoader>>();
        var dataTableLoader = new DataTableLoader(logger);
        dataTableLoader.Run();

        app.MapHealthChecks("/health");
        app.MapControllers();

        await app.RunAsync();
    }
}
