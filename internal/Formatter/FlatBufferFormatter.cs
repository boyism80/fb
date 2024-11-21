using Google.FlatBuffers;
using Request = fb.protocol._internal.request;

namespace Internal.Formatter
{
    public class FlatBufferInputFormatter : Http.Formatter.FlatBufferInputFormatter
    {
        public FlatBufferInputFormatter()
        {

        }

        protected override IFlatBufferEx GetProtocol(BinaryReader reader)
        {
            var protocolType = (Request.FlatBufferProtocolType)reader.ReadInt32();
            var size = reader.ReadInt32();
            var bytes = reader.ReadBytes(size);

            var type = Request.FlatBufferProtocolRouter.GetProtocolType(protocolType);
            return Activator.CreateInstance(type, bytes) as IFlatBufferEx;
        }
    }

    public class FlatBufferOutputFormatter : Http.Formatter.FlatBufferOutputFormatter
    {
        public FlatBufferOutputFormatter()
        {

        }
    }
}
