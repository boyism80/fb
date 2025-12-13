using AutoMapper;
using Dapper;
using fb.protocol._internal;
using Http.Extension;
using Http.Service;
using Http.Worker;
using Newtonsoft.Json;
using Protocol = fb.protocol._internal;

namespace Http;
public class Program
{
    public static void Main(string[] args)
    {
        Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
        SqlMapper.AddTypeHandler(typeof(List<uint>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Model.Buff>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Fb.Model.Dsl>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(Dictionary<string, List<Fb.Model.Dsl>>), new JsonTypeHandler());

        var config = new MapperConfiguration(cfg =>
        {
            cfg.CreateMap<Http.Model.Character, Character>()
            .ForMember(x => x.ClassType, x => x.MapFrom(u => u.Class))
            .ForMember(x => x.Buffs, x => x.MapFrom(u => u.Buffs))
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")))
            .ForMember(x => x.UpdatedDate, x => x.MapFrom(u => u.UpdatedDate.ToString("yyyy-MM-dd HH:mm:ss")))
            .ForMember(x => x.Position, x => x.MapFrom(u => new Position { X = u.PositionX, Y = u.PositionY }))
            .ForMember(x => x.Role, x => x.MapFrom(u => (byte)u.Role))
            .ForMember(x => x.PendingListings, x => x.MapFrom(u => u.PendingListings == null || u.PendingListings.Count == 0 ? null : JsonConvert.SerializeObject(u.PendingListings)));

            cfg.CreateMap<Character, Http.Model.Character>()
            .ForMember(x => x.Buffs, x => x.MapFrom(u => u.Buffs))
            .ForMember(x => x.Class, x => x.MapFrom(u => u.ClassType))
            .ForMember(x => x.UpdatedDate, x => x.MapFrom(u => DateTime.Parse(u.UpdatedDate)))
            .ForMember(x => x.PositionX, x => x.MapFrom(u => u.Position.X))
            .ForMember(x => x.PositionY, x => x.MapFrom(u => u.Position.Y))
            .ForMember(x => x.Role, x => x.MapFrom(u => (Fb.Model.EnumValue.Role)u.Role))
            .ForMember(x => x.PendingListings, x => x.MapFrom(u => string.IsNullOrEmpty(u.PendingListings) ? new Dictionary<string, List<Fb.Model.Dsl>>() : (JsonConvert.DeserializeObject<Dictionary<string, List<Fb.Model.Dsl>>>(u.PendingListings) ?? new Dictionary<string, List<Fb.Model.Dsl>>())));


            cfg.CreateMap<Http.Model.Spell, Spell>()
            .ForMember(x => x.User, x => x.MapFrom(u => u.Owner))
            .ForMember(x => x.Next, x => x.MapFrom(u => u.Next.ToString("yyyy-MM-dd HH:mm:ss")))
            .ReverseMap();

            cfg.CreateMap<Http.Model.Item, Item>()
            .ForMember(x => x.User, x => x.MapFrom(u => u.Owner))
            .ReverseMap();

            cfg.CreateMap<Http.Model.Achievement, Achievement>()
            .ForMember(x => x.User, x => x.MapFrom(u => u.Uid))
            .ReverseMap();

            cfg.CreateMap<Http.Model.Option, Option>()
            .ReverseMap();

            cfg.CreateMap<Http.Model.Bulletin, ArticleSummary>()
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            cfg.CreateMap<Http.Model.Bulletin, Article>()
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            cfg.CreateMap<Http.Model.Mail, MailSummary>()
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            cfg.CreateMap<Http.Model.Mail, Mail>()
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            cfg.CreateMap<Http.Model.Clan, Clan>();

            cfg.CreateMap<Http.Model.Buff, Buff>()
            .ReverseMap();

            cfg.CreateMap<Http.Model.Quest, Quest>()
            .ForMember(x => x.Uid, x => x.MapFrom(u => u.User))
            .ForMember(x => x.Qid, x => x.MapFrom(u => u.Id))
            .ReverseMap();

            cfg.CreateMap<fb.protocol._internal.SystemMailUser, Http.Model.SystemMailUser>()
            .ForMember(x => x.User, x => x.MapFrom(u => u.User))
            .ForMember(x => x.MailId, x => x.MapFrom(u => u.MailId))
            .ForMember(x => x.Read, x => x.MapFrom(u => u.Read))
            .ForMember(x => x.ExpireDate, x => x.MapFrom(u => !string.IsNullOrEmpty(u.ExpireDate) ? (DateTime?)DateTime.Parse(u.ExpireDate) : null))
            .ForMember(x => x.Deleted, x => x.Ignore()) // Deleted is managed by Override logic
            .ForMember(x => x.CreatedDate, x => x.Ignore()) // CreatedDate is managed by DB
            .ReverseMap()
            .ForMember(x => x.ExpireDate, x => x.MapFrom(u => u.ExpireDate.HasValue ? u.ExpireDate.Value.ToString("yyyy-MM-dd HH:mm:ss") : null));

            cfg.CreateMap<Http.Model.StorageBox, Protocol.StorageBox>()
            .ForMember(x => x.Attachments, x => x.Ignore())
            .ForMember(x => x.Title, x => x.MapFrom(u => u.Title ?? string.Empty))
            .ForMember(x => x.ExpiredDate, x => x.MapFrom(u => u.ExpiredDate.HasValue ? u.ExpiredDate.Value.ToString("yyyy-MM-dd HH:mm:ss") : null))
            .AfterMap((src, dest) =>
            {
                dest.Attachments = JsonConvert.SerializeObject(src.Attachments ?? new List<Fb.Model.Dsl>());
            });

            cfg.CreateMap<Protocol.StorageBox, Http.Model.StorageBox>()
            .ForMember(x => x.Attachments, x => x.Ignore())
            .ForMember(x => x.ExpiredDate, x => x.Ignore())
            .ForMember(x => x.Title, x => x.MapFrom(u => u.Title ?? string.Empty))
            .ForMember(x => x.Message, x => x.MapFrom(u => u.Message ?? string.Empty))
            .AfterMap((src, dest) =>
            {
                dest.Attachments = string.IsNullOrWhiteSpace(src.Attachments)
                    ? new List<Fb.Model.Dsl>()
                    : (JsonConvert.DeserializeObject<List<Fb.Model.Dsl>>(src.Attachments) ?? new List<Fb.Model.Dsl>());
                dest.ExpiredDate = string.IsNullOrEmpty(src.ExpiredDate) ? null : DateTime.Parse(src.ExpiredDate);
            });

            cfg.CreateMap<Http.Model.StorageRewardMark, Protocol.StorageRewardMark>()
            .ForMember(x => x.ExpiredDate, x => x.MapFrom(u => u.ExpiredDate.HasValue ? u.ExpiredDate.Value.ToString("yyyy-MM-dd HH:mm:ss") : null));

            cfg.CreateMap<Protocol.StorageRewardMark, Http.Model.StorageRewardMark>()
            .ForMember(x => x.ExpiredDate, x => x.Ignore())
            .AfterMap((src, dest) =>
            {
                dest.ExpiredDate = string.IsNullOrEmpty(src.ExpiredDate) ? null : DateTime.Parse(src.ExpiredDate);
            });

            cfg.CreateMap<Http.Model.StoragePendingBox, Protocol.StoragePendingBox>()
            .ForMember(x => x.Attachments, x => x.Ignore())
            .ForMember(x => x.Title, x => x.MapFrom(u => u.Title ?? string.Empty))
            .ForMember(x => x.ExpiredDate, x => x.MapFrom(u => u.ExpiredDate.HasValue ? u.ExpiredDate.Value.ToString("yyyy-MM-dd HH:mm:ss") : null))
            .AfterMap((src, dest) =>
            {
                dest.Attachments = JsonConvert.SerializeObject(src.Attachments ?? new List<Fb.Model.Dsl>());
            });

            cfg.CreateMap<Protocol.StoragePendingBox, Http.Model.StoragePendingBox>()
            .ForMember(x => x.Attachments, x => x.Ignore())
            .ForMember(x => x.ExpiredDate, x => x.Ignore())
            .ForMember(x => x.Title, x => x.MapFrom(u => u.Title ?? string.Empty))
            .AfterMap((src, dest) =>
            {
                dest.Attachments = string.IsNullOrWhiteSpace(src.Attachments)
                    ? new List<Fb.Model.Dsl>()
                    : (JsonConvert.DeserializeObject<List<Fb.Model.Dsl>>(src.Attachments) ?? new List<Fb.Model.Dsl>());
                dest.ExpiredDate = string.IsNullOrEmpty(src.ExpiredDate) ? null : DateTime.Parse(src.ExpiredDate);
            });
        });

        var builder = WebApplication.CreateBuilder(args);
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
        builder.Services.AddSingleton<IMapper>(_ => new Mapper(config));
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
        builder.Services.AddHostedService<ShutdownListenerService>();
        builder.Services.AddHostedService<Http.Service.SessionTtlRefreshService>();
        builder.Services.AddHealthChecks();
        builder.Services.AddScoped<StorageService>();
        builder.Services.AddScoped<Internal.Services.GroupService>();

        var app = builder.Build();
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

        app.Run();
    }
}
