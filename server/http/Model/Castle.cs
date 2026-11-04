using Fb.Model.EnumValue;
using StackExchange.Redis;
using Response = fb.protocol.@internal.response;

namespace Http.Model
{
    public class CastleKey : BaseModel, IRedisValueKey
    {
        public required byte DivineBeast { get; set; }
        // World-global: castle ownership is shared across all game hosts in a world.
        // Clan FK cannot span DBs (clan lives in sharded world-data), so app-layer
        // validation in CastleController is the source of truth.
        public uint? GetHash() => null;

        public RedisKey GetRedisKey() => $"fb:cache:castle:{DivineBeast}";
    }

    public class Castle : CastleKey, IModel
    {
        public uint? OwnerClan { get; set; }
        public ulong? SiegeId { get; set; }
        public ulong? SiegeKey { get; set; }
        public DateTime? SiegeEndAt { get; set; }
        public bool SiegeEnded { get; set; }
        public ulong Revision { get; set; }

        public static readonly TimeSpan SiegeGrace = TimeSpan.FromMinutes(5);

        public static string DistributedLockKey(byte divineBeast)
        {
            return $"fb:lock:castle:{divineBeast}";
        }

        public bool SiegeActive(DateTime now)
        {
            return SiegeId.HasValue && SiegeEnded == false && SiegeEndAt.HasValue && SiegeEndAt.Value + SiegeGrace > now;
        }

        public uint SiegeRemaining(DateTime now)
        {
            if (SiegeEndAt.HasValue == false || SiegeEndAt.Value <= now)
            {
                return 0;
            }

            return (uint)Math.Ceiling((SiegeEndAt.Value - now).TotalSeconds);
        }

        public uint SiegeActiveRemaining(DateTime now)
        {
            if (SiegeActive(now) == false)
            {
                return 0;
            }

            return (uint)Math.Ceiling((SiegeEndAt.Value + SiegeGrace - now).TotalSeconds);
        }

        public Response.UpdatedCastle ToUpdated(uint host, bool started = false)
        {
            var now = DateTime.UtcNow;
            return new Response.UpdatedCastle
            {
                Host = host,
                DivineBeast = DivineBeast,
                OwnerClanId = OwnerClan,
                SiegeId = SiegeId,
                SiegeRemaining = SiegeRemaining(now),
                SiegeActiveRemaining = SiegeActiveRemaining(now),
                SiegeEnded = SiegeEnded,
                SiegeStarted = started,
                Revision = Revision,
                Error = (uint)ErrorCode.None
            };
        }
    }
}
