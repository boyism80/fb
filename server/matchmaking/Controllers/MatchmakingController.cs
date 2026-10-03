using Fb.Model.EnumValue;
using Http;
using Matchmaking.Core;
using Matchmaking.Model;
using Matchmaking.Services;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol.matchmaking.request;
using Response = fb.protocol.matchmaking.response;

namespace Matchmaking.Controllers;

[ApiController]
[Route("matchmaking")]
public class MatchmakingController : ControllerBase
{
    private readonly ILogger<MatchmakingController> _logger;
    private readonly MatchMaker<CharacterTicketMember> _matchMaker;

    public MatchmakingController(
        ILogger<MatchmakingController> logger,
        MatchMaker<CharacterTicketMember> matchMaker)
    {
        _logger = logger;
        _matchMaker = matchMaker;
    }

    [HttpPost("enqueue")]
    public async Task<Response.Enqueue> Enqueue(Request.Enqueue request, CancellationToken cancellationToken)
    {
        try
        {
            var members = (request.Members ?? new List<fb.protocol.matchmaking.TicketMember>())
                .Select(CharacterTicketMember.FromProtocol)
                .ToList();

            var ticketId = await _matchMaker.EnqueueAsync(request.MatchType, members, cancellationToken);

            return new Response.Enqueue
            {
                TicketId = ticketId,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning(
                "Matchmaking enqueue rejected for match type {MatchType}: {Error}",
                request.MatchType,
                e.Error);
            return new Response.Enqueue
            {
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Matchmaking enqueue failed");
            return new Response.Enqueue
            {
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("dequeue")]
    public async Task<Response.Dequeue> Dequeue(
        Request.Dequeue request,
        CancellationToken cancellationToken)
    {
        try
        {
            // The ticket id is only a hint; the caller is identified by its member id.
            await _matchMaker.DequeueAsync(
                request.MatchType,
                request.TicketId,
                CharacterTicketMember.ToMemberId(request.World, request.CharacterId),
                cancellationToken);

            return new Response.Dequeue
            {
                Success = true,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning(
                "Matchmaking dequeue rejected for character {World}:{CharacterId}: {Error}",
                request.World,
                request.CharacterId,
                e.Error);
            return new Response.Dequeue
            {
                Success = false,
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Matchmaking dequeue failed");
            return new Response.Dequeue
            {
                Success = false,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("confirm")]
    public async Task<Response.Confirm> Confirm(Request.Confirm request, CancellationToken cancellationToken)
    {
        try
        {
            var matchReady = await _matchMaker.ConfirmAsync(
                request.MatchId,
                CharacterTicketMember.ToMemberId(request.World, request.CharacterId),
                cancellationToken);

            return new Response.Confirm
            {
                Error = (uint)ErrorCode.None,
                MatchReady = matchReady
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning(
                "Matchmaking confirm rejected for character {World}:{CharacterId} on match {MatchId}: {Error}",
                request.World,
                request.CharacterId,
                request.MatchId,
                e.Error);
            return new Response.Confirm
            {
                Error = (uint)e.Error,
                MatchReady = false
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Matchmaking confirm failed");
            return new Response.Confirm
            {
                Error = (uint)ErrorCode.Unhandled,
                MatchReady = false
            };
        }
    }

    [HttpPost("decline")]
    public async Task<Response.Decline> Decline(Request.Decline request, CancellationToken cancellationToken)
    {
        try
        {
            await _matchMaker.DeclineAsync(
                request.MatchId,
                CharacterTicketMember.ToMemberId(request.World, request.CharacterId),
                cancellationToken);

            return new Response.Decline
            {
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning(
                "Matchmaking decline rejected for character {World}:{CharacterId} on match {MatchId}: {Error}",
                request.World,
                request.CharacterId,
                request.MatchId,
                e.Error);
            return new Response.Decline
            {
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Matchmaking decline failed");
            return new Response.Decline
            {
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("status")]
    public Response.Status Status(Request.Status request)
    {
        try
        {
            var status = _matchMaker.GetStatus(
                CharacterTicketMember.ToMemberId(request.World, request.CharacterId));

            return new Response.Status
            {
                Queued = status.Queued,
                MatchType = status.MatchType,
                TicketId = status.TicketId,
                PendingMatchId = status.PendingMatchId ?? 0,
                ConfirmDeadline = status.ConfirmDeadline?.ToString("yyyy-MM-dd HH:mm:ss") ?? string.Empty,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            return new Response.Status
            {
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Matchmaking status failed");
            return new Response.Status
            {
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }
}
