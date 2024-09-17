using Google.FlatBuffers;
using http.Util;
using Microsoft.AspNetCore.Mvc.Formatters;
using Microsoft.Net.Http.Headers;

namespace http.Formatter
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
            using var ms = new MemoryStream(bufferLength);
            await context.HttpContext.Request.Body.CopyToAsync(ms);
            ms.Position = 0;

            using var reader = new BinaryReader(ms);
            var protocol = GetProtocol(reader);
            return await InputFormatterResult.SuccessAsync(protocol);
        }
    }

    public abstract class FlatBufferOutputFormatter : OutputFormatter
    {
        protected FlatBufferOutputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/octet-stream"));
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/json"));
        }

        protected override bool CanWriteType(Type? type)
        {
            if (type == null)
                return false;

            return type.IsAssignableTo(typeof(IFlatBufferEx));
        }

        protected abstract int GetProtocolId(IFlatBufferEx protocol);

        public override async Task WriteResponseBodyAsync(OutputFormatterWriteContext context)
        {
            if (context.Object == null)
                throw new InvalidOperationException();

            if (context.Object is IFlatBufferEx protocol)
            {
                using var ms = new MemoryStream();
                using (var writer = new BinaryWriter(ms))
                {
                    var bytes = protocol.Serialize();
                    writer.Write(GetProtocolId(protocol).ToMachineEndian());
                    writer.Write(bytes.Length.ToMachineEndian());
                    writer.Write(bytes);
                    writer.Flush();
                }

                var data = ms.ToArray();
                context.HttpContext.Response.ContentLength = data.Length;
                await context.HttpContext.Response.BodyWriter.WriteAsync(data);
            }
        }
    }
}
