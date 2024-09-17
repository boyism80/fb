using fb.game.flatbuffer.inter;
using Google.FlatBuffers;
using http.Util;
using Microsoft.AspNetCore.Mvc.Formatters;
using Microsoft.Net.Http.Headers;

namespace http.Formatter
{
    public class FlatBufferInputFormatter : InputFormatter
    {
        const int bufferLength = 16384;

        public FlatBufferInputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/octet-stream"));
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/json"));
        }

        public async override Task<InputFormatterResult> ReadRequestBodyAsync(InputFormatterContext context)
        {
            using var ms = new MemoryStream(bufferLength);
            await context.HttpContext.Request.Body.CopyToAsync(ms);
            ms.Position = 0;

            using var reader = new BinaryReader(ms);
            var protocolType = (FlatBufferProtocolType)reader.ReadInt32().ToMachineEndian();
            var size = reader.ReadInt32().ToMachineEndian();
            var bytes = reader.ReadBytes(size);

            var type = FlatBufferProtocolRouter.GetProtocolType(protocolType);
            var protocol = Activator.CreateInstance(type, bytes);
            return await InputFormatterResult.SuccessAsync(protocol);
        }
    }

    public class FlatBufferOutputFormatter : OutputFormatter
    {
        public FlatBufferOutputFormatter()
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

        public override async Task WriteResponseBodyAsync(OutputFormatterWriteContext context)
        {
            if (context.Object == null)
                throw new InvalidOperationException();

            if (context.Object is IFlatBufferEx protocol)
            {
                using var ms = new MemoryStream();
                using (var writer = new BinaryWriter(ms))
                {
                    var protocolType = FlatBufferProtocolRouter.GetProtocolEnum(protocol);
                    var bytes = protocol.Serialize();

                    writer.Write(((int)protocolType).ToMachineEndian());
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
