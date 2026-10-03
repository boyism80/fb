namespace Matchmaking.Model;

public interface ITicketMember
{
    string MemberId { get; }

    double Mu { get; }

    double Sigma { get; }
}
