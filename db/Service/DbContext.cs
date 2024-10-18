using MySqlConnector;

namespace Db.Service
{
    public class DbContext
    {
        private readonly IConfiguration _configuration;

        public uint SharedDbSize
        {
            get
            {
                var section = _configuration.GetSection("ConnectionStrings:MySql");
                return (uint)section.GetChildren().Where(x => x.Key != "-1").Count();
            }
        }

        public DbContext(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        public MySqlConnection Connection(uint id)
        {
            if (SharedDbSize == 0)
                throw new Exception("shard db size cannot be zero");

            return new MySqlConnection(_configuration.GetConnectionString($"MySql:{id % SharedDbSize}"));
        }

        public MySqlConnection Connection(int db)
        {
            return new MySqlConnection(_configuration.GetConnectionString($"MySql:{db}"));
        }
    }
}
