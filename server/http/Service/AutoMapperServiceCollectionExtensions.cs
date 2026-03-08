using AutoMapper;
using Http.Mapping;
using Microsoft.Extensions.DependencyInjection;

namespace Http.Service
{
    /// <summary>
    /// Extension methods for registering HTTP model &lt;-&gt; protocol AutoMapper configuration in DI.
    /// </summary>
    public static class AutoMapperServiceCollectionExtensions
    {
        /// <summary>
        /// Registers the shared Http.Model to fb.protocol._internal mapping profile and adds IMapper to the service collection.
        /// Use this from internal server, admin-tool, or any host that needs to map HTTP models to protocol DTOs.
        /// </summary>
        /// <param name="services">The service collection.</param>
        /// <returns>The service collection for chaining.</returns>
        public static IServiceCollection AddHttpAutoMapper(this IServiceCollection services)
        {
            var config = new MapperConfiguration(cfg =>
            {
                cfg.AddProfile<HttpModelToProtocolProfile>();
            });
            services.AddSingleton<IMapper>(_ => new Mapper(config));
            return services;
        }
    }
}
