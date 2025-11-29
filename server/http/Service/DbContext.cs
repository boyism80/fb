using Http.Reepository;
using MySqlConnector;

namespace Http.Service
{
    /// <summary>
    /// Provides database context and repository management for the HTTP service.
    /// Manages database connections, repository instances, and sharding logic.
    /// </summary>
    public class DbContext
    {
        private readonly IConfiguration _configuration;
        private readonly IServiceProvider _serviceProvider;
        private readonly Dictionary<Type, IRepository> _repositories = new Dictionary<Type, IRepository>();

        /// <summary>
        /// Gets the character repository instance.
        /// </summary>
        public CharacterRepository Character => Bind<CharacterRepository>();

        /// <summary>
        /// Gets the item repository instance.
        /// </summary>
        public ItemRepository Item => Bind<ItemRepository>();

        /// <summary>
        /// Gets the spell repository instance.
        /// </summary>
        public SpellRepository Spell => Bind<SpellRepository>();

        /// <summary>
        /// Gets the option repository instance.
        /// </summary>
        public OptionRepository Option => Bind<OptionRepository>();

        /// <summary>
        /// Gets the group repository instance.
        /// </summary>
        public GroupRepository Group => Bind<GroupRepository>();

        /// <summary>
        /// Gets the achievement repository instance.
        /// </summary>
        public AchievementRepository Achievement => Bind<AchievementRepository>();

        /// <summary>
        /// Gets the clan repository instance.
        /// </summary>
        public ClanRepository Clan => Bind<ClanRepository>();

        /// <summary>
        /// Gets the clan member repository instance.
        /// </summary>
        public ClanMemberRepository ClanMember => Bind<ClanMemberRepository>();

        /// <summary>
        /// Gets the character synchronization repository instance.
        /// </summary>
        public CharacterSyncRepository CharacterSync => Bind<CharacterSyncRepository>();

        /// <summary>
        /// Gets the mail repository instance.
        /// </summary>
        public MailRepository Mail => Bind<MailRepository>();

        /// <summary>
        /// Gets the quest repository instance.
        /// </summary>
        public QuestRepository Quest => Bind<QuestRepository>();

        /// <summary>
        /// Gets the system mail repository instance.
        /// </summary>
        public SystemMailRepository SystemMail => Bind<SystemMailRepository>();

        /// <summary>
        /// Gets the system mail user repository instance.
        /// </summary>
        public SystemMailUserRepository SystemMailUser => Bind<SystemMailUserRepository>();

        /// <summary>
        /// Gets the storage box repository instance.
        /// </summary>
        public StorageBoxRepository StorageBox => Bind<StorageBoxRepository>();

        /// <summary>
        /// Gets the storage pending box repository instance.
        /// </summary>
        public StoragePendingBoxRepository StoragePendingBox => Bind<StoragePendingBoxRepository>();

        /// <summary>
        /// Gets the storage reward mark repository instance.
        /// </summary>
        public StorageRewardMarkRepository StorageRewardMark => Bind<StorageRewardMarkRepository>();

        /// <summary>
        /// Gets the ban repository instance.
        /// </summary>
        public BanRepository Ban => Bind<BanRepository>();

