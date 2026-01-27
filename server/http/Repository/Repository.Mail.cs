using Dapper;
using Fb.Model.EnumValue;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for mail system data management.
    /// Implements direct database operations for mail-related functionality without Redis caching.
    /// </summary>
    public class MailRepository : IRepository
    {
        private readonly DbContext _dbContext;

        /// <summary>
        /// Initializes a new instance of the <see cref="MailRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        public MailRepository(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        /// <summary>
        /// Retrieves a paginated list of mail messages for a specific user.
        /// Automatically resolves sender names and handles pagination.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="user">The unique identifier of the user to retrieve mail for.</param>
        /// <param name="offset">The starting position for pagination (0-based).</param>
        /// <param name="count">The maximum number of mail messages to retrieve.</param>
        /// <returns>A list of mail messages with resolved sender names.</returns>
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

        /// <summary>
        /// Retrieves and marks as read a specific mail message for a user.
        /// Automatically resolves the sender name and updates the read status.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="user">The unique identifier of the user who owns the mail.</param>
        /// <param name="id">The unique identifier of the mail message.</param>
        /// <returns>The mail message with resolved sender name.</returns>
        /// <exception cref="LogicException">Thrown when the mail does not exist.</exception>
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

        /// <summary>
        /// Creates and sends a new mail message to a specified user.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="user">The name of the recipient character or user ID as string.</param>
        /// <param name="sender">The sender's user ID.</param>
        /// <param name="title">The subject/title of the mail message.</param>
        /// <param name="contents">The body content of the mail message.</param>
        /// <returns>The created mail message with resolved sender name.</returns>
        /// <exception cref="LogicException">Thrown when the recipient character is not found, or mail creation fails.</exception>
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

        /// <summary>
        /// Retrieves the count of unread mail messages for a specific user.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="user">The unique identifier of the user to check unread mail for.</param>
        /// <returns>The number of unread mail messages.</returns>
        public async Task<ushort> Unread(uint world, uint user)
        {
            await using var conn = _dbContext.GetShardConnection(world, user);
            return await conn.QueryFirstOrDefaultAsync<ushort>($"SELECT COUNT(id) FROM mail WHERE user = {user} AND `read` = 0 AND deleted = 0;");
        }

        /// <summary>
        /// Soft-deletes a mail message by marking it as deleted.
        /// The mail remains in the database but is hidden from normal queries.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="user">The unique identifier of the user who owns the mail.</param>
        /// <param name="id">The unique identifier of the mail message to delete.</param>
        /// <returns>A task representing the asynchronous delete operation.</returns>
        public async Task<bool> Delete(uint world, uint user, uint id)
        {
            await using var conn = _dbContext.GetShardConnection(world, user);
            var affectedRows = await conn.ExecuteAsync($"UPDATE `mail` SET deleted = 1 WHERE `user` = {user} AND `id` = {id} AND `deleted` = 0");
            return affectedRows == 1;
        }

        /// <summary>
        /// Saves any pending changes to the underlying data store.
        /// This implementation returns a completed task as mail operations are immediately persisted.
        /// </summary>
        /// <returns>A completed task representing the save operation.</returns>
        public Task SaveChangesAsync()
        {
            return Task.CompletedTask;
        }
    }
}