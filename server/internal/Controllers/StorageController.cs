using AutoMapper;
using Fb.Model.EnumValue;
using Http.Model;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Protocol = fb.protocol._internal;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("storage")]
    public class StorageController : ControllerBase
    {
        private readonly ILogger<StorageController> _logger;
        private readonly StorageService _storageService;
        private readonly IMapper _mapper;

        public StorageController(ILogger<StorageController> logger,
            StorageService storageService,
            IMapper mapper)
        {
            _logger = logger;
            _storageService = storageService;
            _mapper = mapper;
        }

        [HttpGet("pending/{world}")]
        public async Task<Response.GetStoragePending> GetPending(uint world)
        {
            try
            {
                var pending = await _storageService.GetGlobalPendingAsync(world);
                return BuildResponse(null, pending);
            }
            catch (Exception e)
            {
                _logger.LogError(e, "storage pending fetch failed");
                return ErrorResponse();
            }
        }

        [HttpGet("pending/{world}/{user}")]
        public async Task<Response.GetStoragePending> GetPendingByUser(uint world, uint user)
        {
            try
            {
                var pending = await _storageService.GetPendingForUserAsync(world, user);
                return BuildResponse(user, pending);
            }
            catch (Exception e)
            {
                _logger.LogError(e, "storage pending fetch failed for user {User}", user);
                return ErrorResponse();
            }
        }

        private Response.GetStoragePending BuildResponse(uint? user, IEnumerable<StoragePendingBox> pending)
        {
            return new Response.GetStoragePending
            {
                User = user,
                Pending = pending.Select(_mapper.Map<Protocol.StoragePendingBox>).ToList(),
                Error = (uint)ErrorCode.None
            };
        }

        private static Response.GetStoragePending ErrorResponse()
        {
            return new Response.GetStoragePending
            {
                Error = (uint)ErrorCode.Unhandled
            };
        }
    }
}
