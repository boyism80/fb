namespace Http.Service.Amqp
{
    [AttributeUsage(AttributeTargets.Class, Inherited = false, AllowMultiple = false)]
    public sealed class AmqpHandlerAttribute : Attribute
    {
        public string RouteKey { get; }

        public AmqpHandlerAttribute(string routeKey)
        {
            RouteKey = routeKey ?? throw new ArgumentNullException(nameof(routeKey));
        }
    }
}
