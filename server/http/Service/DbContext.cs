using Http.Reepository;
using MySqlConnector;

namespace Http.Service
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

        public GroupRepository Group => Bind<GroupRepository>();

        public AchievementRepository Achievement => Bind<AchievementRepository>();

        public ClanRepository Clan => Bind<ClanRepository>();

        public ClanMemberRepository ClanMember => Bind<ClanMemberRepository>();

        public CharacterSyncRepository CharacterSync => Bind<CharacterSyncRepository>();

        public MailRepository Mail => Bind<MailRepository>();

        public QuestRepository Quest => Bind<QuestRepository>();

        public SystemMailRepository SystemMail => Bind<SystemMailRepository>();

        public SystemMailUserRepository SystemMailUser => Bind<SystemMailUserRepository>();

        public StorageBoxRepository StorageBox => Bind<StorageBoxRepository>();

        public StoragePendingBoxRepository StoragePendingBox => Bind<StoragePendingBoxRepository>();

        public StorageRewardMarkRepository StorageRewardMark => Bind<StorageRewardMarkRepository>();

        public BanRepository Ban => Bind<BanRepository>();

        public MarriageRepository Marriage => Bind<MarriageRepository>();


        public MySqlConnection GetUnifiedConnection()
        {
            var connectionString = _configuration.GetValue<string>("ConnectionStrings:MySql:unified");
            if (string.IsNullOrEmpty(connectionString))
                throw new Exception("Unified connection string not found");
            return new MySqlConnection(connectionString);
        }

        public MySqlConnection GetGlobalConnection(uint world)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Use GetUnifiedConnection() for unified.");

            var connectionString = _configuration.GetValue<string>($"ConnectionStrings:MySql:worlds:{world}:global");
            if (string.IsNullOrEmpty(connectionString))
                throw new Exception($"World {world} global connection string not found");
            return new MySqlConnection(connectionString);
        }

        public MySqlConnection GetDataConnection(uint world, int index)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Use GetUnifiedConnection() for unified.");

            if (index < 0)
                throw new ArgumentException("Shard index must be >= 0. Use GetWorldGlobalConnection() for world-global.");

            var dataArray = _configuration.GetSection($"ConnectionStrings:MySql:worlds:{world}:data")
                .Get<string[]>();

            if (dataArray == null || index >= dataArray.Length)
                throw new Exception($"Shard index {index} not found for world {world}");

            return new MySqlConnection(dataArray[index]);
        }

        public MySqlConnection GetShardConnection(uint world, uint id)
        {
            var shardSize = GetShardDbSize(world);
            if (shardSize == 0)
                throw new Exception($"No shards available for world {world}");

            var shardIndex = (int)(id % shardSize);
            return GetDataConnection(world, shardIndex);
        }

        public uint GetShardDbSize(uint world)
        {
            if (world == 0)
                throw new ArgumentException("World must be greater than 0. Unified has no shards.");

            var dataArray = _configuration.GetSection($"ConnectionStrings:MySql:worlds:{world}:data")
                .Get<string[]>();

            return dataArray != null ? (uint)dataArray.Length : 0;
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

        public IEnumerable<(MySqlConnection Connection, uint[] IdList)> GetShardConnections(uint world, IEnumerable<uint> idList)
        {
            var shardSize = GetShardDbSize(world);
            if (shardSize == 0)
                throw new Exception($"shard db size cannot be zero for world {world}");

            foreach (var g in idList.Distinct().GroupBy(id => (int)(id % shardSize)))
            {
                yield return (GetDataConnection(world, g.Key), g.ToArray());
            }
        }

        public IEnumerable<(MySqlConnection Connection, uint?[] IdList)> GetShardConnections(uint world, IEnumerable<uint?> idList)
        {
            var shardSize = GetShardDbSize(world);
            if (shardSize == 0)
                throw new Exception($"shard db size cannot be zero for world {world}");

            var groups = idList.Distinct().GroupBy(id =>
            {
                if (id == null)
                    return -1; // Use -1 as marker for world-global
                else
                    return (int)(id.Value % shardSize);
            });
            foreach (var g in groups)
            {
                if (g.Key == -1)
                    yield return (GetGlobalConnection(world), g.ToArray());
                else
                    yield return (GetDataConnection(world, g.Key), g.ToArray());
            }
        }

        public IEnumerable<(MySqlConnection Connection, T[] Values)> GetShardConnections<T>(uint world, IEnumerable<T> values, Func<T, uint?> selector)
        {
            var shardSize = GetShardDbSize(world);
            if (shardSize == 0)
                throw new Exception($"shard db size cannot be zero for world {world}");

            var groups = values.GroupBy(value =>
            {
                var hash = selector(value);
                if (hash == null)
                    return -1; // Use -1 as marker for world-global
                else
                    return (int)(hash.Value % shardSize);
            });

            foreach (var g in groups)
            {
                if (g.Key == -1)
                    yield return (GetGlobalConnection(world), g.ToArray());
                else
                    yield return (GetDataConnection(world, g.Key), g.ToArray());
            }
        }



        public async Task SaveChangesAsync()
        {
            foreach (var repository in _repositories.Values)
            {
                await repository.SaveChangesAsync();
            }
        }
    }
}