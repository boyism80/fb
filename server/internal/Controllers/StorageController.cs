using AutoMapper;
using Fb.Model.EnumValue;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("storage")]
    public class StorageController : ControllerBase
    {
        private readonly DbContext _dbContext;
        private readonly StorageService _storageService;
        private readonly IMapper _mapper;

        public StorageController(DbContext dbContext, StorageService storageService, IMapper mapper)
        {
            _dbContext = dbContext;
            _storageService = storageService;
            _mapper = mapper;
        }

        [HttpPost("write")]
        public async Task<Response.WriteStorageBox> WriteStorageBox(Request.WriteStorageBox request)
        {
            try
            {
                DateTime? expireDate = null;
                if (!string.IsNullOrEmpty(request.ExpireDate) && DateTime.TryParse(request.ExpireDate, out var parsedDate))
                    expireDate = parsedDate;

                var attachments = string.IsNullOrWhiteSpace(request.Attachments)
                    ? new List<Fb.Model.Dsl>()
                    : (JsonConvert.DeserializeObject<List<Fb.Model.Dsl>>(request.Attachments) ?? new List<Fb.Model.Dsl>());

                var externalRef = string.IsNullOrWhiteSpace(request.ExternalRef) ? null : request.ExternalRef;

                uint userId = request.User;
                if (!string.IsNullOrWhiteSpace(request.Name))
                {
                    var characterId = await _dbContext.Character.GetCharacterId(request.World, request.Name.Trim());
                    if (characterId.HasValue == false)
                    {
                        return new Response.WriteStorageBox
                        {
                            Host  = request.Host,
                            Error = (uint)ErrorCode.NotFoundCharacter
                        };
                    }

                    userId = characterId.Value;
                }

                if (userId == 0)
                {
                    return new Response.WriteStorageBox
                    {
                        Host  = request.Host,
                        Error = (uint)ErrorCode.Unhandled
                    };
                }

                var box = await _storageService.CreateStorageBoxAsync(request.World,
                    userId,
                    request.Title,
                    request.Message,
                    attachments,
                    expireDate,
                    externalRef,
                    request.Host);

                return new Response.WriteStorageBox
                {
                    Box   = _mapper.Map<Protocol.StorageBox>(box),
                    Host  = request.Host,
                    Error = (uint)ErrorCode.None
                };
            }
            catch (Exception)
            {
                return new Response.WriteStorageBox
                {
                    Host  = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost("claim")]
        public async Task<Response.ClaimStorageBox> ClaimStorageBox(Request.ClaimStorageBox request)
        {
            try
            {
                var ok = await _storageService.ClaimAsync(request.World, request.User, request.Id);
                return new Response.ClaimStorageBox
                {
                    Error = ok ? (uint)ErrorCode.None : (uint)ErrorCode.NotFoundMail
                };
            }
            catch (Exception)
            {
                return new Response.ClaimStorageBox
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost("unclaim")]
        public async Task<Response.UnclaimStorageBox> UnclaimStorageBox(Request.UnclaimStorageBox request)
        {
            try
            {
                var ok = await _storageService.UnclaimAsync(request.World, request.User, request.Id);
                return new Response.UnclaimStorageBox
                {
                    Error = ok ? (uint)ErrorCode.None : (uint)ErrorCode.NotFoundMail
                };
            }
            catch (Exception)
            {
                return new Response.UnclaimStorageBox
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}
