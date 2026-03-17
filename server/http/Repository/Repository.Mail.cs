using Dapper;
using Fb.Model.EnumValue;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class MailRepository : IRepository
    {
        private readonly DbContext _dbContext;

        public MailRepository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        public async Task<List<Mail>> GetList(uint world, uint user, ushort offset, ushort count)
        {
            await using var conn = _dbContext.GetShardConnection(world, user);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("user", user);
            dynamicParams.Add("position", offset);
            dynamicParams.Add("count", count);
            var mails = await conn.QueryAsync<Http.Model.Mail>($"USP_MAIL_GET_LIST", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);

            return mails.ToList();
        }

        public async Task<Mail> Get(uint world, uint user, uint id)
        {
            await using var conn = _dbContext.GetShardConnection(world, user);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("user", user);
            dynamicParams.Add("id", id);
            var mail = await conn.QueryFirstOrDefaultAsync<Mail>($"USP_MAIL_READ", dynamicParams, commandType: System.Data.CommandType.StoredProcedure) ??
                throw new LogicException(ErrorCode.MailNotExists);

            return mail;
        }

        public async Task<Mail> Write(uint world, string user, uint sender, string title, string contents)
        {
            uint uid;
            if (uint.TryParse(user, out var userId))
            {
                // User ID provided directly (for offline users)
                uid = userId;
            }
            else
            {
                // Character name provided
                uid = await _dbContext.Character.GetCharacterId(world, user) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
            }

            await using var conn = _dbContext.GetShardConnection(world, uid);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("user", uid);
            dynamicParams.Add("sender", sender);
            dynamicParams.Add("title", title);
            dynamicParams.Add("contents", contents);

            await using var reader = await conn.QueryMultipleAsync($"USP_MAIL_WRITE", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);
            var success = await reader.ReadFirstAsync<bool>();
            if (!success)
                throw new LogicException(ErrorCode.Unhandled);

            var mail = await reader.ReadFirstOrDefaultAsync<Http.Model.Mail>() ??
                throw new LogicException(ErrorCode.MailNotExists);

            return mail;
        }

        public async Task<ushort> Unread(uint world, uint user)
        {
            await using var conn = _dbContext.GetShardConnection(world, user);
            return await conn.QueryFirstOrDefaultAsync<ushort>($"SELECT COUNT(id) FROM mail WHERE user = {user} AND `read` = 0 AND deleted = 0;");
        }

        public async Task<bool> Delete(uint world, uint user, uint id)
        {
            await using var conn = _dbContext.GetShardConnection(world, user);
            var affectedRows = await conn.ExecuteAsync($"UPDATE `mail` SET deleted = 1 WHERE `user` = {user} AND `id` = {id} AND `deleted` = 0");
            return affectedRows == 1;
        }

        public Task SaveChangesAsync()
        {
            return Task.CompletedTask;
        }
    }
}