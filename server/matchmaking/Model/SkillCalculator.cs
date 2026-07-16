namespace Matchmaking.Model;

public static class SkillCalculator
{
    public static Skill ForRegistry<TEntry>(Registry<TEntry> registry)
        where TEntry : IRegistryEntry
    {
        if (registry.Entries.Count == 0)
        {
            return new Skill(0, 0);
        }

        var mu = registry.Entries.Average(entry => entry.Mu);
        var sigmaSquaredSum = registry.Entries.Sum(entry => entry.Sigma * entry.Sigma);
        var sigma = Math.Sqrt(sigmaSquaredSum);
        return new Skill(mu, sigma);
    }

    public static double GetEffectiveMu<TEntry>(Registry<TEntry> registry, double effectiveMuSigmaFactor)
        where TEntry : IRegistryEntry
    {
        if (registry.Entries.Count == 0)
        {
            return 0;
        }

        // Use per-entry effective mu so party size does not shift bucket placement
        // when every member has the same individual rating.
        return registry.Entries.Average(entry =>
            entry.Mu - (effectiveMuSigmaFactor * entry.Sigma));
    }

    public static int GetBucketKey<TEntry>(
        Registry<TEntry> registry,
        double effectiveMuSigmaFactor,
        double bucketWidth)
        where TEntry : IRegistryEntry
    {
        return GetBucketIndex(GetEffectiveMu(registry, effectiveMuSigmaFactor), bucketWidth);
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

    public static Skill ForTeam<TEntry>(IReadOnlyList<Registry<TEntry>> registries)
        where TEntry : IRegistryEntry
    {
        if (registries.Count == 0)
        {
            return new Skill(0, 0);
        }

        var totalEntries = 0;
        var weightedMu = 0.0;
        var sigmaSquaredSum = 0.0;

        foreach (var registry in registries)
        {
            var skill = ForRegistry(registry);
            weightedMu += skill.Mu * registry.Entries.Count;
            sigmaSquaredSum += skill.Sigma * skill.Sigma;
            totalEntries += registry.Entries.Count;
        }

        return new Skill(weightedMu / totalEntries, Math.Sqrt(sigmaSquaredSum));
    }
}
