namespace AdminTool.Options
{
    public class TablePublishOptions
    {
        public const string SectionName = "TablePublish";

        public string UploadBaseUrl { get; set; } = string.Empty;
        public string DownloadBaseUrl { get; set; } = string.Empty;
        public string Username { get; set; } = string.Empty;
        public string Password { get; set; } = string.Empty;

        public bool IsConfigured =>
            !string.IsNullOrWhiteSpace(UploadBaseUrl) &&
            !string.IsNullOrWhiteSpace(DownloadBaseUrl) &&
            !string.IsNullOrWhiteSpace(Username) &&
            !string.IsNullOrWhiteSpace(Password);
    }
}
