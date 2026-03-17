using Google.FlatBuffers;
using Http.Util;
using Microsoft.AspNetCore.Mvc.Formatters;
using Microsoft.Net.Http.Headers;
using Newtonsoft.Json;
using System.Text;

namespace Http.Formatter
{
    public abstract class FlatBufferInputFormatter : InputFormatter
    {
        const int bufferLength = 16384;

        protected FlatBufferInputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/octet-stream"));
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/json"));
        }

        protected abstract IFlatBufferEx GetProtocol(BinaryReader reader);

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

        protected virtual string OnLog(IFlatBufferEx protocol)
        {
            return null;
        }
    }

    public abstract class FlatBufferOutputFormatter : OutputFormatter
    {
        protected FlatBufferOutputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/octet-stream"));
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/json"));
        }

        protected override bool CanWriteType(Type type)
        {
            if (type == null)
                return false;

            return type.IsAssignableTo(typeof(IFlatBufferEx));
        }

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

        protected virtual string OnLog(IFlatBufferEx protocol)
        {
            return null;
        }
    }
}