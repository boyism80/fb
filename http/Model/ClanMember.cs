using StackExchange.Redis;
using System.Text.Json.Serialization;

namespace Http.Model
{
    /// <summary>
    /// Represents a clan member key model that provides Redis hash caching functionality.
    /// Serves as a base class for clan member-related models with Redis integration.
    /// </summary>
    public class ClanMemberKey : BaseModel, IRedisHashKey
    {
        /// <summary>
        /// Gets or sets the unique identifier of the clan this member belongs to.
        /// </summary>
        /// <value>The clan's unique ID that this member is associated with.</value>
        public required uint Clan { get; set; }

        /// <summary>
        /// Gets or sets the unique identifier of the user who is a clan member.
        /// </summary>
        /// <value>The user's unique character ID within the clan.</value>
        public uint User { get; set; }

        /// <summary>
        /// Gets the hash value for Redis sharding based on the clan ID.
        /// </summary>
        /// <returns>The clan ID as the hash value for shard distribution.</returns>
        public uint GetHash() => Clan;

        /// <summary>
        /// Gets the Redis key for caching this clan's member data.
        /// </summary>
        /// <returns>A formatted Redis key string for clan member cache storage.</returns>
        public RedisKey GetRedisKey() => $"cache:clan-member:{Clan}";

        /// <summary>
        /// Gets the Redis field identifier for this specific clan member within the hash.
        /// </summary>
        /// <returns>A formatted Redis field string based on the user ID.</returns>
        public RedisValue GetRedisField() => $"{User}";
    }

    /// <summary>
    /// Represents a complete clan member model with position and key information.
    /// Contains clan member hierarchy data and provides key extraction functionality.
    /// </summary>
    public class ClanMember : ClanMemberKey, IModel
    {
        /// <summary>
        /// Gets or sets the position or rank of this member within the clan.
        /// </summary>
        /// <value>The numeric position value representing the member's rank or role in the clan.</value>
        public uint Position { get; set; }

        /// <summary>
        /// Gets the clan member key information for this member.
        /// </summary>
        /// <value>A ClanMemberKey instance containing the clan and user identification information.</value>
        [JsonIgnore]
        public ClanMemberKey Key => new ClanMemberKey
        {
            Clan = Clan,
            User = User,
        };
    }
}
