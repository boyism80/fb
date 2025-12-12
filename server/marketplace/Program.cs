using Marketplace.Formatter;

namespace Marketplace;

public class Program
{
    public static void Main(string[] args)
    {
        Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;

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
        builder.Services.AddSingleton<Http.Service.RedisService>();
        builder.Services.AddSingleton<Http.Service.RedisDistributedLockService>();
        builder.Services.AddSingleton<Http.Service.RabbitMqService>();
        builder.Services.AddSingleton<Http.Service.LogService>();
        builder.Services.AddScoped<Http.Service.DbContext>();
        builder.Services.AddScoped<Http.Service.SessionService>();
        builder.Services.AddScoped<Http.Service.StorageService>();

        // Marketplace services
        builder.Services.AddScoped<Http.Reepository.MarketplaceRepository>();

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

