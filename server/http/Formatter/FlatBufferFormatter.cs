using Google.FlatBuffers;
using Http.Util;
using Microsoft.AspNetCore.Mvc.Formatters;
using Microsoft.Net.Http.Headers;
using Newtonsoft.Json;
using System.Text;

namespace Http.Formatter
{
    /// <summary>
    /// Provides an abstract base class for input formatters that handle FlatBuffer protocol deserialization.
    /// Supports both binary (application/octet-stream) and JSON (application/json) content types.
    /// </summary>
    public abstract class FlatBufferInputFormatter : InputFormatter
    {
        const int bufferLength = 16384;

        /// <summary>
        /// Initializes a new instance of the <see cref="FlatBufferInputFormatter"/> class.
        /// Configures supported media types for FlatBuffer input processing.
        /// </summary>
        protected FlatBufferInputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/octet-stream"));
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/json"));
        }

        /// <summary>
        /// When overridden in a derived class, deserializes a FlatBuffer protocol from binary data.
        /// </summary>
        /// <param name="reader">The binary reader containing the FlatBuffer data.</param>
        /// <returns>The deserialized FlatBuffer protocol object.</returns>
        protected abstract IFlatBufferEx GetProtocol(BinaryReader reader);

        /// <summary>
        /// Asynchronously reads and deserializes the request body into a FlatBuffer protocol object.
        /// Handles both JSON and binary content types with appropriate deserialization logic.
        /// </summary>
        /// <param name="context">The input formatter context containing request information.</param>
        /// <returns>A task representing the asynchronous read operation with the deserialized result.</returns>
        /// <exception cref="InvalidOperationException">Thrown when the content type is not supported.</exception>
        public async override Task<InputFormatterResult> ReadRequestBodyAsync(InputFormatterContext context)
        {
            var sp = context.HttpContext.RequestServices;
            var logger = sp.GetRequiredService<ILogger<FlatBufferInputFormatter>>();

            using (var ms = new MemoryStream(bufferLength))
            {
                await context.HttpContext.Request.Body.CopyToAsync(ms);
                ms.Position = 0;

                switch (context.HttpContext.Request.ContentType)
                {
                    case "application/json":
                        {
                            var protocol = JsonConvert.DeserializeObject(Encoding.UTF8.GetString(ms.ToArray()), context.ModelType) as IFlatBufferEx;
                            var log = OnLog(protocol);
                            if (log != null)
                                logger.LogInformation(log);

                            return await InputFormatterResult.SuccessAsync(protocol);
                        }

                    case "application/octet-stream":
                        {
                            using (var reader = new BinaryReader(ms))
                            {
                                var protocol = GetProtocol(reader);
                                var log = OnLog(protocol);
                                if (log != null)
                                    logger.LogInformation(log);
                                return await InputFormatterResult.SuccessAsync(protocol);
                            }
                        }

                    default:
                        throw new InvalidOperationException();
                }
            }
        }

        /// <summary>
        /// When overridden in a derived class, generates a log message for the processed protocol.
        /// </summary>
        /// <param name="protocol">The FlatBuffer protocol object to generate a log message for.</param>
        /// <returns>A log message string, or null if no logging is required.</returns>
        protected virtual string OnLog(IFlatBufferEx protocol)
        {
            return null;
        }
    }

    /// <summary>
    /// Provides an abstract base class for output formatters that handle FlatBuffer protocol serialization.
    /// Supports both binary (application/octet-stream) and JSON (application/json) content types.
    /// </summary>
    public abstract class FlatBufferOutputFormatter : OutputFormatter
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FlatBufferOutputFormatter"/> class.
        /// Configures supported media types for FlatBuffer output processing.
        /// </summary>
        protected FlatBufferOutputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/octet-stream"));
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/json"));
        }

        /// <summary>
        /// Determines whether this formatter can write the specified type.
        /// </summary>
        /// <param name="type">The type to check for write compatibility.</param>
        /// <returns>True if the type implements IFlatBufferEx; otherwise, false.</returns>
        protected override bool CanWriteType(Type type)
        {
            if (type == null)
                return false;

            return type.IsAssignableTo(typeof(IFlatBufferEx));
        }

        /// <summary>
        /// Asynchronously writes the FlatBuffer protocol object to the response body.
        /// Handles both JSON and binary output formats based on the request content type.
        /// </summary>
        /// <param name="context">The output formatter context containing response information.</param>
        /// <returns>A task representing the asynchronous write operation.</returns>
        /// <exception cref="InvalidOperationException">Thrown when the context object is null or not a FlatBuffer protocol.</exception>
        public override async Task WriteResponseBodyAsync(OutputFormatterWriteContext context)
        {
            if (context.Object == null || context.ObjectType == null)
                throw new InvalidOperationException();

            var protocol = context.Object as IFlatBufferEx ??
                throw new InvalidOperationException();

            var sp = context.HttpContext.RequestServices;
            var logger = sp.GetRequiredService<ILogger<FlatBufferOutputFormatter>>();
            var log = OnLog(protocol);
            if (log != null)
                logger.LogInformation(log);

            switch (context.HttpContext.Request.ContentType)
            {
                case "application/json":
                    context.HttpContext.Response.ContentType = "application/json";
                    await context.HttpContext.Response.WriteAsJsonAsync(protocol, context.ObjectType);
                    break;

                case "application/octet-stream":
                    {

                        var bytes = protocol.ToBytes();
                        context.HttpContext.Response.ContentLength = bytes.Length;
                        await context.HttpContext.Response.BodyWriter.WriteAsync(bytes);
                    }
                    break;
            }
        }

        /// <summary>
        /// When overridden in a derived class, generates a log message for the processed protocol.
        /// </summary>
        /// <param name="protocol">The FlatBuffer protocol object to generate a log message for.</param>
        /// <returns>A log message string, or null if no logging is required.</returns>
        protected virtual string OnLog(IFlatBufferEx protocol)
        {
            return null;
        }
    }
}
