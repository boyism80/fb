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

        public MySqlConnection Connection(int db)
        {
            return new MySqlConnection(_configuration.GetConnectionString($"MySql:{db}"));
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