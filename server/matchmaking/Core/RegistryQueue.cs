using Fb.Model;
using Matchmaking.Model;
using Matchmaking.Options;

namespace Matchmaking.Core;

public sealed class RegistryQueue<TEntry>
    where TEntry : IRegistryEntry
{
    private readonly Fb.Model.Matchmaking _config;
    private readonly MatchmakingOptions _options;
    private readonly SortedSet<Registry<TEntry>> _byCreatedDate = new();
    private readonly SortedDictionary<int, List<Registry<TEntry>>> _buckets = new();
    private readonly object _lock = new();

    public RegistryQueue(Fb.Model.Matchmaking config, MatchmakingOptions options)
    {
        _config = config;
        _options = options;
        if (_options.SkillBucketWidth <= 0)
        {
            throw new ArgumentOutOfRangeException(
                nameof(options),
                "SkillBucketWidth must be greater than zero.");
        }
    }

    public int Count
    {
        get
        {
            lock (_lock)
            {
                return _byCreatedDate.Count;
            }
        }
    }

    public string Describe()
    {
        lock (_lock)
        {
            if (_byCreatedDate.Count == 0)
            {
                return "empty";
            }

            var oldest = (DateTime.UtcNow - _byCreatedDate.Min.CreatedDateTime).TotalSeconds;
            return $"{_byCreatedDate.Count} registries / {TotalEntryCount()} entries (oldest {oldest:F1}s)";
        }
    }

    public void Add(Registry<TEntry> registry)
    {
        lock (_lock)
        {
            if (_byCreatedDate.Contains(registry))
            {
                throw new InvalidOperationException($"Registry {registry.Id} is already in the queue.");
            }

            _byCreatedDate.Add(registry);

            var bucketKey = SkillCalculator.GetBucketKey(
                registry,
                _options.EffectiveMuSigmaFactor,
                _options.SkillBucketWidth);
            if (!_buckets.TryGetValue(bucketKey, out var bucket))
            {
                bucket = new List<Registry<TEntry>>();
                _buckets[bucketKey] = bucket;
            }

            bucket.Add(registry);
        }
    }

    public bool Remove(Guid registryId)
    {
        lock (_lock)
        {
            var registry = _byCreatedDate.FirstOrDefault(candidate => candidate.Id == registryId);
            if (registry == null)
            {
                return false;
            }

            _byCreatedDate.Remove(registry);
            RemoveFromBucket(registry);
            return true;
        }
    }

    public List<Match<TEntry>> TryFormMatch()
    {
        lock (_lock)
        {
            var matches = new List<Match<TEntry>>();

            while (TotalEntryCount() >= EntriesPerMatch)
            {
                Match<TEntry> match = null;
                foreach (var anchor in _byCreatedDate)
                {
                    match = TryFormMatchWithAnchor(anchor);
                    if (match != null)
                    {
                        break;
                    }
                }

                if (match == null)
                {
                    break;
                }

                RemoveMatchedRegistries(match);
                matches.Add(match);
            }

            return matches;
        }
    }

    private Match<TEntry> TryFormMatchWithAnchor(Registry<TEntry> anchor)
    {
        var tolerance = GetSkillTolerance(anchor);
        var candidates = GetCandidatesInWindow(anchor, tolerance);
        if (candidates.Count == 0)
        {
            return null;
        }

        var anchorTeam = TryBuildTeam(candidates, anchor, (int)_config.MemberCount);
        if (anchorTeam == null)
        {
            return null;
        }

        var used = new HashSet<Guid>(anchorTeam.Select(registry => registry.Id));
        var teams = new List<IReadOnlyList<Registry<TEntry>>> { anchorTeam };

        for (var teamIndex = 1; teamIndex < (int)_config.TeamCount; teamIndex++)
        {
            var remaining = candidates.Where(registry => !used.Contains(registry.Id)).ToList();
            var team = TryBuildTeam(remaining, required: null, (int)_config.MemberCount);
            if (team == null)
            {
                return null;
            }

            teams.Add(team);
            foreach (var registry in team)
            {
                used.Add(registry.Id);
            }
        }

        return new Match<TEntry>(teams);
    }

    private List<Registry<TEntry>> GetCandidatesInWindow(Registry<TEntry> anchor, double tolerance)
    {
        var anchorEffectiveMu = SkillCalculator.GetEffectiveMu(anchor, _options.EffectiveMuSigmaFactor);
        SkillCalculator.GetBucketRange(
            anchorEffectiveMu,
            tolerance,
            _options.SkillBucketWidth,
            out var minBucket,
            out var maxBucket);

        var candidates = new List<Registry<TEntry>>();
        foreach (var entry in _buckets)
        {
            if (entry.Key < minBucket)
            {
                continue;
            }

            if (entry.Key > maxBucket)
            {
                break;
            }

            candidates.AddRange(entry.Value);
        }

        return candidates
            .OrderBy(registry => registry.CreatedDateTime)
            .ThenBy(registry => registry.Id)
            .ToList();
    }

    private List<Registry<TEntry>> TryBuildTeam(List<Registry<TEntry>> candidates, Registry<TEntry> required, int targetEntries)
    {
        var team = new List<Registry<TEntry>>();
        var remaining = candidates;

        if (required != null)
        {
            team.Add(required);
            remaining = candidates.Where(registry => registry.Id != required.Id).ToList();
        }

        return BuildTeamRecursive(team, remaining, targetEntries);
    }

    private List<Registry<TEntry>> BuildTeamRecursive(List<Registry<TEntry>> team, List<Registry<TEntry>> remaining, int targetEntries)
    {
        var currentEntries = team.Sum(registry => registry.Entries.Count);
        if (currentEntries == targetEntries)
        {
            return team;
        }

        if (currentEntries > targetEntries)
        {
            return null;
        }

        var needed = targetEntries - currentEntries;
        if (remaining.Sum(registry => registry.Entries.Count) < needed)
        {
            return null;
        }

        foreach (var next in remaining)
        {
            if (next.Entries.Count > needed)
            {
                continue;
            }

            var newTeam = team.Append(next).ToList();
            var newRemaining = remaining.Where(registry => registry.Id != next.Id).ToList();
            var result = BuildTeamRecursive(newTeam, newRemaining, targetEntries);
            if (result != null)
            {
                return result;
            }
        }

        return null;
    }

    private void RemoveFromBucket(Registry<TEntry> registry)
    {
        var bucketKey = SkillCalculator.GetBucketKey(
            registry,
            _options.EffectiveMuSigmaFactor,
            _options.SkillBucketWidth);
        if (!_buckets.TryGetValue(bucketKey, out var bucket))
        {
            return;
        }

        bucket.RemoveAll(candidate => candidate.Id == registry.Id);
        if (bucket.Count == 0)
        {
            _buckets.Remove(bucketKey);
        }
    }

    private void RemoveMatchedRegistries(Match<TEntry> match)
    {
        foreach (var registry in match.AllRegistries)
        {
            _byCreatedDate.Remove(registry);
            RemoveFromBucket(registry);
        }
    }

    private int TotalEntryCount()
    {
        return _byCreatedDate.Sum(registry => registry.Entries.Count);
    }

    private double GetSkillTolerance(Registry<TEntry> anchor)
    {
        var waitSeconds = Math.Max(0, (DateTime.UtcNow - anchor.CreatedDateTime).TotalSeconds);
        var tolerance = _options.BaseSkillTolerance + (_options.SkillTolerancePerSecond * waitSeconds);
        return Math.Min(_options.MaxSkillTolerance, tolerance);
    }

    private int EntriesPerMatch => (int)(_config.MemberCount * _config.TeamCount);
}
