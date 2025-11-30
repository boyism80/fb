using System.IO.Compression;
using System.Text;
using System.Text.Json;
using Microsoft.AspNetCore.Mvc.Formatters;
using Microsoft.Net.Http.Headers;

namespace Log.Formatter
{
    /// <summary>
    /// Provides an input formatter for handling deflate-compressed binary data (application/octet-stream).
    /// Decompresses raw deflate data (RFC 1951) and decodes UTF-8 to extract JSON log entries.
    /// </summary>
    public class OctetStreamInputFormatter : InputFormatter
    {
        private const int BufferLength = 16384;

        /// <summary>
        /// Initializes a new instance of the <see cref="OctetStreamInputFormatter"/> class.
        /// Configures supported media type for octet-stream input processing.
        /// </summary>
        public OctetStreamInputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/octet-stream"));
        }

        /// <summary>
        /// Determines whether this formatter can read the specified type.
        /// </summary>
        /// <param name="type">The type to check for read compatibility.</param>
        /// <returns>True if the type is JsonElement or JsonElement array; otherwise, false.</returns>
        protected override bool CanReadType(Type type)
        {
            if (type == null)
                return false;

            return type == typeof(JsonElement) || type == typeof(JsonElement[]);
        }

        /// <summary>
        /// Asynchronously reads and deserializes the request body from deflate-compressed binary data.
        /// Decompresses the raw deflate stream (RFC 1951), decodes UTF-8, and parses JSON into JsonElement array.
        /// </summary>
        /// <param name="context">The input formatter context containing request information.</param>
        /// <returns>A task representing the asynchronous read operation with the deserialized JSON result.</returns>
        /// <exception cref="InvalidOperationException">Thrown when decompression or JSON parsing fails.</exception>
        public override async Task<InputFormatterResult> ReadRequestBodyAsync(InputFormatterContext context)
        {
            var sp = context.HttpContext.RequestServices;
            var logger = sp.GetRequiredService<ILogger<OctetStreamInputFormatter>>();

            try
            {
                using (var ms = new MemoryStream(BufferLength))
                {
                    await context.HttpContext.Request.Body.CopyToAsync(ms);
                    ms.Position = 0;

                    // Decompress zlib format (compress2 generates zlib format, not raw deflate)
                    // zlib format: 2-byte header + deflate data + 4-byte ADLER32 checksum
                    // .NET DeflateStream expects raw deflate (RFC 1951), so we need to skip zlib header
                    byte[] decompressedData;
                    var compressedData = ms.ToArray();
                    
                    // Skip zlib header (2 bytes) and checksum (4 bytes at the end)
                    if (compressedData.Length < 6)
                    {
                        throw new InvalidDataException("Compressed data too short for zlib format");
                    }
                    
                    // Extract raw deflate data (skip 2-byte header, remove 4-byte checksum)
                    var rawDeflateData = new byte[compressedData.Length - 6];
                    Array.Copy(compressedData, 2, rawDeflateData, 0, rawDeflateData.Length);
                    
                    using (var rawDeflateMs = new MemoryStream(rawDeflateData))
                    using (var deflateStream = new DeflateStream(rawDeflateMs, CompressionMode.Decompress))
                    using (var decompressedMs = new MemoryStream())
                    {
                        await deflateStream.CopyToAsync(decompressedMs);
                        decompressedData = decompressedMs.ToArray();
                    }

                    // Decode UTF-8
                    var jsonString = Encoding.UTF8.GetString(decompressedData);

                    // Parse JSON
                    using (var doc = JsonDocument.Parse(jsonString))
                    {
                        if (doc.RootElement.ValueKind == JsonValueKind.Array)
                        {
                            var elements = doc.RootElement.EnumerateArray().Select(e => e.Clone()).ToArray();
                            return await InputFormatterResult.SuccessAsync(elements);
                        }
                        else
                        {
                            var element = doc.RootElement.Clone();
                            return await InputFormatterResult.SuccessAsync(element);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                logger.LogError(ex, "Failed to process octet-stream input");
                return await InputFormatterResult.FailureAsync();
            }
        }
    }
}

