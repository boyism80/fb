using Http.Model;
using Http.Service;

namespace AdminTool.Services
{
    /// <summary>
    /// Provides maintenance schedule management services for the admin tool.
    /// Wraps Http.Service.MaintenanceService with UI-friendly methods.
    /// </summary>
    public class MaintenanceService
    {
        private readonly Http.Service.MaintenanceService _maintenanceService;
        private readonly ILogger<MaintenanceService> _logger;

        /// <summary>
        /// Initializes a new instance of the <see cref="MaintenanceService"/> class.
        /// </summary>
        /// <param name="maintenanceService">The underlying maintenance service.</param>
        /// <param name="logger">The logger for recording operations.</param>
        public MaintenanceService(
            Http.Service.MaintenanceService maintenanceService,
            ILogger<MaintenanceService> logger)
        {
            _maintenanceService = maintenanceService;
            _logger = logger;
        }

        /// <summary>
        /// Checks if maintenance is currently active for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>True if maintenance is active; otherwise, false.</returns>
        public async Task<bool> IsMaintenanceActive(uint world)
        {
            return await _maintenanceService.IsMaintenanceActive(world);
        }

        /// <summary>
        /// Gets the current maintenance information for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>The maintenance information if active; otherwise, null.</returns>
        public async Task<MaintenanceInfo?> GetMaintenanceInfo(uint world)
        {
            return await _maintenanceService.GetMaintenanceInfo(world);
        }

        /// <summary>
        /// Creates a new maintenance schedule.
        /// </summary>
        /// <param name="schedule">The maintenance schedule to create.</param>
        /// <returns>True if the schedule was created successfully; otherwise, false.</returns>
        public async Task<bool> CreateMaintenanceSchedule(MaintenanceSchedule schedule)
        {
            return await _maintenanceService.CreateMaintenanceSchedule(schedule);
        }

        /// <summary>
        /// Cancels a maintenance schedule.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="scheduleId">The schedule ID to cancel.</param>
        /// <returns>True if the schedule was cancelled successfully; otherwise, false.</returns>
        public async Task<bool> CancelMaintenanceSchedule(uint world, string scheduleId)
        {
            return await _maintenanceService.CancelMaintenanceSchedule(world, scheduleId);
        }

        /// <summary>
        /// Gets all maintenance schedules for the specified world.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <returns>A list of all maintenance schedules for the world.</returns>
        public async Task<List<MaintenanceSchedule>> GetAllSchedules(uint world)
        {
            return await _maintenanceService.GetAllSchedules(world);
        }

        /// <summary>
        /// Applies a maintenance schedule to all worlds.
        /// </summary>
        /// <param name="schedule">The maintenance schedule to apply. World property will be ignored.</param>
        /// <returns>True if the schedule was applied to all worlds successfully; otherwise, false.</returns>
        public async Task<bool> ApplyToAllWorlds(MaintenanceSchedule schedule)
        {
            return await _maintenanceService.ApplyToAllWorlds(schedule);
        }
    }
}
