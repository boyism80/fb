using Fb.Model;
using Matchmaking.Model;
using Matchmaking.Options;

namespace Matchmaking.Core;

public sealed class TicketQueue<TMember>
    where TMember : ITicketMember
{
    private readonly Fb.Model.Matchmaking _config;
    private readonly MatchmakingOptions _options;
    private readonly SortedSet<Ticket<TMember>> _byCreatedAt = new();
    private readonly SortedDictionary<int, List<Ticket<TMember>>> _buckets = new();
    private readonly object _lock = new();

    public TicketQueue(Fb.Model.Matchmaking config, MatchmakingOptions options)
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
                return _byCreatedAt.Count;
            }
        }
    }

    public string Describe()
    {
        lock (_lock)
        {
            if (_byCreatedAt.Count == 0)
            {
                return "empty";
            }

            var oldest = (DateTime.UtcNow - _byCreatedAt.Min.CreatedAt).TotalSeconds;
            return $"{_byCreatedAt.Count} tickets / {TotalMemberCount()} members (oldest {oldest:F1}s)";
        }
    }

    public void Add(Ticket<TMember> ticket)
    {
        lock (_lock)
        {
            if (_byCreatedAt.Contains(ticket))
            {
                throw new InvalidOperationException($"Ticket {ticket.Id} is already in the queue.");
            }

            _byCreatedAt.Add(ticket);

            var bucketKey = SkillCalculator.GetBucketKey(
                ticket,
                _options.EffectiveMuSigmaFactor,
                _options.SkillBucketWidth);
            if (!_buckets.TryGetValue(bucketKey, out var bucket))
            {
                bucket = new List<Ticket<TMember>>();
                _buckets[bucketKey] = bucket;
            }

            bucket.Add(ticket);
        }
    }

    public Ticket<TMember> Remove(ulong ticketId)
    {
        lock (_lock)
        {
            var ticket = _byCreatedAt.FirstOrDefault(candidate => candidate.Id == ticketId);
            if (ticket == null)
            {
                return null;
            }

            _byCreatedAt.Remove(ticket);
            RemoveFromBucket(ticket);
            return ticket;
        }
    }

    public List<Match<TMember>> TryFormMatch()
    {
        lock (_lock)
        {
            var matches = new List<Match<TMember>>();

            while (TotalMemberCount() >= MembersPerMatch)
            {
                Match<TMember> match = null;
                foreach (var anchor in _byCreatedAt)
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

                RemoveMatchedTickets(match);
                matches.Add(match);
            }

            return matches;
        }
    }

    private Match<TMember> TryFormMatchWithAnchor(Ticket<TMember> anchor)
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

        var used = new HashSet<ulong>(anchorTeam.Select(ticket => ticket.Id));
        var teams = new List<IReadOnlyList<Ticket<TMember>>> { anchorTeam };

        for (var teamIndex = 1; teamIndex < (int)_config.TeamCount; teamIndex++)
        {
            var remaining = candidates.Where(ticket => !used.Contains(ticket.Id)).ToList();
            var team = TryBuildTeam(remaining, required: null, (int)_config.MemberCount);
            if (team == null)
            {
                return null;
            }

            teams.Add(team);
            foreach (var ticket in team)
            {
                used.Add(ticket.Id);
            }
        }

        return new Match<TMember>(teams);
    }

    private List<Ticket<TMember>> GetCandidatesInWindow(Ticket<TMember> anchor, double tolerance)
    {
        var anchorEffectiveMu = SkillCalculator.GetEffectiveMu(anchor, _options.EffectiveMuSigmaFactor);
        SkillCalculator.GetBucketRange(
            anchorEffectiveMu,
            tolerance,
            _options.SkillBucketWidth,
            out var minBucket,
            out var maxBucket);

        var candidates = new List<Ticket<TMember>>();
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
            .OrderBy(ticket => ticket.CreatedAt)
            .ThenBy(ticket => ticket.Id)
            .ToList();
    }

    private List<Ticket<TMember>> TryBuildTeam(List<Ticket<TMember>> candidates, Ticket<TMember> required, int targetMembers)
    {
        var team = new List<Ticket<TMember>>();
        var remaining = candidates;

        if (required != null)
        {
            team.Add(required);
            remaining = candidates.Where(ticket => ticket.Id != required.Id).ToList();
        }

        return BuildTeamRecursive(team, remaining, targetMembers);
    }

    private List<Ticket<TMember>> BuildTeamRecursive(List<Ticket<TMember>> team, List<Ticket<TMember>> remaining, int targetMembers)
    {
        var currentMembers = team.Sum(ticket => ticket.Members.Count);
        if (currentMembers == targetMembers)
        {
            return team;
        }

        if (currentMembers > targetMembers)
        {
            return null;
        }

        var needed = targetMembers - currentMembers;
        if (remaining.Sum(ticket => ticket.Members.Count) < needed)
        {
            return null;
        }

        foreach (var next in remaining)
        {
            if (next.Members.Count > needed)
            {
                continue;
            }

            var newTeam = team.Append(next).ToList();
            var newRemaining = remaining.Where(ticket => ticket.Id != next.Id).ToList();
            var result = BuildTeamRecursive(newTeam, newRemaining, targetMembers);
            if (result != null)
            {
                return result;
            }
        }

        return null;
    }

    private void RemoveFromBucket(Ticket<TMember> ticket)
    {
        var bucketKey = SkillCalculator.GetBucketKey(
            ticket,
            _options.EffectiveMuSigmaFactor,
            _options.SkillBucketWidth);
        if (!_buckets.TryGetValue(bucketKey, out var bucket))
        {
            return;
        }

        bucket.RemoveAll(candidate => candidate.Id == ticket.Id);
        if (bucket.Count == 0)
        {
            _buckets.Remove(bucketKey);
        }
    }

    private void RemoveMatchedTickets(Match<TMember> match)
    {
        foreach (var ticket in match.AllTickets)
        {
            _byCreatedAt.Remove(ticket);
            RemoveFromBucket(ticket);
        }
    }

    private int TotalMemberCount()
    {
        return _byCreatedAt.Sum(ticket => ticket.Members.Count);
    }

    private double GetSkillTolerance(Ticket<TMember> anchor)
    {
        var waitSeconds = Math.Max(0, (DateTime.UtcNow - anchor.CreatedAt).TotalSeconds);
        var tolerance = _options.BaseSkillTolerance + (_options.SkillTolerancePerSecond * waitSeconds);
        return Math.Min(_options.MaxSkillTolerance, tolerance);
    }

    private int MembersPerMatch => (int)(_config.MemberCount * _config.TeamCount);
}
