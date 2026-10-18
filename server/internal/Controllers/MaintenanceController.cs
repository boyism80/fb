using Fb.Model.EnumValue;
using Http.Filters;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Response = fb.protocol.@internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("maintenance")]
    public class MaintenanceController : ControllerBase
    {
        private readonly MaintenanceService _maintenanceService;

        public MaintenanceController(MaintenanceService maintenanceService)
        {
            _maintenanceService = maintenanceService;
        }

        [HttpGet("{world}")]
        [SuppressRequestLog]
        public async Task<Response.GetMaintenance> GetMaintenance(uint world)
        {
            try
            {
                var info = await _maintenanceService.GetMaintenanceInfo(world);
                if (info == null || info.IsActive == false)
                {
                    return new Response.GetMaintenance
                    {
                        Active = false,
                        Message = string.Empty,
                        EndTime = string.Empty,
                        Error = (uint)ErrorCode.None
                    };
                }

                return new Response.GetMaintenance
                {
                    Active = true,
                    Message = info.Message ?? string.Empty,
                    EndTime = info.EndTime.ToString("yyyy-MM-dd HH:mm:ss"),
                    Error = (uint)ErrorCode.None
                };
            }
            catch (Exception)
            {
                return new Response.GetMaintenance
                {
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }
    }
}
