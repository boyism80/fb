using System.Net.Http;
using Fb.Model;
using Microsoft.Extensions.Logging;
using Response = fb.protocol._internal.response;

namespace Http.Service.Amqp
{
    [AmqpHandler("fb.global")]
    public sealed class ReloadTablesHandler : AmqpHandler<Response.ReloadTables>
    {
        private readonly ILogger<ReloadTablesHandler> _logger;
        private static readonly HttpClient Http = new HttpClient
        {
            Timeout = TimeSpan.FromSeconds(30)
        };

        public ReloadTablesHandler(ILogger<ReloadTablesHandler> logger)
        {
            _logger = logger;
        }

        protected override async Task HandleAsync(Response.ReloadTables message, CancellationToken cancellationToken)
        {
            if (message.Error != 0)
            {
                _logger.LogWarning("ReloadTables ignored due to error code {Error}", message.Error);
                return;
            }

            try
            {
                if (!string.IsNullOrWhiteSpace(message.Url) &&
                    message.TableNames != null &&
                    message.TableNames.Count > 0)
                {
                    await DownloadTablesAsync(message.Url, message.TableNames, cancellationToken);
                }

                _logger.LogInformation("ReloadTables received; reloading data tables");
                await Table.ReloadAsync(cancellationToken);
                _logger.LogInformation("Data tables reloaded");
            }
            catch (Exception ex)
            {
                // Hosts without loaded tables (e.g. write-back) still receive fb.global.
                _logger.LogError(ex, "Data table reload failed");
            }
        }

        private async Task DownloadTablesAsync(string baseUrl, List<string> tableNames, CancellationToken cancellationToken)
        {
            var baseTrimmed = baseUrl.TrimEnd('/');
            var downloaded = new List<(string Stem, byte[] Bytes)>();

            foreach (var name in tableNames)
            {
                var stem = NormalizeStem(name);
                if (string.IsNullOrEmpty(stem))
                    throw new InvalidOperationException("empty table name in ReloadTables");

                var url = $"{baseTrimmed}/{stem}.json";
                using var response = await Http.GetAsync(url, cancellationToken);
                if (!response.IsSuccessStatusCode)
                {
                    throw new HttpRequestException(
                        $"HTTP GET failed with status {(int)response.StatusCode}: {url}");
                }

                var bytes = await response.Content.ReadAsByteArrayAsync(cancellationToken);
                if (bytes.Length == 0)
                    throw new InvalidOperationException($"downloaded empty body: {url}");

                downloaded.Add((stem, bytes));
            }

            var jsonDir = Path.Combine(Directory.GetCurrentDirectory(), "json");
            Directory.CreateDirectory(jsonDir);

            var tempPaths = new List<string>();
            try
            {
                foreach (var (stem, bytes) in downloaded)
                {
                    var finalPath = Path.Combine(jsonDir, $"{stem}.json");
                    var tempPath = Path.Combine(jsonDir, $"{stem}.json.tmp");
                    await File.WriteAllBytesAsync(tempPath, bytes, cancellationToken);
                    tempPaths.Add(tempPath);
                    File.Move(tempPath, finalPath, overwrite: true);
                }
            }
            catch
            {
                foreach (var temp in tempPaths)
                {
                    try { File.Delete(temp); } catch { /* ignore cleanup errors */ }
                }
                throw;
            }

            _logger.LogInformation("Downloaded {Count} table file(s) from {Url}", downloaded.Count, baseUrl);
        }

        private static string NormalizeStem(string name)
        {
            var stem = name.Trim();
            if (stem.EndsWith(".json", StringComparison.OrdinalIgnoreCase))
                stem = stem[..^5];
            return stem;
        }
    }
}
