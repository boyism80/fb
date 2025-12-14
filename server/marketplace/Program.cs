using Dapper;
using Http.Extension;
using Http.Service;
using Marketplace.Formatter;

namespace Marketplace;

public class Program
{
    public static void Main(string[] args)
    {
        Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
        SqlMapper.AddTypeHandler(typeof(List<uint>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Http.Model.Buff>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Fb.Model.Dsl>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(Dictionary<string, List<Fb.Model.Dsl>>), new JsonTypeHandler());

        var builder = WebApplication.CreateBuilder(args);
        builder.Logging.ClearProviders();
        builder.Logging.AddConsole();

        // Add services to the container
        builder.Services.AddControllers();
        builder.Services.AddMvc(opt =>
        {
            opt.InputFormatters.Insert(0, new FlatBufferInputFormatter());
            opt.OutputFormatters.Insert(0, new FlatBufferOutputFormatter());
        });

        builder.Services.AddEndpointsApiExplorer();
        builder.Services.AddSwaggerGen();

        // Add custom services from Http
        builder.Services.AddSingleton<WriteBackService>();
        builder.Services.AddSingleton<RedisService>();
        builder.Services.AddSingleton<RedisDistributedLockService>();
        builder.Services.AddSingleton<RabbitMqService>();
        builder.Services.AddSingleton<LogService>();
        builder.Services.AddScoped<DbContext>();
        builder.Services.AddScoped<SessionService>();
        builder.Services.AddScoped<StorageService>();

        // Marketplace services
        builder.Services.AddScoped<Http.Reepository.MarketplaceRepository>();
        builder.Services.AddScoped<Http.Reepository.MarketplacePurchaseRepository>();

        // Register marketplace service implementation based on configuration
        var useSharding = builder.Configuration.GetValue<bool>("Marketplace:UseSharding", false);
        if (useSharding)
        {
            builder.Services.AddScoped<Marketplace.Services.IMarketplaceService, Marketplace.Services.ShardedMarketplaceService>();
        }
        else
        {
            builder.Services.AddScoped<Marketplace.Services.IMarketplaceService, Marketplace.Services.MarketplaceService>();
        }

        builder.Services.AddHealthChecks();

        var app = builder.Build();
        app.MapHealthChecks("/health");

        // Load data tables
        var logger = app.Services.GetRequiredService<ILogger<Http.Worker.DataTableLoader>>();
        var dataTableLoader = new Http.Worker.DataTableLoader(logger);
        dataTableLoader.Run();

        // Configure the HTTP request pipeline
        if (app.Environment.IsDevelopment())
        {
            app.UseSwagger();
            app.UseSwaggerUI();
        }

        app.UseAuthorization();
        app.MapControllers();

        app.Run();
    }
}

