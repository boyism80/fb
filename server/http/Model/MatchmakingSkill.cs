using StackExchange.Redis;

namespace Http.Model
{
    public class MatchmakingSkillKey : BaseModel, IRedisHashKey
    {
        public const double DefaultMu = 25.0;

        public const double DefaultSigma = 25.0 / 3.0;

        public required uint CharacterId { get; set; }

        public uint MatchType { get; set; }

        public uint? GetHash() => CharacterId;

        public RedisKey GetRedisKey() => $"fb:cache:matchmaking_skill:{CharacterId}";

        public RedisValue GetRedisField() => $"{MatchType}";
    }

    public class MatchmakingSkill : MatchmakingSkillKey, IModel
    {
        public double Mu { get; set; }

        public double Sigma { get; set; }

        public static MatchmakingSkill CreateDefault(uint characterId, uint matchType)
        {
            return new MatchmakingSkill
            {
                CharacterId = characterId,
                MatchType = matchType,
                Mu = DefaultMu,
                Sigma = DefaultSigma,
                CreatedDate = DateTime.Now,
                UpdatedDate = DateTime.Now
            };
        }
    }
}
