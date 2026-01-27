using Http.Reepository;
using Marketplace.Reepository;
using MySqlConnector;

namespace Marketplace.Service
{
    public class DbContext
    {
        private readonly IConfiguration _configuration;
        private readonly IServiceProvider _serviceProvider;
        private readonly Dictionary<Type, IRepository> _repositories = new Dictionary<Type, IRepository>();

        public MarketplaceRepository Marketplace => Bind<MarketplaceRepository>();

        public MarketplacePurchaseRepository MarketplacePurchase => Bind<MarketplacePurchaseRepository>();

        public DbContext(IConfiguration configuration, IServiceProvider serviceProvider)
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

        /// <summary>
        /// Creates a MySQL connection for the unified-global database.
        /// Marketplace uses unified-global database for all sections.
        /// </summary>
        /// <param name="db">The database index. For marketplace, always use -1 (unified-global).</param>
        /// <returns>A new <see cref="MySqlConnection"/> instance for the unified-global database.</returns>
        public MySqlConnection Connection(int db = -1)
        {
            var connectionString = _configuration.GetConnectionString($"MySql:unified-global:{db}");
            if (string.IsNullOrEmpty(connectionString))
            {
                throw new Exception($"Connection string not found for unified-global database index '{db}'");
            }
            return new MySqlConnection(connectionString);
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