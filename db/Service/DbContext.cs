using Db.Model;
using Db.Reepository;
using MySqlConnector;

namespace Db.Service
{
    public class DbContext
    {
        private readonly IConfiguration _configuration;
        private readonly IServiceProvider _serviceProvider;
        private readonly Dictionary<Type, IRepository> _repositories = new Dictionary<Type, IRepository>();

        public CharacterRepository Character => Bind<CharacterRepository>();
        public ItemRepository Item => Bind<ItemRepository>();
        public SpellRepository Spell => Bind<SpellRepository>();
        public OptionRepository Option => Bind<OptionRepository>();

        public uint SharedDbSize
        {
            get
            {
                var section = _configuration.GetSection("ConnectionStrings:MySql");
                return (uint)section.GetChildren().Where(x => x.Key != "-1").Count();
            }
        }

        public DbContext(IConfiguration configuration,
            IServiceProvider serviceProvider)
        {
            _configuration = configuration;
            _serviceProvider = serviceProvider;
        }

        private T Bind<T>() where T : class, IRepository
        {
            if (_repositories.TryGetValue(typeof(T), out var repository))
                return repository as T;

            repository = ActivatorUtilities.CreateInstance(_serviceProvider, typeof(T)) as T;
            _repositories.Add(typeof(T), repository);
            return repository as T;
        }

        public MySqlConnection Connection(uint id)
        {
            if (SharedDbSize == 0)
                throw new Exception("shard db size cannot be zero");

            return new MySqlConnection(_configuration.GetConnectionString($"MySql:{id % SharedDbSize}"));
        }

        public IEnumerable<(MySqlConnection Connection, uint[] IdList)> Connections(IEnumerable<uint> idList)
        {
            if (SharedDbSize == 0)
                throw new Exception("shard db size cannot be zero");

            foreach (var g in idList.Distinct().GroupBy(id => (int)(id % SharedDbSize)))
            {
                yield return (Connection(g.Key), g.ToArray());
            }
        }

        public IEnumerable<(MySqlConnection Connection, T[] Values)> Connections<T>(IEnumerable<T> values, Func<T, uint> selector)
        {
            if (SharedDbSize == 0)
                throw new Exception("shard db size cannot be zero");

            foreach (var g in values.GroupBy(value => (int)(selector(value) % SharedDbSize)))
            {
                yield return (Connection(g.Key), g.ToArray());
            }
        }

        public MySqlConnection Connection(int db)
        {
            return new MySqlConnection(_configuration.GetConnectionString($"MySql:{db}"));
        }
    }
}
