using AutoMapper;
using Db.Formatter;
using fb.protocol.db;
using http.Service;
using http.Worker;

namespace db
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;

            var config = new MapperConfiguration(cfg => 
            {
                cfg.CreateMap<Db.Model.Character, fb.protocol.db.Character>()
                .ForMember(x => x.LastLogin, x => x.MapFrom(u => u.LastLogin.ToString("yyyy-MM-dd HH:mm:ss")))
                .ForMember(x => x.Position, x => x.MapFrom(u => new Position { X = u.PositionX, Y = u.PositionY }));

                cfg.CreateMap<fb.protocol.db.Character, Db.Model.Character>()
                .ForMember(x => x.LastLogin, x => x.MapFrom(u => DateTime.Parse(u.LastLogin)))
                .ForMember(x => x.PositionX, x => x.MapFrom(u => u.Position.X))
                .ForMember(x => x.PositionY, x => x.MapFrom(u => u.Position.Y));


                cfg.CreateMap<Db.Model.Spell, fb.protocol.db.Spell>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.Owner));

                cfg.CreateMap<Db.Model.Item, fb.protocol.db.Item>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.Owner));
            });

            var builder = WebApplication.CreateBuilder(args);

            // Add services to the container.

            builder.Services.AddControllers();
            builder.Services.AddMvc(opt =>
            {
                opt.InputFormatters.Insert(0, new FlatBufferInputFormatter());
                opt.OutputFormatters.Insert(0, new FlatBufferOutputFormatter());
            });
            builder.Services.AddSingleton<RedisService>();
            builder.Services.AddSingleton<Fb.Model.Model>();
            builder.Services.AddSingleton<IMapper>(_ => new Mapper(config));

            var app = builder.Build();
            var dataTableLoader = ActivatorUtilities.CreateInstance(app.Services.CreateScope().ServiceProvider, typeof(DataTableLoader)) as DataTableLoader;
            dataTableLoader.Run();

            // Configure the HTTP request pipeline.

            app.UseAuthorization();


            app.MapControllers();

            app.Run();
        }
    }
}