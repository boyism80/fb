namespace Matchmaking.Model;

public readonly struct Skill
{
    public Skill(double mu, double sigma)
    {
        Mu = mu;
        Sigma = sigma;
    }

    public double Mu { get; }

    public double Sigma { get; }
}
