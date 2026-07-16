using AutoMapper;
using Fb.Model.EnumValue;
using Http.Filters;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

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
        public async Task<Response.GetSystemStorageBoxes> GetSystemStorageBoxes(uint world, [FromQuery(Name = "offset")] uint offset = 0)
        {
            try
            {
                var boxes = await _dbContext.SystemStorageBox.GetAll(world, offset);
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

        [HttpPost]
        public async Task<Response.WriteSystemStorageBox> WriteSystemStorageBox(Request.WriteSystemStorageBox request)
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

                var box = await _storageService.CreateSystemStorageAsync(request.World,
                    request.Title,
                    request.Message,
                    expireDate,
                    attachments,
                    externalRef);

                return new Response.WriteSystemStorageBox
                {
                    Box   = _mapper.Map<Protocol.SystemStorageBox>(box),
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

        [HttpPost("deliver")]
        public async Task<Response.DeliverSystemStorage> DeliverSystemStorage(Request.DeliverSystemStorage request)
        {
            try
            {
                var users = request.Users ?? new List<uint>();
                var written = await _storageService.DeliverSystemStorageAsync(request.World,
                    request.SystemStorageBoxId,
                    users,
                    request.Host);

                var entries = written.Select(box => new Protocol.StorageWriteEntry
                {
                    User = box.User,
                    Box  = _mapper.Map<Protocol.StorageBox>(box)
                }).ToList();

                return new Response.DeliverSystemStorage
                {
                    Entries = entries,
                    Host    = request.Host,
                    Error   = (uint)ErrorCode.None
                };
            }
            catch (Exception)
            {
                return new Response.DeliverSystemStorage
                {
                    Host  = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}
