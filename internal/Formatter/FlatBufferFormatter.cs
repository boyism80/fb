using http.fb.protocol;
using http.Util;
using request = fb.protocol.inter.request;

namespace Internal.Formatter
{
    public class FlatBufferInputFormatter : http.Formatter.FlatBufferInputFormatter
    {
        public FlatBufferInputFormatter()
        {

        }

        protected override IFlatBufferEx GetProtocol(BinaryReader reader)
        {
            var protocolType = (request.FlatBufferProtocolType)reader.ReadInt32().ToMachineEndian();
            var size = reader.ReadInt32().ToMachineEndian();
            var bytes = reader.ReadBytes(size);

            var type = request.FlatBufferProtocolRouter.GetProtocolType(protocolType);
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
