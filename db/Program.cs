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
                .ForMember(x => x.Creature, x => x.MapFrom(u => new nullopt_ushort { Value = u.Creature ?? default, Empty = !u.Creature.HasValue }))
                .ForMember(x => x.Position, x => x.MapFrom(u => new Position { X = u.PositionX, Y = u.PositionY }))
                .ForMember(x => x.Disguise, x => x.MapFrom(u => new nullopt_ushort { Value = u.Disguise ?? default, Empty = !u.Disguise.HasValue }))
                .ForMember(x => x.WeaponColor, x => x.MapFrom(u => new nullopt_ubyte { Value = u.WeaponColor ?? default, Empty = !u.WeaponColor.HasValue }))
                .ForMember(x => x.HelmetColor, x => x.MapFrom(u => new nullopt_ubyte { Value = u.HelmetColor ?? default, Empty = !u.HelmetColor.HasValue }))
                .ForMember(x => x.ArmorColor, x => x.MapFrom(u => new nullopt_ubyte { Value = u.ArmorColor ?? default, Empty = !u.ArmorColor.HasValue }))
                .ForMember(x => x.ShieldColor, x => x.MapFrom(u => new nullopt_ubyte { Value = u.ShieldColor ?? default, Empty = !u.ShieldColor.HasValue }))
                .ForMember(x => x.RingLeftColor, x => x.MapFrom(u => new nullopt_ubyte { Value = u.RingLeftColor ?? default, Empty = !u.RingLeftColor.HasValue }))
                .ForMember(x => x.RingRightColor, x => x.MapFrom(u => new nullopt_ubyte { Value = u.RingRightColor ?? default, Empty = !u.RingRightColor.HasValue }))
                .ForMember(x => x.AuxTopColor, x => x.MapFrom(u => new nullopt_ubyte { Value = u.AuxTopColor ?? default, Empty = !u.AuxTopColor.HasValue }))
                .ForMember(x => x.AuxBotColor, x => x.MapFrom(u => new nullopt_ubyte { Value = u.AuxBotColor ?? default, Empty = !u.AuxBotColor.HasValue }))
                .ForMember(x => x.Clan, x => x.MapFrom(u => new nullopt_uint { Value = u.Clan ?? default, Empty = !u.Clan.HasValue }));

                cfg.CreateMap<fb.protocol.db.Character, Db.Model.Character>()
                .ForMember(x => x.LastLogin, x => x.MapFrom(u => DateTime.Parse(u.LastLogin)))
                .ForMember(x => x.Creature, x => x.MapFrom(u => u.Creature.Empty ? null : (ushort?)u.Creature.Value))
                .ForMember(x => x.PositionX, x => x.MapFrom(u => u.Position.X))
                .ForMember(x => x.PositionY, x => x.MapFrom(u => u.Position.Y))
                .ForMember(x => x.Disguise, x => x.MapFrom(u => u.Disguise.Empty ? null : (ushort?)u.Disguise.Value))
                .ForMember(x => x.WeaponColor, x => x.MapFrom(u => u.WeaponColor.Empty ? null : (byte?)u.WeaponColor.Value))
                .ForMember(x => x.HelmetColor, x => x.MapFrom(u => u.HelmetColor.Empty ? null : (byte?)u.HelmetColor.Value))
                .ForMember(x => x.ArmorColor, x => x.MapFrom(u => u.ArmorColor.Empty ? null : (byte?)u.ArmorColor.Value))
                .ForMember(x => x.ShieldColor, x => x.MapFrom(u => u.ShieldColor.Empty ? null : (byte?)u.ShieldColor.Value))
                .ForMember(x => x.RingLeftColor, x => x.MapFrom(u => u.RingLeftColor.Empty ? null : (byte?)u.RingLeftColor.Value))
                .ForMember(x => x.RingRightColor, x => x.MapFrom(u => u.RingRightColor.Empty ? null : (byte?)u.RingRightColor.Value))
                .ForMember(x => x.AuxTopColor, x => x.MapFrom(u => u.AuxTopColor.Empty ? null : (byte?)u.AuxTopColor.Value))
                .ForMember(x => x.AuxBotColor, x => x.MapFrom(u => u.AuxBotColor.Empty ? null : (byte?)u.AuxBotColor.Value))
                .ForMember(x => x.Clan, x => x.MapFrom(u => u.Clan.Empty ? null : (uint?)u.Clan.Value));


                cfg.CreateMap<Db.Model.Spell, fb.protocol.db.Spell>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.Owner));

                cfg.CreateMap<Db.Model.Item, fb.protocol.db.Item>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.Owner))
                .ForMember(x => x.Durability, x => x.MapFrom(u => new nullopt_uint { Value = u.Durability ?? default, Empty = !u.Durability.HasValue }));
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