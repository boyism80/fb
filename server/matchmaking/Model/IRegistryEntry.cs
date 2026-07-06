namespace Matchmaking.Model;

public interface IRegistryEntry
{
    string EntryId { get; }

    double Mu { get; }

    double Sigma { get; }
}
