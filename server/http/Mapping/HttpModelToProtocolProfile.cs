using AutoMapper;
using Newtonsoft.Json;
using Protocol = fb.protocol._internal;

namespace Http.Mapping
{
    public class HttpModelToProtocolProfile : Profile
    {
        public HttpModelToProtocolProfile()
        {
            CreateMap<Http.Model.Mimicry, Protocol.Mimicry>();
            CreateMap<Protocol.Mimicry, Http.Model.Mimicry>();

            CreateMap<Http.Model.Character, Protocol.Character>()
                .ForMember(x => x.ClassType, x => x.MapFrom(u => u.Class))
                .ForMember(x => x.Mimicry, x => x.MapFrom(u => u.Mimicry))
                .ForMember(x => x.Buffs, x => x.MapFrom(u => u.Buffs))
                .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")))
                .ForMember(x => x.UpdatedDate, x => x.MapFrom(u => u.UpdatedDate.ToString("yyyy-MM-dd HH:mm:ss")))
                .ForMember(x => x.FirstLoginDate, x => x.MapFrom(u => u.FirstLoginDate.HasValue ? u.FirstLoginDate.Value.ToString("yyyy-MM-dd HH:mm:ss") : null))
                .ForMember(x => x.Position, x => x.MapFrom(u => new Protocol.Position { X = u.PositionX, Y = u.PositionY }))
                .ForMember(x => x.Role, x => x.MapFrom(u => (byte)u.Role));

            CreateMap<Protocol.Character, Http.Model.Character>()
                .ForMember(x => x.Mimicry, x => x.MapFrom(u => u.Mimicry))
                .ForMember(x => x.Buffs, x => x.MapFrom(u => u.Buffs))
                .ForMember(x => x.Class, x => x.MapFrom(u => u.ClassType))
                .ForMember(x => x.UpdatedDate, x => x.MapFrom(u => DateTime.Parse(u.UpdatedDate)))
                .ForMember(x => x.FirstLoginDate, x => x.MapFrom(u => string.IsNullOrEmpty(u.FirstLoginDate) ? null : (DateTime?)DateTime.Parse(u.FirstLoginDate)))
                .ForMember(x => x.PositionX, x => x.MapFrom(u => u.Position.X))
                .ForMember(x => x.PositionY, x => x.MapFrom(u => u.Position.Y))
                .ForMember(x => x.Role, x => x.MapFrom(u => (Fb.Model.EnumValue.Role)u.Role));

            CreateMap<Http.Model.MarketplacePending, Protocol.MarketplacePending>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.User))
                .ForMember(x => x.Attachments, x => x.Ignore())
                .AfterMap((src, dest) =>
                {
                    dest.Attachments = JsonConvert.SerializeObject(src.Attachments ?? new List<Fb.Model.Dsl>());
                });

            CreateMap<Protocol.MarketplacePending, Http.Model.MarketplacePending>()
                .ForMember(x => x.Attachments, x => x.Ignore())
                .AfterMap((src, dest) =>
                {
                    dest.Attachments = string.IsNullOrWhiteSpace(src.Attachments)
                        ? new List<Fb.Model.Dsl>()
                        : (JsonConvert.DeserializeObject<List<Fb.Model.Dsl>>(src.Attachments) ?? new List<Fb.Model.Dsl>());
                });

