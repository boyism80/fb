using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;

namespace Http.Migration
{
    public static class DatabaseMigrationHost
    {
        public static async Task RunAsync(
            IServiceProvider services,
            MigrationProfile profile,
            CancellationToken cancellationToken = default)
        {
            var configuration = services.GetRequiredService<IConfiguration>();
            if (!configuration.GetValue<bool>($"{DatabaseMigrationOptions.SectionName}:AutoMigration", false))
            {
                return;
            }

            var logger = services.GetRequiredService<ILoggerFactory>().CreateLogger(typeof(DatabaseMigrationHost));
            logger.LogInformation("Database auto migration is enabled for profile {Profile}.", profile);

            var runner = services.GetRequiredService<DatabaseMigrationRunner>();
            await runner.RunAsync(profile, cancellationToken);
        }
    }

    public static class MigrationServiceCollectionExtensions
    {
        public static IServiceCollection AddDatabaseMigrations(this IServiceCollection services, IConfiguration configuration)
        {
            services.Configure<DatabaseMigrationOptions>(configuration.GetSection(DatabaseMigrationOptions.SectionName));
            services.PostConfigure<DatabaseMigrationOptions>(options =>
            {
                if (string.IsNullOrWhiteSpace(options.AppliedBy))
                {
                    options.AppliedBy = Environment.MachineName;
                }

                var serverName = configuration.GetValue<string>("Log:ServerName");
                if (!string.IsNullOrWhiteSpace(serverName))
                {
                    options.AppliedBy = serverName;
                }
            });

            services.AddSingleton<MigrationScriptProvider>();
            services.AddSingleton<MigrationTargetResolver>();
            services.AddSingleton<DatabaseMigrationRunner>();
            return services;
        }

        public static WebApplicationBuilder AddDatabaseMigrations(this WebApplicationBuilder builder)
        {
            builder.Services.AddDatabaseMigrations(builder.Configuration);
            return builder;
        }
    }
}
