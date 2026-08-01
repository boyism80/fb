using System.Net.Http.Headers;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using AdminTool.Options;
using Fb.Model;
using Microsoft.Extensions.Options;

namespace AdminTool.Services
{
    public sealed class TablePublishService
    {
        private static readonly Regex SafeFileName = new(@"^[a-z0-9_]+\.json$", RegexOptions.IgnoreCase | RegexOptions.Compiled);
        private readonly TablePublishOptions _options;
        private readonly IHttpClientFactory _httpClientFactory;
        private readonly ILogger<TablePublishService> _logger;
        private readonly Lazy<HashSet<string>> _allowedStems;

        public TablePublishService(
            IOptions<TablePublishOptions> options,
            IHttpClientFactory httpClientFactory,
            ILogger<TablePublishService> logger)
        {
            _options = options.Value;
            _httpClientFactory = httpClientFactory;
            _logger = logger;
            _allowedStems = new Lazy<HashSet<string>>(DiscoverAllowedStems);
        }

        public IReadOnlyCollection<string> AllowedStems => _allowedStems.Value;

        public bool IsConfigured => _options.IsConfigured;

        public bool IsAllowedFileName(string fileName)
        {
            if (string.IsNullOrWhiteSpace(fileName) || !SafeFileName.IsMatch(Path.GetFileName(fileName)))
                return false;

            var stem = NormalizeStem(fileName);
            return _allowedStems.Value.Contains(stem);
        }

        public async Task PublishAsync(IReadOnlyList<(string FileName, Stream Content)> files, CancellationToken cancellationToken = default)
        {
            if (!IsConfigured)
            {
                throw new InvalidOperationException(
                    "TablePublish 설정이 없습니다. UploadBaseUrl, DownloadBaseUrl, Username, Password를 " +
                    "appsettings.Local.json 또는 환경변수 TablePublish__* 로 설정하세요.");
            }

            if (files == null || files.Count == 0)
                throw new InvalidOperationException("업로드할 파일이 없습니다.");

            var uploadBase = _options.UploadBaseUrl.TrimEnd('/');
            var client = _httpClientFactory.CreateClient(nameof(TablePublishService));
            var authToken = Convert.ToBase64String(Encoding.UTF8.GetBytes($"{_options.Username}:{_options.Password}"));

            foreach (var (fileName, content) in files)
            {
                var name = Path.GetFileName(fileName);
                if (!IsAllowedFileName(name))
                    throw new InvalidOperationException($"허용되지 않은 테이블 파일입니다: {name}");

                var stem = NormalizeStem(name);
                var bytes = await ReadAllBytesAsync(content, cancellationToken);
                if (bytes.Length == 0)
                    throw new InvalidOperationException($"빈 파일입니다: {name}");

                var url = $"{uploadBase}/{stem}.json";
                using var request = new HttpRequestMessage(HttpMethod.Put, url)
                {
                    Content = new ByteArrayContent(bytes)
                };
                request.Content.Headers.ContentType = new MediaTypeHeaderValue("application/octet-stream");
                request.Headers.Authorization = new AuthenticationHeaderValue("Basic", authToken);

                using var response = await client.SendAsync(request, cancellationToken);
                if (!response.IsSuccessStatusCode)
                {
                    var body = await response.Content.ReadAsStringAsync(cancellationToken);
                    throw new HttpRequestException(
                        $"업로드 실패 ({(int)response.StatusCode}) {name}: {body}");
                }

                _logger.LogInformation("Uploaded table file {File} to {Url}", name, url);
            }
        }

        public string DownloadBaseUrl => _options.DownloadBaseUrl.TrimEnd('/');

        public static string NormalizeStem(string fileName)
        {
            var stem = Path.GetFileName(fileName).Trim();
            if (stem.EndsWith(".json", StringComparison.OrdinalIgnoreCase))
                stem = stem[..^5];
            return stem.ToLowerInvariant();
        }

        private static async Task<byte[]> ReadAllBytesAsync(Stream content, CancellationToken cancellationToken)
        {
            using var ms = new MemoryStream();
            await content.CopyToAsync(ms, cancellationToken);
            return ms.ToArray();
        }

        private static HashSet<string> DiscoverAllowedStems()
        {
            var stems = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (var type in typeof(Table).GetNestedTypes(BindingFlags.Public | BindingFlags.NonPublic))
            {
                var attr = type.GetCustomAttribute<TableAttribute>(true);
                if (attr?.Path == null)
                    continue;

                var fileName = Path.GetFileName(attr.Path);
                if (string.IsNullOrEmpty(fileName))
                    continue;

                stems.Add(NormalizeStem(fileName));
            }

            return stems;
        }
    }
}
