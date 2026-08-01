using System.Net.Http.Headers;
using System.Text;
using System.Text.RegularExpressions;
using AdminTool.Options;
using Microsoft.Extensions.Options;

namespace AdminTool.Services
{
    public sealed class ScriptPublishService
    {
        private static readonly Regex SafeRelativePath = new(
            @"^(?:(?:spell|npc|item|mob|map|lib|schedule)/[A-Za-z0-9_./-]+\.lua|(?:server|interaction|script|init)\.lua)$",
            RegexOptions.IgnoreCase | RegexOptions.Compiled | RegexOptions.CultureInvariant);

        private readonly ScriptPublishOptions _options;
        private readonly IHttpClientFactory _httpClientFactory;
        private readonly ILogger<ScriptPublishService> _logger;

        public ScriptPublishService(
            IOptions<ScriptPublishOptions> options,
            IHttpClientFactory httpClientFactory,
            ILogger<ScriptPublishService> logger)
        {
            _options = options.Value;
            _httpClientFactory = httpClientFactory;
            _logger = logger;
        }

        public bool IsConfigured => _options.IsConfigured;

        public string DownloadBaseUrl => _options.DownloadBaseUrl.TrimEnd('/');

        public bool IsAllowedRelativePath(string relativePath)
        {
            var path = ScriptDependencyAnalyzer.NormalizeRelativePath(relativePath);
            if (string.IsNullOrEmpty(path))
                return false;
            if (path.Contains("..", StringComparison.Ordinal))
                return false;
            return SafeRelativePath.IsMatch(path);
        }

        public async Task PublishAsync(
            IReadOnlyList<(string RelativePath, Stream Content)> files,
            CancellationToken cancellationToken = default)
        {
            if (!IsConfigured)
            {
                throw new InvalidOperationException(
                    "ScriptPublish 설정이 없습니다. UploadBaseUrl, DownloadBaseUrl, Username, Password를 " +
                    "appsettings.Local.json 또는 환경변수 ScriptPublish__* 로 설정하세요.");
            }

            if (files == null || files.Count == 0)
                throw new InvalidOperationException("업로드할 파일이 없습니다.");

            var uploadBase = _options.UploadBaseUrl.TrimEnd('/');
            var client = _httpClientFactory.CreateClient(nameof(ScriptPublishService));
            var authToken = Convert.ToBase64String(Encoding.UTF8.GetBytes($"{_options.Username}:{_options.Password}"));

            foreach (var (relativePath, content) in files)
            {
                var path = ScriptDependencyAnalyzer.NormalizeRelativePath(relativePath);
                if (!IsAllowedRelativePath(path))
                    throw new InvalidOperationException($"허용되지 않은 스크립트 경로입니다: {relativePath}");

                var bytes = await ReadAllBytesAsync(content, cancellationToken);
                if (bytes.Length == 0)
                    throw new InvalidOperationException($"빈 파일입니다: {path}");

                var url = $"{uploadBase}/{path}";
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
                        $"업로드 실패 ({(int)response.StatusCode}) {path}: {body}");
                }

                _logger.LogInformation("Uploaded script file {Path} to {Url}", path, url);
            }
        }

        private static async Task<byte[]> ReadAllBytesAsync(Stream content, CancellationToken cancellationToken)
        {
            using var ms = new MemoryStream();
            await content.CopyToAsync(ms, cancellationToken);
            return ms.ToArray();
        }
    }
}
