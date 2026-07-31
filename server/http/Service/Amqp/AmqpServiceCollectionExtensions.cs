using Microsoft.Extensions.DependencyInjection;

namespace Http.Service.Amqp
{
    public static class AmqpServiceCollectionExtensions
    {
        public static IServiceCollection AddAmqpListener<THandler>(this IServiceCollection services)
            where THandler : class, IAmqpHandler
            => services.AddAmqpListener(registry => registry.Register<THandler>());

        public static IServiceCollection AddAmqpListener<THandler1, THandler2>(this IServiceCollection services)
            where THandler1 : class, IAmqpHandler
            where THandler2 : class, IAmqpHandler
            => services.AddAmqpListener(registry =>
            {
                registry.Register<THandler1>();
                registry.Register<THandler2>();
            });

        public static IServiceCollection AddAmqpListener(this IServiceCollection services, Action<AmqpHandlerRegistry> configure)
        {
            ArgumentNullException.ThrowIfNull(configure);

            var registry = new AmqpHandlerRegistry();
            configure(registry);
            if (registry.Handlers.Count == 0)
                throw new ArgumentException("At least one AMQP handler must be registered", nameof(configure));

            services.AddSingleton(registry);

            foreach (var registration in registry.Handlers)
                services.AddTransient(registration.HandlerType);

            services.AddHostedService<AmqpListenerService>();
            return services;
        }
    }
}
