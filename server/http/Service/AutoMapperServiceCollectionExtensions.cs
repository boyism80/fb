using AutoMapper;
using Http.Mapping;
using Microsoft.Extensions.Logging.Abstractions;

namespace Http.Service
{
    public static class AutoMapperServiceCollectionExtensions
    {
        public static IServiceCollection AddHttpAutoMapper(this IServiceCollection services)
        {
            var config = new MapperConfiguration(
                cfg => cfg.AddProfile<HttpModelToProtocolProfile>(),
                NullLoggerFactory.Instance);
            services.AddSingleton<IMapper>(_ => new Mapper(config));
            return services;
        }
    }
}