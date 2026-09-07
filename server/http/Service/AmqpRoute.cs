using Http.Model.Redis;

namespace Http.Service
{
    public static class AmqpRoute
    {
        public const string Exchange = "amq.direct";
        public const string Cluster = "fb.global";

        public static string Scope(uint? world) =>
            world.HasValue ? $"w{world.Value}" : "cross";

        public static string Key(string topic, string scope, uint host = 0) =>
            topic == "game" ? $"fb.game.{scope}.{host}" : $"fb.{topic}.{scope}";

        public static string Unicast(Session session) =>
            Key("game", Scope(session?.World), session.Host);

        public static string Unicast(uint? world, uint host) =>
            Key("game", Scope(world), host);

        public static string[] Fanout(string topic, uint homeWorld) =>
            new[] { Key(topic, $"w{homeWorld}"), Key(topic, "cross") };

        public static string Home(string topic, uint? world) =>
            Key(topic, Scope(world));
    }
}
