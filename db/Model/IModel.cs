using StackExchange.Redis;

namespace Db.Model
{
    public interface IModel
    { }

    public interface IModelKey
    {
        uint GetDbKey();
    }

    public interface IRedisCacheKey : IModelKey
    {
        RedisKey GetRedisKey();
        RedisValue GetRedisField();
    }
}