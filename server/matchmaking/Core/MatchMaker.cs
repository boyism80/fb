using Fb.Model;
using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Http.Util;
using Matchmaking.Model;
using Matchmaking.Options;
using Microsoft.Extensions.Options;

namespace Matchmaking.Core;

public class MatchMaker<TMember>
    where TMember : ITicketMember
{
    private readonly MatchmakingOptions _options;
    private readonly ILogger _logger;
    private readonly Dictionary<uint, TicketQueue<TMember>> _ticketQueues = new();
    private readonly Dictionary<ulong, Match<TMember>> _pendingMatches = new();
    private readonly Dictionary<string, TicketRef> _memberTickets = new();
    private readonly SnowflakeId _ids = new();
    private readonly object _lock = new();

    public event Func<ProposedMatchResult<TMember>, CancellationToken, Task> MatchProposed;

    public event Func<Match<TMember>, CancellationToken, Task> MatchReady;

    public event Func<DissolvedMatchResult<TMember>, CancellationToken, Task> MatchDissolved;

    public event Func<Ticket<TMember>, CancellationToken, Task> TicketRemoved;

    public MatchMaker(IOptions<MatchmakingOptions> options, ILogger logger)
    {
        _options = options.Value;
        _logger = logger;
    }

    public async Task<ulong> EnqueueAsync(
        uint matchType,
        IReadOnlyList<TMember> members,
        CancellationToken cancellationToken = default)
    {
        if (members == null || members.Count == 0)
        {
            throw new LogicException(ErrorCode.Unhandled);
        }

        if (!Table.Matchmaking.TryGetValue((Fb.Model.EnumValue.MatchType)matchType, out var matchmakingConfig))
        {
            throw new LogicException(ErrorCode.MatchmakingUnknownQueue);
        }

        if (members.Count > matchmakingConfig.MemberCount)
        {
            throw new LogicException(ErrorCode.Unhandled);
        }

        Ticket<TMember> ticket;
        var evicted = new List<Ticket<TMember>>();

        lock (_lock)
        {
            var memberIds = new HashSet<string>();
            foreach (var member in members)
            {
                if (member == null
                    || string.IsNullOrWhiteSpace(member.MemberId)
                    || !double.IsFinite(member.Mu)
                    || !double.IsFinite(member.Sigma)
                    || member.Sigma <= 0)
                {
                    throw new LogicException(ErrorCode.Unhandled);
                }

                if (!memberIds.Add(member.MemberId))
                {
                    throw new LogicException(ErrorCode.MatchmakingDuplicateEntry);
                }

                if (_memberTickets.TryGetValue(member.MemberId, out var active)
                    && active.State == TicketState.Proposed)
                {
                    throw new LogicException(ErrorCode.MatchmakingAlreadyEnrolled);
                }
            }

            // A queued ticket without a live owner is leftover state, so drop it
            // instead of locking the character out of the queue forever.
            foreach (var member in members)
            {
                if (!_memberTickets.TryGetValue(member.MemberId, out var stale))
                {
                    continue;
                }

                _logger.LogWarning(
                    "Enqueue evicted leftover ticket {TicketId} of {MemberId} for match type {MatchType}",
                    stale.TicketId,
                    member.MemberId,
                    stale.MatchType);
                var removed = RemoveTicket(stale.MatchType, stale.TicketId);
                ClearMemberTickets(stale.TicketId);
                if (removed == null)
                {
                    continue;
                }

                // Members re-entering with this request must not be told their new ticket is gone.
                var notified = removed.Members.Where(m => !memberIds.Contains(m.MemberId)).ToList();
                if (notified.Count > 0)
                {
                    evicted.Add(new Ticket<TMember>(removed.Id, removed.MatchType, removed.CreatedAt, notified));
                }
            }

            ticket = new Ticket<TMember>(_ids.Next(0), matchType, DateTime.UtcNow, members.ToList());
            if (!_ticketQueues.TryGetValue(matchType, out var queue))
            {
                queue = new TicketQueue<TMember>(matchmakingConfig, _options);
                _ticketQueues[matchType] = queue;
            }

            queue.Add(ticket);
            foreach (var member in members)
            {
                _memberTickets[member.MemberId] = new TicketRef(TicketState.Queued, matchType, ticket.Id);
            }

            _logger.LogInformation(
                "Enqueued ticket {TicketId} for match type {MatchType} with members {Members}",
                ticket.Id,
                matchType,
                string.Join(",", members.Select(member => member.MemberId)));
            LogQueueState("enqueue");
        }

        foreach (var removed in evicted)
        {
            await RaiseTicketRemovedAsync(removed, cancellationToken);
        }

        return ticket.Id;
    }

    public async Task DequeueAsync(
        uint matchType,
        ulong ticketId,
        string memberId,
        CancellationToken cancellationToken = default)
    {
        DissolvedMatchResult<TMember> dissolved = null;
        Ticket<TMember> removed = null;

        lock (_lock)
        {
            if (!_memberTickets.TryGetValue(memberId, out var ticketRef))
            {
                _logger.LogWarning(
                    "Dequeue from {MemberId} found nothing to remove (requested ticket {TicketId}, match type {MatchType})",
                    memberId,
                    ticketId,
                    matchType);
                throw new LogicException(ErrorCode.MatchmakingRegistryNotFound);
            }

            if (ticketRef.TicketId != ticketId || ticketRef.MatchType != matchType)
            {
                _logger.LogWarning(
                    "Dequeue from {MemberId} referenced {TicketId}/{MatchType} but the live ticket is {LiveTicketId}/{LiveMatchType}",
                    memberId,
                    ticketId,
                    matchType,
                    ticketRef.TicketId,
                    ticketRef.MatchType);
            }

            if (ticketRef.State == TicketState.Proposed && ticketRef.MatchId.HasValue
                && _pendingMatches.TryGetValue(ticketRef.MatchId.Value, out var match))
            {
                dissolved = Dissolve(match, DissolveReason.Decline, memberId);
            }
            else
            {
                removed = RemoveTicket(ticketRef.MatchType, ticketRef.TicketId);
                ClearMemberTickets(ticketRef.TicketId);
                _logger.LogInformation(
                    "Dequeued ticket {TicketId} of match type {MatchType} requested by {MemberId}",
                    ticketRef.TicketId,
                    ticketRef.MatchType,
                    memberId);
                LogQueueState("dequeue");
            }
        }

        if (dissolved != null)
        {
            await RaiseMatchDissolvedAsync(dissolved, cancellationToken);
        }

        if (removed != null)
        {
            await RaiseTicketRemovedAsync(removed, cancellationToken);
        }
    }

    public async Task<bool> ConfirmAsync(ulong matchId, string memberId, CancellationToken cancellationToken = default)
    {
        Match<TMember> readyMatch;

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

            if (!match.AllMemberIds.Contains(memberId))
            {
                throw new LogicException(ErrorCode.MatchmakingNotParticipant);
            }

            if (match.ConfirmedMemberIds.Contains(memberId))
            {
                throw new LogicException(ErrorCode.MatchmakingAlreadyConfirmed);
            }

            match.ConfirmedMemberIds.Add(memberId);
            _logger.LogInformation(
                "Match {MatchId} confirmed by {MemberId} ({Confirmed}/{Total})",
                matchId,
                memberId,
                match.ConfirmedMemberIds.Count,
                match.AllMemberIds.Count());

            if (!match.AllMemberIds.All(id => match.ConfirmedMemberIds.Contains(id)))
            {
                return false;
            }

            readyMatch = MarkReady(match);
        }

        await RaiseMatchReadyAsync(readyMatch, cancellationToken);
        return true;
    }

    public async Task DeclineAsync(ulong matchId, string memberId, CancellationToken cancellationToken = default)
    {
        DissolvedMatchResult<TMember> result;

        lock (_lock)
        {
            if (!_pendingMatches.TryGetValue(matchId, out var match))
            {
                throw new LogicException(ErrorCode.MatchmakingMatchNotFound);
            }

            if (!match.AllMemberIds.Contains(memberId))
            {
                throw new LogicException(ErrorCode.MatchmakingNotParticipant);
            }

            _logger.LogInformation("Match {MatchId} declined by {MemberId}", matchId, memberId);
            result = Dissolve(match, DissolveReason.Decline, memberId);
        }

        await RaiseMatchDissolvedAsync(result, cancellationToken);
    }

    public MatchmakingStatus GetStatus(string memberId)
    {
        lock (_lock)
        {
            if (!_memberTickets.TryGetValue(memberId, out var ticketRef))
            {
                return new MatchmakingStatus();
            }

            if (ticketRef.State == TicketState.Queued)
            {
                return new MatchmakingStatus
                {
                    Queued = true,
                    MatchType = ticketRef.MatchType,
                    TicketId = ticketRef.TicketId
                };
            }

            if (!ticketRef.MatchId.HasValue
                || !_pendingMatches.TryGetValue(ticketRef.MatchId.Value, out var match))
            {
                return new MatchmakingStatus();
            }

            return new MatchmakingStatus
            {
                MatchType = ticketRef.MatchType,
                TicketId = ticketRef.TicketId,
                PendingMatchId = ticketRef.MatchId,
                ConfirmDeadline = GetConfirmDeadline(match)
            };
        }
    }

    public async Task TickMatchmakingAsync(IReadOnlyList<ServerStateService.ServerInfo> hosts, CancellationToken cancellationToken = default)
    {
        List<ProposedMatchResult<TMember>> proposed;

        lock (_lock)
        {
            proposed = new List<ProposedMatchResult<TMember>>();
            foreach (var match in TryFormMatches(hosts))
            {
                foreach (var ticket in match.AllTickets)
                {
                    foreach (var member in ticket.Members)
                    {
                        _memberTickets[member.MemberId] = new TicketRef(
                            TicketState.Proposed,
                            match.Type,
                            ticket.Id,
                            match.Id);
                    }
                }

                _logger.LogInformation(
                    "Proposed match {MatchId} on host {HostId} for match type {MatchType} with tickets {Tickets}",
                    match.Id,
                    SnowflakeId.HostId(match.Id),
                    match.Type,
                    string.Join(",", match.AllTickets.Select(ticket => ticket.Id)));

                proposed.Add(new ProposedMatchResult<TMember>
                {
                    Match = match,
                    ConfirmDeadline = GetConfirmDeadline(match)
                });
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
        List<DissolvedMatchResult<TMember>> dissolved;

        lock (_lock)
        {
            dissolved = new List<DissolvedMatchResult<TMember>>();
            var expiredMatches = _pendingMatches.Values
                .Where(match => IsExpired(match, DateTime.UtcNow))
                .ToList();

            foreach (var match in expiredMatches)
            {
                if (!_pendingMatches.ContainsKey(match.Id))
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
                _ticketQueues.Select(pair => $"{pair.Key}:{pair.Value.Describe()}"));

            _logger.LogInformation(
                "Matchmaking state after {Reason}: queues [{Queues}], pending matches {PendingMatches}, members {Members}",
                reason,
                queues,
                _pendingMatches.Count,
                _memberTickets.Count);
        }
    }

    private async Task RaiseMatchProposedAsync(
        ProposedMatchResult<TMember> args,
        CancellationToken cancellationToken)
    {
        var handler = MatchProposed;
        if (handler == null)
        {
            return;
        }

        var handlers = handler.GetInvocationList()
            .Cast<Func<ProposedMatchResult<TMember>, CancellationToken, Task>>();
        await Task.WhenAll(handlers.Select(h => h(args, cancellationToken)));
    }

    private async Task RaiseMatchReadyAsync(Match<TMember> match, CancellationToken cancellationToken)
    {
        var handler = MatchReady;
        if (handler == null)
        {
            return;
        }

        var handlers = handler.GetInvocationList()
            .Cast<Func<Match<TMember>, CancellationToken, Task>>();
        await Task.WhenAll(handlers.Select(h => h(match, cancellationToken)));
    }

    private async Task RaiseMatchDissolvedAsync(
        DissolvedMatchResult<TMember> result,
        CancellationToken cancellationToken)
    {
        var handler = MatchDissolved;
        if (handler == null)
        {
            return;
        }

        var handlers = handler.GetInvocationList()
            .Cast<Func<DissolvedMatchResult<TMember>, CancellationToken, Task>>();
        await Task.WhenAll(handlers.Select(h => h(result, cancellationToken)));
    }

    private async Task RaiseTicketRemovedAsync(Ticket<TMember> ticket, CancellationToken cancellationToken)
    {
        var handler = TicketRemoved;
        if (handler == null)
        {
            return;
        }

        var handlers = handler.GetInvocationList()
            .Cast<Func<Ticket<TMember>, CancellationToken, Task>>();
        await Task.WhenAll(handlers.Select(h => h(ticket, cancellationToken)));
    }

    private Match<TMember> MarkReady(Match<TMember> match)
    {
        foreach (var ticket in match.AllTickets)
        {
            ClearMemberTickets(ticket.Id);
        }

        _pendingMatches.Remove(match.Id);
        LogQueueState("ready");
        return match;
    }

    private DissolvedMatchResult<TMember> Dissolve(Match<TMember> match, DissolveReason reason, string decliningMemberId = null)
    {
        var outcomes = new List<TicketOutcome<TMember>>();
        foreach (var ticket in match.AllTickets)
        {
            var hasFault = reason == DissolveReason.Decline
                ? ticket.Members.Any(member => member.MemberId == decliningMemberId)
                : ticket.Members.Any(member => !match.ConfirmedMemberIds.Contains(member.MemberId));

            outcomes.Add(new TicketOutcome<TMember>
            {
                TicketId = ticket.Id,
                Requeued = !hasFault,
                Members = ticket.Members.ToList()
            });
        }

        foreach (var (ticket, outcome) in match.AllTickets.Zip(outcomes))
        {
            if (!outcome.Requeued)
            {
                ClearMemberTickets(ticket.Id);
                _logger.LogInformation(
                    "Ticket {TicketId} dropped from match {MatchId} ({Reason})",
                    ticket.Id,
                    match.Id,
                    reason);
            }
            else
            {
                // The ticket keeps its id so game servers never hold a dangling reference.
                _ticketQueues[match.Type].Add(ticket);
                foreach (var member in ticket.Members)
                {
                    _memberTickets[member.MemberId] = new TicketRef(
                        TicketState.Queued,
                        match.Type,
                        ticket.Id);
                }

                _logger.LogInformation(
                    "Ticket {TicketId} requeued after match {MatchId} ({Reason})",
                    ticket.Id,
                    match.Id,
                    reason);
            }
        }

        _pendingMatches.Remove(match.Id);
        LogQueueState("dissolve");
        return new DissolvedMatchResult<TMember>
        {
            Match = match,
            Reason = reason,
            Outcomes = outcomes
        };
    }

    private void ClearMemberTickets(ulong ticketId)
    {
        var memberIds = _memberTickets
            .Where(pair => pair.Value.TicketId == ticketId)
            .Select(pair => pair.Key)
            .ToList();

        foreach (var memberId in memberIds)
        {
            _memberTickets.Remove(memberId);
        }
    }

    private Ticket<TMember> RemoveTicket(uint matchType, ulong ticketId)
    {
        if (!_ticketQueues.TryGetValue(matchType, out var queue))
        {
            return null;
        }

        return queue.Remove(ticketId);
    }

    private List<Match<TMember>> TryFormMatches(IReadOnlyList<ServerStateService.ServerInfo> hosts)
    {
        var results = new List<Match<TMember>>();

        // Without a live cross server nobody could host the match, so tickets stay queued.
        if (hosts.Count == 0)
        {
            return results;
        }

        foreach (var (matchType, queue) in _ticketQueues)
        {
            var createdAt = DateTime.UtcNow;
            foreach (var match in queue.TryFormMatch())
            {
                // The less populated of two random hosts; always taking the emptiest would pull every match there while the count lags.
                ServerStateService.ServerInfo host;
                if (hosts.Count == 1)
                {
                    host = hosts[0];
                }
                else
                {
                    var first = Random.Shared.Next(hosts.Count);
                    var second = Random.Shared.Next(hosts.Count - 1);
                    if (second >= first)
                        second++;
                    host = hosts[second].Online < hosts[first].Online ? hosts[second] : hosts[first];
                }
                match.Id = _ids.Next(host.Id);
                match.Type = matchType;
                match.CreatedAt = createdAt;
                _pendingMatches[match.Id] = match;
                results.Add(match);
            }
        }

        return results;
    }

    private DateTime GetConfirmDeadline(Match<TMember> match)
    {
        return match.CreatedAt.AddSeconds(Math.Max(1, _options.ConfirmTimeoutSeconds));
    }

    private bool IsExpired(Match<TMember> match, DateTime utcNow)
    {
        return utcNow >= GetConfirmDeadline(match);
    }
}
