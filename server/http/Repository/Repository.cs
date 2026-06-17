using Dapper;
using Http.Model;
using Http.Service;
using StackExchange.Redis;

namespace Http.Reepository
{
    public interface IRepository
    {
        Task SaveChangesAsync();
    }

    public abstract class Repository<TModel, TKey> : IRepository where TModel : class, IModel, TKey where TKey : IModelKey
    {
        protected readonly DbContext _dbContext;

        protected readonly Queue<Func<Task>> _buffer = new Queue<Func<Task>>();

        protected Repository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        protected abstract string OnSelect(TKey key);

        protected virtual string OnSelectBulk(TKey key) { throw new NotImplementedException(); }

        protected abstract string OnUpsert(TModel value);

        protected virtual string OnUpsert(TModel[] values) { throw new NotImplementedException(); }

        protected virtual string OnDelete(TKey key) { throw new NotImplementedException(); }

        protected virtual string OnDeleteMany(IReadOnlyList<TKey> keys) { throw new NotImplementedException(); }

        protected virtual async Task<TModel> Get(uint world, TKey key)
        {
            var hash = key.GetHash();
            await using var conn = hash == null ? _dbContext.GetGlobalConnection(world) : _dbContext.GetShardConnection(world, hash.Value);
            return await conn.QuerySingleOrDefaultAsync<TModel>(OnSelect(key));
        }

        protected virtual async Task<IEnumerable<TModel>> GetAll(uint world, TKey key)
        {
            var hash = key.GetHash();
            await using var conn = hash == null ? _dbContext.GetGlobalConnection(world) : _dbContext.GetShardConnection(world, hash.Value);
            return await conn.QueryAsync<TModel>(OnSelectBulk(key));
        }

        protected virtual string OnSelectMany(IReadOnlyList<TKey> keys) { throw new NotImplementedException(); }

        protected abstract TKey GetKeyFromRow(TModel row);

        protected virtual async Task<IReadOnlyDictionary<TKey, IReadOnlyList<TModel>>> GetManyFromDb(uint world, IReadOnlyList<TKey> keys)
        {
            if (keys == null || keys.Count == 0)
                return new Dictionary<TKey, IReadOnlyList<TModel>>();

            var result = new Dictionary<TKey, List<TModel>>();
            foreach (var (connection, keyArray) in _dbContext.GetShardConnections(world, keys, k => k.GetHash()))
            {
                await using (connection)
                {
                    var sql = OnSelectMany(keyArray);
                    var rows = await connection.QueryAsync<TModel>(sql);
                    foreach (var row in rows)
                    {
                        var key = GetKeyFromRow(row);
                        if (!result.TryGetValue(key, out var list))
                        {
                            list = new List<TModel>();
                            result[key] = list;
                        }
                        list.Add(row);
                    }
                }
            }
            return result.ToDictionary(kv => kv.Key, kv => (IReadOnlyList<TModel>)kv.Value);
        }

        public virtual TModel Set(uint world, TModel value)
        {
            _buffer.Enqueue(async () =>
            {
                var hash = value.GetHash();
                await using var conn = hash == null ? _dbContext.GetGlobalConnection(world) : _dbContext.GetShardConnection(world, hash.Value);
                await conn.ExecuteAsync(OnUpsert(value));
            });
            return value;
        }

        public virtual TModel[] Set(uint world, TModel[] values)
        {
            _buffer.Enqueue(async () =>
            {
                foreach (var (conn, items) in _dbContext.GetShardConnections(world, values, value => value.GetHash()))
                {
                    await conn.ExecuteAsync(OnUpsert(items));
                }
            });

            return values;
        }

        public virtual void Delete(uint world, TKey key)
        {
            _buffer.Enqueue(async () =>
            {
                var hash = key.GetHash();
                await using var conn = hash == null ? _dbContext.GetGlobalConnection(world) : _dbContext.GetShardConnection(world, hash.Value);
                await conn.ExecuteAsync(OnDelete(key));
            });
        }

        public virtual void Delete(uint world, TKey[] keys)
        {
            if (keys == null || keys.Length == 0)
                return;

            _buffer.Enqueue(async () =>
            {
                foreach (var (connection, keyArray) in _dbContext.GetShardConnections(world, keys, k => k.GetHash()))
                {
                    await using (connection)
                    {
                        await connection.ExecuteAsync(OnDeleteMany(keyArray));
                    }
                }
            });
        }

        public async Task SaveChangesAsync()
        {
            while (_buffer.TryDequeue(out var func))
            {
                await func();
            }
        }
    }
}
