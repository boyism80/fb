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
                    Home = Window(maintenances, label),
                    Cross = Window(maintenances, MaintenanceLabel.Cross),
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

        private static Response.MaintenanceWindow Window(List<MaintenanceInfo> maintenances, string label)
        {
            var matched = maintenances.Where(x => MaintenanceLabel.Matches(x.Labels, label)).ToList();
            if (matched.Count == 0)
            {
                return null;
            }

            return new Response.MaintenanceWindow
            {
                Message = matched[0].Message ?? string.Empty,
                FinishMatches = matched.All(x => x.FinishMatches)
            };
        }
    }
}
