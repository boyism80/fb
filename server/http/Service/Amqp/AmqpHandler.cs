using System.Reflection;
using Google.FlatBuffers;

namespace Http.Service.Amqp
{
    public interface IAmqpHandler
    {
        int ProtocolType { get; }
        string RouteKey { get; }
        Type MessageType { get; }
        Task HandleAsync(IFlatBufferEx message, CancellationToken cancellationToken);
    }

    public abstract class AmqpHandler<T> : IAmqpHandler
        where T : class, IFlatBufferEx, new()
    {
        public int ProtocolType { get; } = new T().ProtocolType;
        public Type MessageType => typeof(T);

        public string RouteKey =>
            GetType().GetCustomAttribute<AmqpHandlerAttribute>()?.RouteKey
            ?? throw new InvalidOperationException($"{GetType().Name} is missing [AmqpHandler]");

        public Task HandleAsync(IFlatBufferEx message, CancellationToken cancellationToken)
            => HandleAsync((T)message, cancellationToken);

        protected abstract Task HandleAsync(T message, CancellationToken cancellationToken);
    }
}
