using AutoMapper;
using Dapper;
using fb.protocol._internal;
using Http.Extension;
using Http.Service;
using Http.Worker;

namespace Http;

public class Program
{
    public static void Main(string[] args)
    {
        Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
        SqlMapper.AddTypeHandler(typeof(List<uint>), new JsonTypeHandler());
        SqlMapper.AddTypeHandler(typeof(List<Model.Buff>), new JsonTypeHandler());

        var config = new MapperConfiguration(cfg =>
        {
            cfg.CreateMap<Http.Model.Character, Character>()
            .ForMember(x => x.ClassType, x => x.MapFrom(u => u.Class))
            .ForMember(x => x.Buffs, x => x.MapFrom(u => u.Buffs))
            .ForMember(x => x.UpdatedDate, x => x.MapFrom(u => u.UpdatedDate.ToString("yyyy-MM-dd HH:mm:ss")))
            .ForMember(x => x.Position, x => x.MapFrom(u => new Position { X = u.PositionX, Y = u.PositionY }));

            cfg.CreateMap<Character, Http.Model.Character>()
            .ForMember(x => x.Buffs, x => x.MapFrom(u => u.Buffs))
            .ForMember(x => x.Class, x => x.MapFrom(u => u.ClassType))
            .ForMember(x => x.UpdatedDate, x => x.MapFrom(u => DateTime.Parse(u.UpdatedDate)))
            .ForMember(x => x.PositionX, x => x.MapFrom(u => u.Position.X))
            .ForMember(x => x.PositionY, x => x.MapFrom(u => u.Position.Y));


            cfg.CreateMap<Http.Model.Spell, Spell>()
            .ForMember(x => x.User, x => x.MapFrom(u => u.Owner))
            .ForMember(x => x.Next, x => x.MapFrom(u => u.Next.ToString("yyyy-MM-dd HH:mm:ss")))
            .ReverseMap();

            cfg.CreateMap<Http.Model.Item, Item>()
            .ForMember(x => x.User, x => x.MapFrom(u => u.Owner))
            .ReverseMap();

            cfg.CreateMap<Http.Model.Trace, Trace>()
            .ForMember(x => x.User, x => x.MapFrom(u => u.Uid))
            .ReverseMap();

            cfg.CreateMap<Http.Model.Option, Option>()
            .ReverseMap();

            cfg.CreateMap<Http.Model.Board, ArticleSummary>()
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            cfg.CreateMap<Http.Model.Board, Article>()
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            cfg.CreateMap<Http.Model.Mail, MailSummary>()
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            cfg.CreateMap<Http.Model.Mail, Mail>()
            .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            cfg.CreateMap<Http.Model.Clan, Clan>();

            cfg.CreateMap<Http.Model.Buff, Buff>()
            .ReverseMap();
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
        builder.Services.AddSingleton<Fb.Model.Model>();
        builder.Services.AddSingleton<IMapper>(_ => new Mapper(config));
        builder.Services.AddSingleton<RabbitMqService>();
        builder.Services.AddSingleton<SessionService>();
        builder.Services.AddScoped<DbContext>();
        builder.Services.AddSingleton<WriteBackService>();

        var app = builder.Build();
        var dataTableLoader = ActivatorUtilities.CreateInstance(app.Services.CreateScope().ServiceProvider, typeof(DataTableLoader)) as DataTableLoader;
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
