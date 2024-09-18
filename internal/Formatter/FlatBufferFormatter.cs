using Google.FlatBuffers;
using http.Util;
using request = fb.protocol.inter.request;
using response = fb.protocol.inter.response;

namespace fb.protocol.flatbuffer.inter
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

        protected override int GetProtocolId(IFlatBufferEx protocol)
        {
            return (int)response.FlatBufferProtocolRouter.GetProtocolEnum(protocol);
        }
    }
}