            CreateMap<Http.Model.Spell, Protocol.Spell>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.Owner))
                .ForMember(x => x.Next, x => x.MapFrom(u => u.Next.ToString("yyyy-MM-dd HH:mm:ss")))
                .ReverseMap();

            CreateMap<Http.Model.MatchmakingSkill, Protocol.MatchmakingSkill>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.CharacterId))
                .ReverseMap()
                .ForMember(x => x.CharacterId, x => x.MapFrom(u => u.User))
                .ForMember(x => x.CreatedDate, x => x.Ignore())
                .ForMember(x => x.UpdatedDate, x => x.Ignore());

            CreateMap<Http.Model.Item, Protocol.Item>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.Owner))
                .ForMember(x => x.ExpireTime, x => x.MapFrom(u => u.ExpireTime.HasValue ? u.ExpireTime.Value.ToString("yyyy-MM-dd HH:mm:ss") : null))
                .ReverseMap()
                .ForMember(x => x.ExpireTime, x => x.MapFrom(u => string.IsNullOrEmpty(u.ExpireTime) ? null : (DateTime?)DateTime.Parse(u.ExpireTime)));

            CreateMap<Http.Model.Achievement, Protocol.Achievement>()
                .ForMember(x => x.User, x => x.MapFrom(u => u.Uid))
                .ForMember(x => x.Model, x => x.MapFrom(u => u.Id))
                .ReverseMap()
                .ForMember(x => x.Id, x => x.MapFrom(u => u.Model));

            CreateMap<Http.Model.Option, Protocol.Option>()
                .ReverseMap();

            CreateMap<Http.Model.Marriage, Protocol.Marriage>()
                .ForMember(x => x.SpouseName, x => x.Ignore())
                .ForMember(x => x.RemarriageAfter, x => x.MapFrom(u => u.RemarriageAfter.ToString("yyyy-MM-dd HH:mm:ss")));

            CreateMap<Protocol.Marriage, Http.Model.Marriage>()
                .ForMember(x => x.CharacterId, x => x.Ignore())
                .ForMember(x => x.CreatedDate, x => x.Ignore())
                .ForMember(x => x.UpdatedDate, x => x.Ignore())
                .ForMember(x => x.RemarriageAfter, x => x.MapFrom(src => string.IsNullOrWhiteSpace(src.RemarriageAfter) ? DateTime.UtcNow : DateTime.Parse(src.RemarriageAfter)));

            CreateMap<Http.Model.Bulletin, Protocol.Article>()
                .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            CreateMap<Http.Model.Bulletin, Protocol.ArticleSummary>()
                .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            CreateMap<Http.Model.Mail, Protocol.MailSummary>()
                .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")));

            CreateMap<Http.Model.Mail, Protocol.Mail>()
                .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")))
                .ForMember(x => x.SystemMailId, x => x.MapFrom(u => u.SystemMailId));

            CreateMap<Http.Model.Clan, Protocol.Clan>();

            CreateMap<Http.Model.Buff, Protocol.Buff>()
                .ReverseMap();

            CreateMap<Http.Model.Quest, Protocol.Quest>()
                .ForMember(x => x.Uid, x => x.MapFrom(u => u.User))
                .ForMember(x => x.Qid, x => x.MapFrom(u => u.Id))
                .ReverseMap();

            CreateMap<Http.Model.StorageBox, Protocol.StorageBox>()
                .ForMember(x => x.Attachments, x => x.Ignore())
                .ForMember(x => x.SystemStorageBoxId, x => x.MapFrom(u => u.SystemStorageBoxId ?? 0))
                .ForMember(x => x.Title, x => x.MapFrom(u => u.Title ?? string.Empty))
                .ForMember(x => x.ExpiredDate, x => x.MapFrom(u => u.ExpiredDate.HasValue ? u.ExpiredDate.Value.ToString("yyyy-MM-dd HH:mm:ss") : null))
                .AfterMap((src, dest) =>
                {
                    dest.Attachments = JsonConvert.SerializeObject(src.Attachments ?? new List<Fb.Model.Dsl>());
                });

            CreateMap<Protocol.StorageBox, Http.Model.StorageBox>()
                .ForMember(x => x.Attachments, x => x.Ignore())
                .ForMember(x => x.SystemStorageBoxId, x => x.MapFrom(u => u.SystemStorageBoxId != 0 ? (uint?)u.SystemStorageBoxId : null))
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

            CreateMap<Http.Model.SystemStorageBox, Protocol.SystemStorageBox>()
                .ForMember(x => x.Attachments, x => x.Ignore())
                .ForMember(x => x.Title, x => x.MapFrom(u => u.Title ?? string.Empty))
                .ForMember(x => x.ExpiredDate, x => x.MapFrom(u => u.ExpiredDate.HasValue ? u.ExpiredDate.Value.ToString("yyyy-MM-dd HH:mm:ss") : null))
                .ForMember(x => x.CreatedDate, x => x.MapFrom(u => u.CreatedDate.ToString("yyyy-MM-dd HH:mm:ss")))
                .AfterMap((src, dest) =>
                {
                    dest.Attachments = JsonConvert.SerializeObject(src.Attachments ?? new List<Fb.Model.Dsl>());
                });
        }
    }
}