using System.Reflection;

namespace Http.Service.Amqp
{
    public sealed class AmqpHandlerRegistration
    {
        public required Type HandlerType { get; init; }
        public required string RouteKey { get; init; }
        public required int ProtocolType { get; init; }
        public required Type MessageType { get; init; }
    }

    public sealed class AmqpHandlerRegistry
    {
        private readonly Dictionary<(string RouteKey, int ProtocolType), AmqpHandlerRegistration> _handlers = new();

        public IReadOnlyCollection<AmqpHandlerRegistration> Handlers => _handlers.Values;

        public IReadOnlyCollection<string> RouteKeys =>
            _handlers.Keys.Select(k => k.RouteKey).Distinct(StringComparer.Ordinal).ToArray();

        public bool TryGet(string routeKey, int protocolType, out AmqpHandlerRegistration registration)
            => _handlers.TryGetValue((routeKey, protocolType), out registration!);

        public void Register<THandler>() where THandler : class, IAmqpHandler
        {
            var handlerType = typeof(THandler);
            if (handlerType is not { IsClass: true, IsAbstract: false })
                throw new ArgumentException($"{handlerType} must be a concrete class");

            var attr = handlerType.GetCustomAttribute<AmqpHandlerAttribute>()
                       ?? throw new InvalidOperationException($"{handlerType.FullName} is missing [AmqpHandler]");

            var messageType = GetMessageType(handlerType)
                              ?? throw new InvalidOperationException(
                                  $"{handlerType.FullName} does not inherit AmqpHandler<T>");

            var protocolType = CreateProtocolType(messageType);
            var key = (attr.RouteKey, protocolType);
            if (_handlers.ContainsKey(key))
            {
                throw new InvalidOperationException(
                    $"Duplicate AMQP handler for route '{attr.RouteKey}' opcode {protocolType}: " +
                    $"{_handlers[key].HandlerType.Name} and {handlerType.Name}");
            }

            _handlers[key] = new AmqpHandlerRegistration
            {
                HandlerType = handlerType,
                RouteKey = attr.RouteKey,
                ProtocolType = protocolType,
                MessageType = messageType
            };
        }

        private static Type GetMessageType(Type handlerType)
        {
            for (var current = handlerType; current != null; current = current.BaseType)
            {
                if (current.IsGenericType && current.GetGenericTypeDefinition() == typeof(AmqpHandler<>))
                    return current.GetGenericArguments()[0];
            }

            return null;
        }

        private static int CreateProtocolType(Type messageType)
        {
            var instance = Activator.CreateInstance(messageType)
                           ?? throw new InvalidOperationException($"Cannot create instance of {messageType.FullName}");
            var property = messageType.GetProperty(nameof(Google.FlatBuffers.IFlatBufferEx.ProtocolType))
                           ?? throw new InvalidOperationException($"{messageType.FullName} has no ProtocolType");
            return (int)property.GetValue(instance)!;
        }
    }
}
