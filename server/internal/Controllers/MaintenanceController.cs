using Fb.Model.EnumValue;
using Http.Filters;
using Http.Model;
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

        // A home server also receives cross maintenances, since its players enter cross through matchmaking.
        [HttpGet("{world?}")]
        [SuppressRequestLog]
        public async Task<Response.GetMaintenance> GetMaintenance(uint? world)
        {
            try
            {
                var label = world.HasValue ? MaintenanceLabel.World(world.Value) : MaintenanceLabel.Cross;
                var maintenances = await _maintenanceService.GetActiveMaintenances();
                return new Response.GetMaintenance
                {
                    Maintenances = maintenances
                        .Where(x => MaintenanceLabel.Matches(x.Labels, label) || MaintenanceLabel.Matches(x.Labels, MaintenanceLabel.Cross))
                        .Select(x => new Response.ActiveMaintenance
                        {
                            Labels = x.Labels,
                            Message = x.Message ?? string.Empty,
                            EndTime = x.EndTime.ToString("yyyy-MM-dd HH:mm:ss"),
                            FinishMatches = x.FinishMatches
                        })
                        .ToList(),
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
