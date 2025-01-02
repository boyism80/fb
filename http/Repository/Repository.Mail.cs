using Dapper;
using Fb.Model.EnumValue;
using Http.Model;
using Http.Service;
using System.Data;

namespace Http.Reepository
{
    public class MailRepository : IRepository
    {
        private readonly DbContext _dbContext;

        public MailRepository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        public async Task<List<Mail>> GetList(uint user, ushort offset, ushort count)
        {
            await using var conn = _dbContext.Connection(user);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("user", user);
            dynamicParams.Add("position", offset);
            dynamicParams.Add("count", count);
            var mails = await conn.QueryAsync<Http.Model.Mail>($"USP_MAIL_GET_LIST", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);

            if (mails.Any())
            {
                var names = await _dbContext.Character.GetName(mails.Select(x => x.Sender));
                foreach (var mail in mails)
                {
                    if (names.TryGetValue(mail.Sender, out var name))
                        mail.SenderName = name;
                    else
                        mail.SenderName = "Unknown";
                };
            }

            return mails.ToList();
        }

        public async Task<Mail> Get(uint user, uint id)
        {
            await using var conn = _dbContext.Connection(user);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("user", user);
            dynamicParams.Add("id", id);
            var mail = await conn.QueryFirstOrDefaultAsync<Mail>($"USP_MAIL_READ", dynamicParams, commandType: System.Data.CommandType.StoredProcedure) ??
                throw new LogicException(ErrorCode.MailNotExists);

            mail.SenderName = await _dbContext.Character.GetName(mail.Sender) ?? "Unknown";
            return mail;
        }

        public async Task<Mail> Write(string user, uint sender, string title, string contents)
        {
            var uid = await _dbContext.Character.GetCharacterId(user) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);

            var senderName = await _dbContext.Character.GetName(sender) ??
                throw new LogicException(ErrorCode.NotFoundCharacter);

            await using var conn = _dbContext.Connection(uid);
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

            mail.SenderName = senderName;
            return mail;
        }

        public async Task<ushort> Unread(uint user)
        {
            return await _dbContext.Connection(user).QueryFirstOrDefaultAsync<ushort>($"SELECT COUNT(id) FROM mail WHERE user = {user} AND `read` = 0 AND deleted = 0;");
        }

        public async Task Delete(uint user, uint id)
        {
            await _dbContext.Connection(user).ExecuteAsync($"UPDATE `mail` SET deleted = 1 WHERE `user` = {user} AND `id` = {id}");
        }

        public Task SaveChangesAsync()
        {
            return Task.CompletedTask;
        }
    }
}