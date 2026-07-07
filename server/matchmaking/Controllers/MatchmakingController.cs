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
    private readonly MatchMaker<CharacterRegistryEntry> _matchMaker;

    public MatchmakingController(
        ILogger<MatchmakingController> logger,
        MatchMaker<CharacterRegistryEntry> matchMaker)
    {
        _logger = logger;
        _matchMaker = matchMaker;
    }

    [HttpPost("register")]
    public Response.Register Register(Request.Register request)
    {
        try
        {
            var entries = (request.Entries ?? new List<fb.protocol.matchmaking.RegistryEntry>())
                .Select(CharacterRegistryEntry.FromProtocol)
                .ToList();

            var entryIds = string.Join(",", entries.Select(entry => entry.EntryId));
            _logger.LogInformation(
                "Register request matchType={MatchType} entries=[{EntryIds}]",
                request.MatchType,
                entryIds);

            var registryId = _matchMaker.Enroll(request.MatchType, entries);

            _logger.LogInformation(
                "Register success matchType={MatchType} registryId={RegistryId} entries=[{EntryIds}] queue={QueueSnapshot}",
                request.MatchType,
                registryId,
                entryIds,
                _matchMaker.DescribeQueues());

            return new Response.Register
            {
                RegistryId = registryId.ToString(),
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning(
                "Register rejected matchType={MatchType} error={Error} queue={QueueSnapshot}",
                request.MatchType,
                e.Error,
                _matchMaker.DescribeQueues());

            return new Response.Register
            {
                RegistryId = string.Empty,
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Matchmaking register failed");
            return new Response.Register
            {
                RegistryId = string.Empty,
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }

    [HttpPost("unregister")]
    public async Task<Response.Unregister> Unregister(
        Request.Unregister request,
        CancellationToken cancellationToken)
    {
        try
        {
            if (!Guid.TryParse(request.RegistryId, out var registryId))
            {
                throw new LogicException(ErrorCode.MatchmakingRegistryNotFound);
            }

            var entryId = CharacterRegistryEntry.ToEntryId(request.World, request.CharacterId);
            _logger.LogInformation(
                "Unregister request matchType={MatchType} registryId={RegistryId} entryId={EntryId}",
                request.MatchType,
                registryId,
                entryId);

            await _matchMaker.UnenrollAsync(
                request.MatchType,
                registryId,
                entryId,
                cancellationToken);

            _logger.LogInformation(
                "Unregister success matchType={MatchType} registryId={RegistryId} entryId={EntryId} queue={QueueSnapshot}",
                request.MatchType,
                registryId,
                entryId,
                _matchMaker.DescribeQueues());

            return new Response.Unregister
            {
                Success = true,
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
            _logger.LogWarning(
                "Unregister rejected matchType={MatchType} registryId={RegistryId} characterId={CharacterId} error={Error} queue={QueueSnapshot}",
                request.MatchType,
                request.RegistryId,
                request.CharacterId,
                e.Error,
                _matchMaker.DescribeQueues());

            return new Response.Unregister
            {
                Success = false,
                Error = (uint)e.Error
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Matchmaking unregister failed");
            return new Response.Unregister
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
            if (!Guid.TryParse(request.MatchId, out var matchId))
            {
                throw new LogicException(ErrorCode.MatchmakingMatchNotFound);
            }

            var matchFinalized = await _matchMaker.ConfirmAsync(
                matchId,
                CharacterRegistryEntry.ToEntryId(request.World, request.CharacterId),
                cancellationToken);

            _logger.LogInformation(
                "Confirm success matchId={MatchId} entryId={EntryId} finalized={MatchFinalized} queue={QueueSnapshot}",
                matchId,
                CharacterRegistryEntry.ToEntryId(request.World, request.CharacterId),
                matchFinalized,
                _matchMaker.DescribeQueues());

            return new Response.Confirm
            {
                Error = (uint)ErrorCode.None,
                MatchFinalized = matchFinalized
            };
        }
        catch (LogicException e)
        {
            return new Response.Confirm
            {
                Error = (uint)e.Error,
                MatchFinalized = false
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Matchmaking confirm failed");
            return new Response.Confirm
            {
                Error = (uint)ErrorCode.Unhandled,
                MatchFinalized = false
            };
        }
    }

    [HttpPost("decline")]
    public async Task<Response.Decline> Decline(Request.Decline request, CancellationToken cancellationToken)
    {
        try
        {
            if (!Guid.TryParse(request.MatchId, out var matchId))
            {
                throw new LogicException(ErrorCode.MatchmakingMatchNotFound);
            }

            await _matchMaker.DeclineAsync(
                matchId,
                CharacterRegistryEntry.ToEntryId(request.World, request.CharacterId),
                cancellationToken);

            _logger.LogInformation(
                "Decline success matchId={MatchId} entryId={EntryId} queue={QueueSnapshot}",
                matchId,
                CharacterRegistryEntry.ToEntryId(request.World, request.CharacterId),
                _matchMaker.DescribeQueues());

            return new Response.Decline
            {
                Error = (uint)ErrorCode.None
            };
        }
        catch (LogicException e)
        {
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
                CharacterRegistryEntry.ToEntryId(request.World, request.CharacterId));

            return new Response.Status
            {
                InQueue = status.InQueue,
                MatchType = status.MatchType,
                RegistryId = status.RegistryId == Guid.Empty ? string.Empty : status.RegistryId.ToString(),
                PendingMatchId = status.PendingMatchId?.ToString() ?? string.Empty,
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
