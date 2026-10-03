namespace Matchmaking.Model;

public static class SkillCalculator
{
    public static Skill ForTicket<TMember>(Ticket<TMember> ticket)
        where TMember : ITicketMember
    {
        if (ticket.Members.Count == 0)
        {
            return new Skill(0, 0);
        }

        var mu = ticket.Members.Average(member => member.Mu);
        var sigmaSquaredSum = ticket.Members.Sum(member => member.Sigma * member.Sigma);
        var sigma = Math.Sqrt(sigmaSquaredSum);
        return new Skill(mu, sigma);
    }

    public static double GetEffectiveMu<TMember>(Ticket<TMember> ticket, double effectiveMuSigmaFactor)
        where TMember : ITicketMember
    {
        if (ticket.Members.Count == 0)
        {
            return 0;
        }

        // Use per-member effective mu so party size does not shift bucket placement
        // when every member has the same individual rating.
        return ticket.Members.Average(member =>
            member.Mu - (effectiveMuSigmaFactor * member.Sigma));
    }

    public static int GetBucketKey<TMember>(
        Ticket<TMember> ticket,
        double effectiveMuSigmaFactor,
        double bucketWidth)
        where TMember : ITicketMember
    {
        return GetBucketIndex(GetEffectiveMu(ticket, effectiveMuSigmaFactor), bucketWidth);
    }

    public static int GetBucketIndex(double effectiveMu, double bucketWidth)
    {
        return (int)Math.Floor(effectiveMu / bucketWidth);
    }

    public static void GetBucketRange(
        double anchorEffectiveMu,
        double tolerance,
        double bucketWidth,
        out int minBucket,
        out int maxBucket)
    {
        minBucket = GetBucketIndex(anchorEffectiveMu - tolerance, bucketWidth);
        maxBucket = GetBucketIndex(anchorEffectiveMu + tolerance, bucketWidth);
    }

    public static Skill ForTeam<TMember>(IReadOnlyList<Ticket<TMember>> tickets)
        where TMember : ITicketMember
    {
        if (tickets.Count == 0)
        {
            return new Skill(0, 0);
        }

        var totalMembers = 0;
        var weightedMu = 0.0;
        var sigmaSquaredSum = 0.0;

        foreach (var ticket in tickets)
        {
            var skill = ForTicket(ticket);
            weightedMu += skill.Mu * ticket.Members.Count;
            sigmaSquaredSum += skill.Sigma * skill.Sigma;
            totalMembers += ticket.Members.Count;
        }

        return new Skill(weightedMu / totalMembers, Math.Sqrt(sigmaSquaredSum));
    }
}
