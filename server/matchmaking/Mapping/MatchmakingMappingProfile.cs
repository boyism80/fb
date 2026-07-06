using AutoMapper;
using Matchmaking.Model;

namespace Matchmaking.Mapping;

public class MatchmakingMappingProfile : Profile
{
    public MatchmakingMappingProfile()
    {
        CreateMap<CharacterRegistryEntry, fb.protocol.matchmaking.RegistryEntry>();

        CreateMap<Registry<CharacterRegistryEntry>, fb.protocol.matchmaking.MatchRegistry>()
            .ForMember(dest => dest.RegistryId, opt => opt.MapFrom(src => src.Id.ToString()))
            .ForMember(dest => dest.Entries, opt => opt.MapFrom(src => src.Entries));
    }
}
