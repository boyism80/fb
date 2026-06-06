using Fb.Model;
using Http.Model;
using Http.Service;

namespace Http.Service
{
    public class StorageService
    {
        private readonly DbContext _dbContext;
        private readonly LogService _logService;

        public StorageService(DbContext dbContext, LogService logService)
        {
            _dbContext = dbContext;
            _logService = logService;
        }

        public async Task<SystemStorageBox> CreateSystemStorageAsync(uint world,
            string title,
            string message,
            uint? userId = null,
            DateTime? expiredDate = null,
            List<Dsl> attachments = null,
            string externalRef = null)
        {
            if (string.IsNullOrWhiteSpace(title))
                throw new ArgumentException("Title is required", nameof(title));

            if (string.IsNullOrWhiteSpace(message))
                throw new ArgumentException("Message is required", nameof(message));

            var attachmentsList = attachments ?? new List<Dsl>();
            if (attachmentsList.Count == 0)
                throw new ArgumentException("At least one attachment is required (item, money, or exp)", nameof(attachments));

            var box = await _dbContext.SystemStorageBox.Write(world,
                userId,
                title.Trim(),
                message.Trim(),
                attachmentsList,
                expiredDate,
                externalRef);

            await _logService.WriteAsync("system_storage_box_create", new
            {
                box_id       = box.Id,
                user_id      = box.User,
                external_ref = box.ExternalRef,
                expired_date = expiredDate?.ToString("yyyy-MM-dd HH:mm:ss")
            });

            return box;
        }

        public async Task<SystemStorageBox> CreateSystemStorageAsync(uint world,
            string title,
            string message,
            string userName = null,
            DateTime? expiredDate = null,
            List<Dsl> attachments = null,
            string externalRef = null)
        {
            uint? userId = null;
            if (string.IsNullOrWhiteSpace(userName) == false)
            {
                var id = await _dbContext.Character.GetCharacterId(world, userName.Trim());
                if (id.HasValue == false)
                    throw new KeyNotFoundException($"Character '{userName}' not found.");

                userId = id.Value;
            }

            return await CreateSystemStorageAsync(world, title, message, userId, expiredDate, attachments, externalRef);
        }
    }
}
