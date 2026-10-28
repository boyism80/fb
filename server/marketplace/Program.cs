using Dapper;
using Http.Extension;
using Http.Migration;
using Http.Service;
using Http.Service.Amqp;
using Marketplace.Formatter;

namespace Marketplace;

public class Program
{
    public static async Task Main(string[] args)
    {
        Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
        SqlMapper.AddTypeHandler(typeof(List<uint>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Http.Model.Buff>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Fb.Model.Dsl>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(Dictionary<string, List<Fb.Model.Dsl>>), new JsonTypeHandler());

        var builder = WebApplication.CreateBuilder(args);
        builder.AddDatabaseMigrations();
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
        builder.Services.AddHostedService(sp => sp.GetRequiredService<RabbitMqService>());
        builder.Services.AddSingleton<LogService>();
        builder.Services.AddHostedService(sp => sp.GetRequiredService<LogService>());
        builder.Services.AddHttpAutoMapper();
        builder.Services.AddScoped<DbContext>();
        builder.Services.AddScoped<Marketplace.Service.DbContext>();
        builder.Services.AddScoped<SessionService>();
        builder.Services.AddScoped<StorageService>();
        builder.Services.AddScoped<Marketplace.Services.MarketplaceParcelService>();

        builder.Services.AddScoped<Marketplace.Services.MarketplaceService>();

        // Marketplace archive services
        builder.Services.AddHostedService<Marketplace.Services.MarketplaceArchiveBackgroundService>();

        // Marketplace expire services
        builder.Services.AddHostedService<Marketplace.Services.MarketplaceExpireBackgroundService>();
        builder.Services.AddSingleton<Marketplace.Services.MarketplaceParcelBackgroundService>();
        builder.Services.AddHostedService(sp => sp.GetRequiredService<Marketplace.Services.MarketplaceParcelBackgroundService>());
        builder.Services.AddAmqpListener<ReloadTablesHandler>();

        builder.Services.AddFbHealthChecks();

        var app = builder.Build();

        // Configure the HTTP request pipeline
        if (app.Environment.IsDevelopment())
        {
            app.UseSwagger();
            app.UseSwaggerUI();
        }

        app.UseAuthorization();
        app.MapFbHealthChecks();
        app.MapControllers();

        await app.StartAsync();
        try
        {
            await DatabaseMigrationHost.RunAsync(app.Services, MigrationProfile.Marketplace);
            var logger = app.Services.GetRequiredService<ILogger<Http.Worker.DataTableLoader>>();
            var dataTableLoader = new Http.Worker.DataTableLoader(logger);
            dataTableLoader.Run();
            app.Services.GetRequiredService<HealthCheckService>().MarkReady();
            await app.WaitForShutdownAsync();
        }
        finally
        {
            await app.StopAsync();
        }
    }
}

