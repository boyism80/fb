using Fb.Model;
using Fb.Model.EnumValue;

namespace http.Worker
{
    public class DataTableLoader : ParallelWorker<Fb.Model.Container>
    {
        private readonly Model _model;
        private readonly ILogger<DataTableLoader> _logger;

        public DataTableLoader(Model model, ILogger<DataTableLoader> logger) : base(logger)
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

        protected override IEnumerable<Container> OnReady()
        {
            foreach (var container in _model.Containers)
            {
                yield return container;
            }
        }

        protected override void OnStart()
        {
            _logger.LogInformation("데이터 테이블 로드를 시작합니다.");
        }

        protected override void OnWork(Container value)
        {
            value.Load();
        }

        protected override void OnWorked(Container input, int percent)
        {
            var attr = input.GetType().GetCustomAttributes(typeof(TableAttribute), true).FirstOrDefault() as TableAttribute;
            if (attr != null)
                _logger.LogInformation($"[{percent,3}%] {attr.Path} 로드 완료 완료");
            base.OnWorked(input, percent);
        }
    }
}