        /// <summary>
        /// Gets the number of shared database instances configured.
        /// </summary>
        /// <value>The count of database shards excluding the default (-1) connection.</value>
        public uint SharedDbSize
        {
            get
            {
                var section = _configuration.GetSection("ConnectionStrings:MySql");
                return (uint)section.GetChildren().Where(x => x.Key != "-1").Count();
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="DbContext"/> class.
        /// </summary>
        /// <param name="configuration">The application configuration instance.</param>
        /// <param name="serviceProvider">The service provider for dependency injection.</param>
        public DbContext(IConfiguration configuration,
            IServiceProvider serviceProvider)
        {
            _configuration = configuration;
            _serviceProvider = serviceProvider;
        }

        /// <summary>
        /// Binds and returns a repository instance of the specified type.
        /// Uses singleton pattern to ensure only one instance per repository type.
        /// </summary>
        /// <typeparam name="T">The repository type that implements <see cref="IRepository"/>.</typeparam>
        /// <returns>The repository instance of type T.</returns>
        private T Bind<T>() where T : class, IRepository
        {
            if (_repositories.TryGetValue(typeof(T), out var repository))
                return repository as T;

            repository = ActivatorUtilities.CreateInstance(_serviceProvider, typeof(T)) as T;
            _repositories.Add(typeof(T), repository);
            return repository as T;
        }

        /// <summary>
        /// Creates a MySQL connection for the specified shard ID.
        /// Uses modulo operation to distribute connections across available shards.
        /// </summary>
        /// <param name="id">The ID used to determine the target shard.</param>
        /// <returns>A new <see cref="MySqlConnection"/> instance for the calculated shard.</returns>
        /// <exception cref="Exception">Thrown when SharedDbSize is zero.</exception>
        public MySqlConnection Connection(uint id)
        {
            if (SharedDbSize == 0)
                throw new Exception("shard db size cannot be zero");

            return new MySqlConnection(_configuration.GetConnectionString($"MySql:{id % SharedDbSize}"));
        }

        /// <summary>
        /// Creates multiple MySQL connections grouped by shard for the specified ID list.
        /// Groups IDs by their target shard to optimize database operations.
        /// </summary>
        /// <param name="idList">The collection of IDs to group by shard.</param>
        /// <returns>An enumerable of tuples containing the connection and associated ID array for each shard.</returns>
        /// <exception cref="Exception">Thrown when SharedDbSize is zero.</exception>
        public IEnumerable<(MySqlConnection Connection, uint[] IdList)> Connections(IEnumerable<uint> idList)
        {
            if (SharedDbSize == 0)
                throw new Exception("shard db size cannot be zero");

            foreach (var g in idList.Distinct().GroupBy(id => (int)(id % SharedDbSize)))
            {
                yield return (Connection(g.Key), g.ToArray());
            }
        }

        /// <summary>
        /// Creates multiple MySQL connections grouped by shard for the specified values.
        /// Uses a selector function to extract the sharding key from each value.
        /// </summary>
        /// <typeparam name="T">The type of values to group by shard.</typeparam>
        /// <param name="values">The collection of values to group by shard.</param>
        /// <param name="selector">A function to extract the sharding key from each value.</param>
        /// <returns>An enumerable of tuples containing the connection and associated value array for each shard.</returns>
        /// <exception cref="Exception">Thrown when SharedDbSize is zero.</exception>
        public IEnumerable<(MySqlConnection Connection, T[] Values)> Connections<T>(IEnumerable<T> values, Func<T, uint> selector)
        {
            if (SharedDbSize == 0)
                throw new Exception("shard db size cannot be zero");

            foreach (var g in values.GroupBy(value => (int)(selector(value) % SharedDbSize)))
            {
                yield return (Connection(g.Key), g.ToArray());
            }
        }

        /// <summary>
        /// Creates a MySQL connection for the specified database index.
        /// Directly connects to the database with the given index without sharding logic.
        /// </summary>
        /// <param name="db">The database index to connect to.</param>
        /// <returns>A new <see cref="MySqlConnection"/> instance for the specified database.</returns>
        public MySqlConnection Connection(int db)
        {
            return new MySqlConnection(_configuration.GetConnectionString($"MySql:{db}"));
        }

        /// <summary>
        /// Saves all pending changes across all repository instances asynchronously.
        /// Iterates through all bound repositories and calls their SaveChangesAsync method.
        /// </summary>
        /// <returns>A task representing the asynchronous save operation.</returns>
        public async Task SaveChangesAsync()
        {
            foreach (var repository in _repositories.Values)
            {
                await repository.SaveChangesAsync();
            }
        }
    }
}
