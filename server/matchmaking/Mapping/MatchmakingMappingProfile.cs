using AutoMapper;
using Matchmaking.Model;

namespace Matchmaking.Mapping;

public class MatchmakingMappingProfile : Profile
{
    public MatchmakingMappingProfile()
    {
        CreateMap<CharacterTicketMember, fb.protocol.matchmaking.TicketMember>();

        CreateMap<Ticket<CharacterTicketMember>, fb.protocol.matchmaking.Ticket>()
            .ForMember(dest => dest.TicketId, opt => opt.MapFrom(src => src.Id))
            .ForMember(dest => dest.Members, opt => opt.MapFrom(src => src.Members));
    }
}
