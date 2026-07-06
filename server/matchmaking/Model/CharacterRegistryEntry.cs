namespace Matchmaking.Model;

public class CharacterRegistryEntry : IRegistryEntry
{
    public uint World { get; set; }

    public uint CharacterId { get; set; }

    public double Mu { get; set; }

    public double Sigma { get; set; }

    public string EntryId => ToEntryId(World, CharacterId);

    public static string ToEntryId(uint world, uint characterId)
    {
        return $"{world}:{characterId}";
    }

    public static CharacterRegistryEntry FromProtocol(fb.protocol.matchmaking.RegistryEntry entry)
    {
        return new CharacterRegistryEntry
        {
            World = entry.World,
            CharacterId = entry.CharacterId,
            Mu = entry.Mu,
            Sigma = entry.Sigma
        };
    }
}
