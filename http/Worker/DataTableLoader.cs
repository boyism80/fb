using Fb.Model;
using Fb.Model.EnumValue;

namespace Http.Worker
{
    /// <summary>
    /// Provides parallel loading functionality for game data tables.
    /// Inherits from ParallelWorker to load multiple data containers concurrently with progress tracking.
    /// </summary>
    public class DataTableLoader : ParallelWorker<Fb.Model.Container>
    {
        private readonly Fb.Model.Model _model;
        private readonly ILogger<DataTableLoader> _logger;

        /// <summary>
        /// Initializes a new instance of the <see cref="DataTableLoader"/> class.
        /// Sets up the item type factory for proper deserialization of different item types.
        /// </summary>
        /// <param name="model">The game model containing data containers to load.</param>
        /// <param name="logger">The logger instance for tracking load progress and errors.</param>
        public DataTableLoader(Fb.Model.Model model, ILogger<DataTableLoader> logger) : base(logger)
        {
            _model = model;
            _logger = logger;

            _model.Item.Hook.Build = token =>
            {
                var type = token["type"].ToObject<ItemType>();
                switch (type)
                {
                    case ItemType.Stuff:
                        return token.ToObject<Item>();

                    case ItemType.Cash:
                        return token.ToObject<Cash>();

                    case ItemType.Consume:
                        return token.ToObject<Consume>();

                    case ItemType.Weapon:
                        return token.ToObject<Weapon>();

                    case ItemType.Armor:
                        return token.ToObject<Armor>();

                    case ItemType.Helmet:
                        return token.ToObject<Helmet>();

                    case ItemType.Ring:
                        return token.ToObject<Ring>();

                    case ItemType.Shield:
                        return token.ToObject<Shield>();

                    case ItemType.Auxiliary:
                        return token.ToObject<Auxiliary>();

                    case ItemType.Bow:
                        return token.ToObject<Bow>();

                    case ItemType.Package:
                        return token.ToObject<Pack>();

                    default:
                        return null;
                }
            };
        }

        /// <summary>
        /// Provides the collection of data containers ready for loading.
        /// </summary>
        /// <returns>An enumerable collection of all data containers in the model.</returns>
        protected override IEnumerable<Container> OnReady()
        {
            foreach (var container in _model.Containers)
            {
                yield return container;
            }
        }

        /// <summary>
        /// Performs initialization before data table loading begins.
        /// Logs the start of the data table loading process.
        /// </summary>
        protected override void OnStart()
        {
            _logger.LogInformation("Starting data table loading process.");
        }

        /// <summary>
        /// Loads a single data container.
        /// </summary>
        /// <param name="value">The data container to load.</param>
        protected override void OnWork(Container value)
        {
            value.Load();
        }

        /// <summary>
        /// Handles completion of a data container load with progress information.
        /// Logs the completion status with the table path and progress percentage.
        /// </summary>
        /// <param name="input">The data container that was loaded.</param>
        /// <param name="percent">The completion percentage (0-100) of all data containers.</param>
        protected override void OnWorked(Container input, int percent)
        {
            var attr = input.GetType().GetCustomAttributes(typeof(TableAttribute), true).FirstOrDefault() as TableAttribute;
            if (attr != null)
                _logger.LogInformation($"[{percent,3}%] {attr.Path} loading completed");
            base.OnWorked(input, percent);
        }
    }
}
