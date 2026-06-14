using Fb.Model;
using Fb.Model.EnumValue;

namespace Http.Worker
{
    public class DataTableLoader : ParallelWorker<Fb.Model.Container>
    {
        private readonly ILogger<DataTableLoader> _logger;

        public DataTableLoader(ILogger<DataTableLoader> logger) : base(logger)
        {
            _logger = logger;

            Table.Item.Hook.Build = token =>
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

                    case ItemType.Package:
                        return token.ToObject<Pack>();

                    default:
                        return null;
                }
            };
        }

        protected override IEnumerable<Container> OnReady()
        {
            foreach (var container in Table.Containers)
            {
                yield return container;
            }
        }

        protected override void OnStart()
        {
            _logger.LogInformation("Starting data table loading process.");
        }

        protected override void OnWork(Container value)
        {
            value.Load();
        }

        protected override void OnFinish()
        {
            AbilityStackedExp.BuildCache();
            base.OnFinish();
        }

        protected override void OnWorked(Container input, int percent)
        {
            var attr = input.GetType().GetCustomAttributes(typeof(TableAttribute), true).FirstOrDefault() as TableAttribute;
            if (attr != null)
                _logger.LogInformation($"[{percent,3}%] {attr.Path} loading completed");
            base.OnWorked(input, percent);
        }
    }
}