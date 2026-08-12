using Dapper;
using Http.Extension;
using Http.Migration;
using Http.Service;
using Http.Service.Amqp;
using Http.Worker;

namespace Http;
public class Program
{
    public static async Task Main(string[] args)
    {
        Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
        SqlMapper.AddTypeHandler(typeof(List<uint>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Model.Buff>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Fb.Model.Dsl>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(Dictionary<string, List<Fb.Model.Dsl>>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(Http.Model.Mimicry), new JsonTypeHandler());

        var builder = WebApplication.CreateBuilder(args);
        builder.AddDatabaseMigrations();
        builder.Logging.ClearProviders();
        builder.Logging.AddConsole();

        // Add services to the container.

        builder.Services.AddControllers();
        builder.Services.AddMvc(opt =>
        {
            opt.InputFormatters.Insert(0, new Internal.Formatter.FlatBufferInputFormatter());
            opt.OutputFormatters.Insert(0, new Internal.Formatter.FlatBufferOutputFormatter());
        });
        // Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
        builder.Services.AddEndpointsApiExplorer();
        builder.Services.AddSwaggerGen();
        builder.Services.AddSingleton<RedisService>();
        builder.Services.AddSingleton<RedisDistributedLockService>();
        builder.Services.AddHttpAutoMapper();
        builder.Services.AddSingleton<RabbitMqService>();
        builder.Services.AddSingleton<LogService>();
        builder.Services.AddSingleton<SessionService>();
        builder.Services.AddSingleton<ServerStateService>();
        builder.Services.AddScoped<DbContext>();
        builder.Services.AddScoped<BanService>();
        builder.Services.AddScoped<CacheService>();
        builder.Services.AddSingleton<WriteBackService>();
        builder.Services.AddSingleton<Http.Service.BulletinService>();
        builder.Services.AddSingleton<Http.Service.BulletinCacheService>();
        builder.Services.AddHostedService<Http.Service.BulletinBackgroundService>();
        builder.Services.AddAmqpListener<ShutdownHandler, ReloadTablesHandler>();
        builder.Services.AddHostedService<Http.Service.SessionTtlRefreshService>();
        builder.Services.AddSingleton<Http.Service.MaintenanceService>();
        builder.Services.AddHostedService<Internal.Services.MaintenanceBackgroundService>();
        builder.Services.AddHealthChecks();
        builder.Services.AddScoped<StorageService>();
        builder.Services.AddScoped<Internal.Services.GroupService>();
        builder.Services.AddScoped<Internal.Services.FriendService>();

        var app = builder.Build();
        await DatabaseMigrationHost.RunAsync(app.Services, MigrationProfile.Internal);
        app.MapHealthChecks("/health");
        var logger = app.Services.GetRequiredService<ILogger<DataTableLoader>>();
        var dataTableLoader = new DataTableLoader(logger);
        dataTableLoader.Run();

        // Configure the HTTP request pipeline.
        if (app.Environment.IsDevelopment())
        {
            app.UseSwagger();
            app.UseSwaggerUI();
        }

        app.UseAuthorization();


        app.MapControllers();

        await app.RunAsync();
    }
}