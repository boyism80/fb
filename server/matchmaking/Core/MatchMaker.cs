using Fb.Model;
using Fb.Model.EnumValue;
using Http;
using Matchmaking.Model;
using Matchmaking.Options;
using Microsoft.Extensions.Options;

namespace Matchmaking.Core;

public class MatchMaker<TEntry>
    where TEntry : IRegistryEntry
{
    private readonly MatchmakingOptions _options;
    private readonly ILogger _logger;
    private readonly Dictionary<uint, RegistryQueue<TEntry>> _registryQueues = new();
    private readonly Dictionary<Guid, Match<TEntry>> _pendingMatches = new();
    private readonly Dictionary<string, EnrollmentRef> _activeEntries = new();
    private readonly Dictionary<Guid, (uint MatchType, EnrollmentState State)> _registryIndex = new();
    private readonly object _lock = new();

    public event Func<ProposedMatchResult<TEntry>, CancellationToken, Task> MatchProposed;

    public event Func<Match<TEntry>, CancellationToken, Task> MatchReady;

    public event Func<DissolvedMatchResult<TEntry>, CancellationToken, Task> MatchDissolved;

    public MatchMaker(IOptions<MatchmakingOptions> options, ILogger logger)
    {
        _options = options.Value;
        _logger = logger;
    }

    public Guid Enroll(uint matchType, IReadOnlyList<TEntry> entries)
    {
        if (entries == null || entries.Count == 0)
        {
            throw new LogicException(ErrorCode.Unhandled);
        }

        lock (_lock)
        {
            var entryIds = new HashSet<string>();
            foreach (var entry in entries)
            {
                if (entry == null)
                {
                    throw new LogicException(ErrorCode.Unhandled);
                }

                if (!entryIds.Add(entry.EntryId))
                {
                    throw new LogicException(ErrorCode.MatchmakingDuplicateEntry);
                }

                if (_activeEntries.TryGetValue(entry.EntryId, out var active)
                    && active.State == EnrollmentState.Pending)
                {
                    throw new LogicException(ErrorCode.MatchmakingAlreadyEnrolled);
                }
            }

            // A waiting enrollment without a live owner is leftover state, so drop it
            // instead of locking the character out of the queue forever.
            foreach (var entry in entries)
            {
                if (!_activeEntries.TryGetValue(entry.EntryId, out var stale))
                {
                    continue;
                }

                _logger.LogWarning(
                    "Enroll evicted leftover registry {RegistryId} of {EntryId} for match type {MatchType}",
                    stale.RegistryId,
                    entry.EntryId,
                    stale.MatchType);
                UnregisterFromQueue(stale.MatchType, stale.RegistryId);
                ClearRegistryEnrollment(stale.RegistryId);
            }

            Registry<TEntry> registry;
            try
            {
                registry = RegisterToQueue(matchType, entries, DateTime.UtcNow);
            }
            catch (ArgumentException ex) when (ex.Message.Contains("Unknown match type"))
            {
                throw new LogicException(ErrorCode.MatchmakingUnknownQueue);
            }
            catch (ArgumentException ex) when (ex.Message.Contains("Duplicate EntryId"))
            {
                throw new LogicException(ErrorCode.MatchmakingDuplicateEntry);
            }
            catch (ArgumentException)
            {
                throw new LogicException(ErrorCode.Unhandled);
            }

            foreach (var entry in entries)
            {
                _activeEntries[entry.EntryId] = new EnrollmentRef(
                    EnrollmentState.Waiting,
                    matchType,
                    registry.Id);
            }

            _registryIndex[registry.Id] = (matchType, EnrollmentState.Waiting);
            _logger.LogInformation(
                "Enrolled registry {RegistryId} for match type {MatchType} with entries {Entries}",
                registry.Id,
                matchType,
                string.Join(",", entries.Select(entry => entry.EntryId)));
            LogQueueState("enroll");
            return registry.Id;
        }
    }

    public async Task UnenrollAsync(
        uint matchType,
        Guid registryId,
        string entryId,
        CancellationToken cancellationToken = default)
    {
        DissolvedMatchResult<TEntry> dissolved = null;

        lock (_lock)
        {
            if (!_activeEntries.TryGetValue(entryId, out var enrollment))
            {
                _logger.LogWarning(
                    "Unenroll from {EntryId} found nothing to remove (requested registry {RegistryId}, match type {MatchType})",
                    entryId,
                    registryId,
                    matchType);
                throw new LogicException(ErrorCode.MatchmakingRegistryNotFound);
            }

            if (enrollment.RegistryId != registryId || enrollment.MatchType != matchType)
            {
                _logger.LogWarning(
                    "Unenroll from {EntryId} referenced {RegistryId}/{MatchType} but the live enrollment is {LiveRegistryId}/{LiveMatchType}",
                    entryId,
                    registryId,
                    matchType,
                    enrollment.RegistryId,
                    enrollment.MatchType);
            }

            if (enrollment.State == EnrollmentState.Pending && enrollment.MatchId.HasValue
                && _pendingMatches.TryGetValue(enrollment.MatchId.Value, out var match))
            {
                dissolved = Dissolve(match, DissolveReason.Decline, entryId);
            }
            else
            {
                UnregisterFromQueue(enrollment.MatchType, enrollment.RegistryId);
                ClearRegistryEnrollment(enrollment.RegistryId);
                _logger.LogInformation(
                    "Unenrolled registry {RegistryId} of match type {MatchType} requested by {EntryId}",
                    enrollment.RegistryId,
                    enrollment.MatchType,
                    entryId);
                LogQueueState("unenroll");
            }
        }

        if (dissolved != null)
        {
            await RaiseMatchDissolvedAsync(dissolved, cancellationToken);
        }
    }

    public async Task<bool> ConfirmAsync(Guid matchId, string entryId, CancellationToken cancellationToken = default)
    {
        Match<TEntry> readyMatch = null;
        var matchFinalized = false;

        lock (_lock)
        {
            if (!_pendingMatches.TryGetValue(matchId, out var match))
            {
                throw new LogicException(ErrorCode.MatchmakingMatchNotFound);
            }

            if (IsExpired(match, DateTime.UtcNow))
            {
                throw new LogicException(ErrorCode.MatchmakingConfirmExpired);
            }

            if (!match.AllEntryIds.Contains(entryId))
            {
                throw new LogicException(ErrorCode.MatchmakingNotParticipant);
            }

            if (match.ConfirmedEntryIds.Contains(entryId))
            {
                throw new LogicException(ErrorCode.MatchmakingAlreadyConfirmed);
            }

            match.ConfirmedEntryIds.Add(entryId);
            _logger.LogInformation(
                "Match {MatchId} confirmed by {EntryId} ({Confirmed}/{Total})",
                matchId,
                entryId,
                match.ConfirmedEntryIds.Count,
                match.AllEntryIds.Count());

            if (!match.AllEntryIds.All(id => match.ConfirmedEntryIds.Contains(id)))
            {
                return false;
            }

            readyMatch = FinalizeMatch(match);
            matchFinalized = true;
        }

        if (matchFinalized && readyMatch != null)
        {
            await RaiseMatchReadyAsync(readyMatch, cancellationToken);
        }

        return matchFinalized;
    }

    public async Task DeclineAsync(Guid matchId, string entryId, CancellationToken cancellationToken = default)
    {
        DissolvedMatchResult<TEntry> result;

        lock (_lock)
        {
            if (!_pendingMatches.TryGetValue(matchId, out var match))
            {
                throw new LogicException(ErrorCode.MatchmakingMatchNotFound);
            }

            if (!match.AllEntryIds.Contains(entryId))
            {
                throw new LogicException(ErrorCode.MatchmakingNotParticipant);
            }

            _logger.LogInformation("Match {MatchId} declined by {EntryId}", matchId, entryId);
            result = Dissolve(match, DissolveReason.Decline, entryId);
        }

        await RaiseMatchDissolvedAsync(result, cancellationToken);
    }

    public MatchmakingStatus GetStatus(string entryId)
    {
        lock (_lock)
        {
            if (!_activeEntries.TryGetValue(entryId, out var enrollment))
            {
                return new MatchmakingStatus();
            }

            if (enrollment.State == EnrollmentState.Waiting)
            {
                return new MatchmakingStatus
                {
                    InQueue = true,
                    MatchType = enrollment.MatchType,
                    RegistryId = enrollment.RegistryId
                };
            }

            if (!enrollment.MatchId.HasValue
                || !_pendingMatches.TryGetValue(enrollment.MatchId.Value, out var match))
            {
                return new MatchmakingStatus();
            }

            return new MatchmakingStatus
            {
                MatchType = enrollment.MatchType,
                RegistryId = enrollment.RegistryId,
                PendingMatchId = enrollment.MatchId,
                ConfirmDeadline = GetConfirmDeadline(match)
            };
        }
    }

    public async Task TickMatchmakingAsync(CancellationToken cancellationToken = default)
    {
        List<ProposedMatchResult<TEntry>> proposed;

        lock (_lock)
        {
            proposed = new List<ProposedMatchResult<TEntry>>();
            var results = TryFormMatches();

            foreach (var (matchType, matches) in results)
            {
                foreach (var match in matches)
                {
                    foreach (var registry in match.AllRegistries)
                    {
                        _registryIndex[registry.Id] = (matchType, EnrollmentState.Pending);
                        foreach (var entry in registry.Entries)
                        {
                            _activeEntries[entry.EntryId] = new EnrollmentRef(
                                EnrollmentState.Pending,
                                matchType,
                                registry.Id,
                                match.MatchId);
                        }
                    }

                    _logger.LogInformation(
                        "Proposed match {MatchId} for match type {MatchType} with registries {Registries}",
                        match.MatchId,
                        matchType,
                        string.Join(",", match.AllRegistries.Select(registry => registry.Id)));

                    proposed.Add(new ProposedMatchResult<TEntry>
                    {
                        Match = match,
                        ConfirmDeadline = GetConfirmDeadline(match)
                    });
                }
            }

            if (proposed.Count > 0)
            {
                LogQueueState("propose");
            }
        }

        foreach (var item in proposed)
        {
            cancellationToken.ThrowIfCancellationRequested();
            await RaiseMatchProposedAsync(item, cancellationToken);
        }
    }

    public async Task TickConfirmationAsync(CancellationToken cancellationToken = default)
    {
        List<DissolvedMatchResult<TEntry>> dissolved;

        lock (_lock)
        {
            dissolved = new List<DissolvedMatchResult<TEntry>>();
            var expiredMatches = _pendingMatches.Values
                .Where(match => IsExpired(match, DateTime.UtcNow))
                .ToList();

            foreach (var match in expiredMatches)
            {
                if (!_pendingMatches.ContainsKey(match.MatchId))
                {
                    continue;
                }

                dissolved.Add(Dissolve(match, DissolveReason.Timeout));
            }
        }

        foreach (var item in dissolved)
        {
            cancellationToken.ThrowIfCancellationRequested();
            await RaiseMatchDissolvedAsync(item, cancellationToken);
        }
    }

    public void LogQueueState(string reason)
    {
        lock (_lock)
        {
            var queues = string.Join(
                ", ",
                _registryQueues.Select(pair => $"{pair.Key}:{pair.Value.Describe()}"));

            _logger.LogInformation(
                "Matchmaking state after {Reason}: queues [{Queues}], pending matches {PendingMatches}, enrolled entries {EnrolledEntries}",
                reason,
                queues,
                _pendingMatches.Count,
                _activeEntries.Count);
        }
    }

    private async Task RaiseMatchProposedAsync(
        ProposedMatchResult<TEntry> args,
        CancellationToken cancellationToken)
    {
        var handler = MatchProposed;
        if (handler == null)
        {
            return;
        }

        var handlers = handler.GetInvocationList()
            .Cast<Func<ProposedMatchResult<TEntry>, CancellationToken, Task>>();
        await Task.WhenAll(handlers.Select(h => h(args, cancellationToken)));
    }

    private async Task RaiseMatchReadyAsync(Match<TEntry> match, CancellationToken cancellationToken)
    {
        var handler = MatchReady;
        if (handler == null)
        {
            return;
        }

        var handlers = handler.GetInvocationList()
            .Cast<Func<Match<TEntry>, CancellationToken, Task>>();
        await Task.WhenAll(handlers.Select(h => h(match, cancellationToken)));
    }

    private async Task RaiseMatchDissolvedAsync(
        DissolvedMatchResult<TEntry> result,
        CancellationToken cancellationToken)
    {
        var handler = MatchDissolved;
        if (handler == null)
        {
            return;
        }

        var handlers = handler.GetInvocationList()
            .Cast<Func<DissolvedMatchResult<TEntry>, CancellationToken, Task>>();
        await Task.WhenAll(handlers.Select(h => h(result, cancellationToken)));
    }

    private Match<TEntry> FinalizeMatch(Match<TEntry> match)
    {
        foreach (var registry in match.AllRegistries)
        {
            ClearRegistryEnrollment(registry.Id);
        }

        _pendingMatches.Remove(match.MatchId);
        LogQueueState("finalize");
        return match;
    }

    private DissolvedMatchResult<TEntry> Dissolve(Match<TEntry> match, DissolveReason reason, string decliningEntryId = null)
    {
        var outcomes = new List<RegistryOutcomeNotification<TEntry>>();
        foreach (var registry in match.AllRegistries)
        {
            var hasFault = reason == DissolveReason.Decline
                ? registry.Entries.Any(entry => entry.EntryId == decliningEntryId)
                : registry.Entries.Any(entry => !match.ConfirmedEntryIds.Contains(entry.EntryId));

            outcomes.Add(new RegistryOutcomeNotification<TEntry>
            {
                RegistryId = registry.Id,
                Requeued = !hasFault,
                Entries = registry.Entries.ToList()
            });
        }

        foreach (var (registry, outcome) in match.AllRegistries.Zip(outcomes))
        {
            if (!outcome.Requeued)
            {
                ClearRegistryEnrollment(registry.Id);
                _logger.LogInformation(
                    "Registry {RegistryId} dropped from match {MatchId} ({Reason})",
                    registry.Id,
                    match.MatchId,
                    reason);
            }
            else
            {
                // The registry keeps its id so game servers never hold a dangling reference.
                _registryQueues[match.MatchType].Add(registry);
                _registryIndex[registry.Id] = (match.MatchType, EnrollmentState.Waiting);
                foreach (var entry in registry.Entries)
                {
                    _activeEntries[entry.EntryId] = new EnrollmentRef(
                        EnrollmentState.Waiting,
                        match.MatchType,
                        registry.Id);
                }

                _logger.LogInformation(
                    "Registry {RegistryId} requeued after match {MatchId} ({Reason})",
                    registry.Id,
                    match.MatchId,
                    reason);
            }
        }

        _pendingMatches.Remove(match.MatchId);
        LogQueueState("dissolve");
        return new DissolvedMatchResult<TEntry>
        {
            Match = match,
            Reason = reason,
            Outcomes = outcomes
        };
    }

    private void ClearRegistryEnrollment(Guid registryId)
    {
        if (!_registryIndex.Remove(registryId, out _))
        {
            return;
        }

        var entryIds = _activeEntries
            .Where(pair => pair.Value.RegistryId == registryId)
            .Select(pair => pair.Key)
            .ToList();

        foreach (var entryId in entryIds)
        {
            _activeEntries.Remove(entryId);
        }
    }

    private Registry<TEntry> RegisterToQueue(uint matchType, IReadOnlyList<TEntry> entries, DateTime createdDateTime)
    {
        if (matchType == 0)
        {
            throw new ArgumentException("Match type is required.", nameof(matchType));
        }

        if (entries == null || entries.Count == 0)
        {
            throw new ArgumentException("At least one registry entry is required.", nameof(entries));
        }

        if (!Table.Matchmaking.TryGetValue((Fb.Model.EnumValue.MatchType)matchType, out var matchmakingConfig))
        {
            throw new ArgumentException($"Unknown match type: {matchType}", nameof(matchType));
        }

        if (entries.Count > matchmakingConfig.MemberCount)
        {
            throw new ArgumentException(
                $"Registry cannot contain more than {matchmakingConfig.MemberCount} entries for match type {matchType}.",
                nameof(entries));
        }

        ValidateEntries(entries);

        var registry = new Registry<TEntry>(Guid.NewGuid(), createdDateTime, entries.ToList());
        if (!_registryQueues.TryGetValue(matchType, out var queue))
        {
            queue = new RegistryQueue<TEntry>(matchmakingConfig, _options);
            _registryQueues[matchType] = queue;
        }

        queue.Add(registry);
        return registry;
    }

    private bool UnregisterFromQueue(uint matchType, Guid registryId)
    {
        if (matchType == 0)
        {
            throw new ArgumentException("Match type is required.", nameof(matchType));
        }

        if (!_registryQueues.TryGetValue(matchType, out var queue))
        {
            return false;
        }

        return queue.Remove(registryId);
    }

    private IReadOnlyList<(uint MatchType, List<Match<TEntry>> Matches)> TryFormMatches()
    {
        var results = new List<(uint, List<Match<TEntry>>)>();

        foreach (var entry in _registryQueues)
        {
            var matches = entry.Value.TryFormMatch();
            if (matches.Count == 0)
            {
                continue;
            }

            var createdAt = DateTime.UtcNow;
            foreach (var match in matches)
            {
                match.MatchId = Guid.NewGuid();
                match.MatchType = entry.Key;
                match.CreatedAt = createdAt;
                _pendingMatches[match.MatchId] = match;
            }

            results.Add((entry.Key, matches));
        }

        return results;
    }

    private static DateTime GetConfirmDeadline(Match<TEntry> match, MatchmakingOptions options)
    {
        return match.CreatedAt.AddSeconds(Math.Max(1, options.ConfirmTimeoutSeconds));
    }

    private DateTime GetConfirmDeadline(Match<TEntry> match)
    {
        return GetConfirmDeadline(match, _options);
    }

    private static bool IsExpired(Match<TEntry> match, DateTime utcNow, MatchmakingOptions options)
    {
        return utcNow >= GetConfirmDeadline(match, options);
    }

    private bool IsExpired(Match<TEntry> match, DateTime utcNow)
    {
        return IsExpired(match, utcNow, _options);
    }

    private static void ValidateEntries(IReadOnlyList<TEntry> entries)
    {
        var entryIds = new HashSet<string>();
        foreach (var entry in entries)
        {
            if (entry == null)
            {
                throw new ArgumentException("Registry entry must not be null.");
            }

            if (string.IsNullOrWhiteSpace(entry.EntryId))
            {
                throw new ArgumentException("EntryId is required.");
            }

            if (!double.IsFinite(entry.Mu))
            {
                throw new ArgumentException("Mu must be a finite number.");
            }

            if (!double.IsFinite(entry.Sigma) || entry.Sigma <= 0)
            {
                throw new ArgumentException("Sigma must be a finite number greater than zero.");
            }

            if (!entryIds.Add(entry.EntryId))
            {
                throw new ArgumentException("Duplicate EntryId within registry entries.");
            }
        }
    }
}
