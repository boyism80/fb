using Http.Model.Redis;
using Protocol = fb.protocol._internal;

namespace Http.Service
{
    public static class AmqpRoute
    {
        public const string Exchange = "amq.direct";
        public const string Cluster = "fb.global";

        public static string Name(Protocol.ProcessRole role) =>
            role == Protocol.ProcessRole.Cross ? "cross" : "home";

        public static Protocol.ProcessRole Parse(string role) =>
            role == "cross" ? Protocol.ProcessRole.Cross : Protocol.ProcessRole.Home;

        public static string Scope(Protocol.ProcessRole role, uint world) =>
            role == Protocol.ProcessRole.Cross ? "cross" : $"w{world}";

        public static string Scope(string role, uint world) =>
            Scope(Parse(role), world);

        public static string Key(string topic, string scope, uint host = 0) =>
            topic == "game" ? $"fb.game.{scope}.{host}" : $"fb.{topic}.{scope}";

        public static string Unicast(Session session, uint homeWorld) =>
            Key("game", Scope(session?.Role, homeWorld), session.Host);

        public static string Unicast(Protocol.ProcessRole role, uint homeWorld, uint host) =>
            Key("game", Scope(role, homeWorld), host);

        public static string Unicast(string role, uint homeWorld, uint host) =>
            Unicast(Parse(role), homeWorld, host);

        public static string[] Fanout(string topic, uint homeWorld) =>
            new[] { Key(topic, $"w{homeWorld}"), Key(topic, "cross") };

        public static string Home(string topic, uint world) =>
            Key(topic, $"w{world}");
    }
}
