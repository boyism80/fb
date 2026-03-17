using StackExchange.Redis;

namespace Http.Model
{
    public interface IModel
    {
        DateTime CreatedDate { get; set; }

        DateTime UpdatedDate { get; set; }

        bool Deleted { get; set; }
    }

    public interface IModelKey
    {
        uint? GetHash();
    }

    public interface IRedisValueKey : IModelKey
    {
        RedisKey GetRedisKey();
    }

    public interface IRedisHashKey : IRedisValueKey
    {
        RedisValue GetRedisField();
    }

    public abstract class BaseModel : IModel
    {
        protected BaseModel()
        { }

        public DateTime CreatedDate { get; set; } = DateTime.Now;

        public DateTime UpdatedDate { get; set; } = DateTime.Now;

        public bool Deleted { get; set; } = false;
    }
}