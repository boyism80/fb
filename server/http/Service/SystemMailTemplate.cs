namespace Http.Service
{
    public sealed record SystemMailVariable(string Token, string Description);

    public sealed class SystemMailRecipientContext
    {
        public SystemMailRecipientContext(uint userId, string userName, byte level)
        {
            UserId   = userId;
            UserName = userName ?? string.Empty;
            Level    = level;
        }

        public uint UserId { get; }
        public string UserName { get; }
        public byte Level { get; }
    }

    public static class SystemMailTemplate
    {
        public const int MaxTitleLength    = 64;
        public const int MaxContentsLength = 256;

        public static IReadOnlyList<SystemMailVariable> Variables { get; } = new[]
        {
            new SystemMailVariable("USER_NAME", "유저 이름"),
            new SystemMailVariable("LEVEL", "유저 레벨"),
            new SystemMailVariable("USER_ID", "유저 ID"),
        };

        public static (string Title, string Contents) Render(
            string titleTemplate,
            string contentsTemplate,
            SystemMailRecipientContext context)
        {
            var title    = ApplyTokens(titleTemplate, context);
            var contents = ApplyTokens(contentsTemplate, context);

            return (Truncate(title, MaxTitleLength), Truncate(contents, MaxContentsLength));
        }

        private static string ApplyTokens(string template, SystemMailRecipientContext context)
        {
            if (string.IsNullOrEmpty(template))
                return string.Empty;

            var result = template;
            foreach (var variable in Variables)
            {
                var value = variable.Token switch
                {
                    "USER_NAME" => context.UserName,
                    "LEVEL"     => context.Level.ToString(),
                    "USER_ID"   => context.UserId.ToString(),
                    _           => string.Empty
                };

                result = result.Replace($"{{{variable.Token}}}", value);
            }

            return result;
        }

        private static string Truncate(string value, int maxLength)
        {
            if (string.IsNullOrEmpty(value))
                return string.Empty;

            return value.Length <= maxLength ? value : value[..maxLength];
        }
    }
}
