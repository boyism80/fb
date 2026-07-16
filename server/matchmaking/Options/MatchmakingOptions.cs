namespace Matchmaking.Options;

public class MatchmakingOptions
{
    public int TickIntervalMs { get; set; } = 500;

    /// <summary>Initial bucket window half-width in Mu space.</summary>
    public double BaseSkillTolerance { get; set; } = 1.0;

    /// <summary>Bucket window growth per second of anchor wait (linear).</summary>
    public double SkillTolerancePerSecond { get; set; } = 0.1;

    /// <summary>Upper cap on tolerance; linear growth stops at this value.</summary>
    public double MaxSkillTolerance { get; set; } = 5.0;

    /// <summary>k in effectiveMu = mu - k * sigma for bucket assignment.</summary>
    public double EffectiveMuSigmaFactor { get; set; } = 3.0;

    /// <summary>Effective-Mu span per bucket index (e.g. 1.0 or 0.5).</summary>
    public double SkillBucketWidth { get; set; } = 1.0;

    /// <summary>Seconds for all registry entries to confirm after match creation.</summary>
    public int ConfirmTimeoutSeconds { get; set; } = 30;
}
