using Google.FlatBuffers;
using http.Util;
using Request = fb.protocol._internal.request;

namespace Internal.Formatter
{
    public class FlatBufferInputFormatter : http.Formatter.FlatBufferInputFormatter
    {
        public FlatBufferInputFormatter()
        {

        }

        protected override IFlatBufferEx GetProtocol(BinaryReader reader)
        {
            var protocolType = (Request.FlatBufferProtocolType)reader.ReadInt32().ToMachineEndian();
            var size = reader.ReadInt32().ToMachineEndian();
            var bytes = reader.ReadBytes(size);

            var type = Request.FlatBufferProtocolRouter.GetProtocolType(protocolType);
            return Activator.CreateInstance(type, bytes) as IFlatBufferEx;
        }
    }

    public class FlatBufferOutputFormatter : http.Formatter.FlatBufferOutputFormatter
    {
        public FlatBufferOutputFormatter()
        {

        }
    }
}
