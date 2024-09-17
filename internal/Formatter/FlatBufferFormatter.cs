using fb.game.flatbuffer.inter;
using Google.FlatBuffers;
using http.Util;

namespace Internal.Formatter
{
    public class FlatBufferInputFormatter : http.Formatter.FlatBufferInputFormatter
    {
        public FlatBufferInputFormatter()
        {

        }

        protected override IFlatBufferEx GetProtocol(BinaryReader reader)
        {
            var protocolType = (FlatBufferProtocolType)reader.ReadInt32().ToMachineEndian();
            var size = reader.ReadInt32().ToMachineEndian();
            var bytes = reader.ReadBytes(size);

            var type = FlatBufferProtocolRouter.GetProtocolType(protocolType);
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
            return (int)FlatBufferProtocolRouter.GetProtocolEnum(protocol);
        }
    }
}
