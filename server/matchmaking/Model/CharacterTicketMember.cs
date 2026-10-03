namespace Matchmaking.Model;

public class CharacterTicketMember : ITicketMember
{
    public uint World { get; set; }

    public uint CharacterId { get; set; }

    public double Mu { get; set; }

    public double Sigma { get; set; }

    public string MemberId => ToMemberId(World, CharacterId);

    public static string ToMemberId(uint world, uint characterId)
    {
        return $"{world}:{characterId}";
    }

    public static CharacterTicketMember FromProtocol(fb.protocol.matchmaking.TicketMember member)
    {
        return new CharacterTicketMember
        {
            World = member.World,
            CharacterId = member.CharacterId,
            Mu = member.Mu,
            Sigma = member.Sigma
        };
    }
}
