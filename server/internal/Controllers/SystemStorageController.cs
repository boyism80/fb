using AutoMapper;
using Fb.Model.EnumValue;
using Http.Filters;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using Protocol = fb.protocol.@internal;
using Request = fb.protocol.@internal.request;
using Response = fb.protocol.@internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("storage/system")]
    public class SystemStorageController : ControllerBase
    {
        private readonly DbContext _dbContext;
        private readonly StorageService _storageService;
        private readonly IMapper _mapper;

        public SystemStorageController(DbContext dbContext, StorageService storageService, IMapper mapper)
        {
            _dbContext = dbContext;
            _storageService = storageService;
            _mapper = mapper;
        }

        [HttpGet("{world}")]
        [SuppressRequestLog]
        public async Task<Response.GetSystemStorageBoxes> GetSystemStorageBoxes(uint world)
        {
            try
            {
                var boxes = await _dbContext.SystemStorageBox.GetActive(world);
                var protocolBoxes = boxes.Select(_mapper.Map<Protocol.SystemStorageBox>).ToList();

                return new Response.GetSystemStorageBoxes
                {
                    Boxes = protocolBoxes,
                    Error = (uint)ErrorCode.None
                };
            }
            catch (Exception)
            {
                return new Response.GetSystemStorageBoxes
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpGet("{world}/{uid}")]
        [SuppressRequestLog]
        public async Task<Response.GetSystemStorageBoxes> GetSystemStorageBoxes(uint world, uint uid)
        {
            try
            {
                var boxes = await _dbContext.SystemStorageBox.GetAll(world, uid);
                return new Response.GetSystemStorageBoxes
                {
                    Boxes = boxes.Select(_mapper.Map<Protocol.SystemStorageBox>).ToList(),
                    Error = (uint)ErrorCode.None
                };
            }
            catch (Exception)
            {
                return new Response.GetSystemStorageBoxes
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        [HttpPost]
        public async Task<Response.WriteSystemStorageBox> WriteSystemStorageBox(Request.WriteSystemStorageBox request)
        {
            try
            {
                uint? user = null;
                if (string.IsNullOrWhiteSpace(request.Name) == false)
                {
                    user = await _dbContext.Character.GetCharacterId(request.World, request.Name.Trim());
                    if (user.HasValue == false)
                    {
                        return new Response.WriteSystemStorageBox
                        {
                            Error = (uint)ErrorCode.NotFoundCharacter
                        };
                    }
                }

                DateTime? expireDate = null;
                if (!string.IsNullOrEmpty(request.ExpireDate))
                {
                    if (DateTime.TryParse(request.ExpireDate, out var parsedDate) == false)
                    {
                        return new Response.WriteSystemStorageBox
                        {
                            Error = (uint)ErrorCode.Unhandled
                        };
                    }
                    expireDate = parsedDate;
                }

                var attachments = string.IsNullOrWhiteSpace(request.Attachments)
                    ? new List<Fb.Model.Dsl>()
                    : (JsonConvert.DeserializeObject<List<Fb.Model.Dsl>>(request.Attachments) ?? new List<Fb.Model.Dsl>());

                var box = await _storageService.CreateSystemStorageAsync(request.World,
                    request.ExternalRef,
                    request.Title,
                    request.Message,
                    attachments,
                    expireDate,
                    user: user);

                return new Response.WriteSystemStorageBox
                {
                    Box = _mapper.Map<Protocol.SystemStorageBox>(box),
                    Error = (uint)ErrorCode.None
                };
            }
            catch (Exception)
            {
                return new Response.WriteSystemStorageBox
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}
