using Google.FlatBuffers;
using Newtonsoft.Json;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

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

        protected override string OnLog(IFlatBufferEx protocol)
        {
            var protocolType = (Request.FlatBufferProtocolType)protocol.ProtocolType;
            switch (protocolType)
            {
                default:
                    return $"Request {protocolType} < {JsonConvert.SerializeObject(protocol)}";
            }
        }
    }

    public class FlatBufferOutputFormatter : Http.Formatter.FlatBufferOutputFormatter
    {
        public FlatBufferOutputFormatter()
        {

        }

        protected override string OnLog(IFlatBufferEx protocol)
        {
            var protocolType = (Response.FlatBufferProtocolType)protocol.ProtocolType;
            switch (protocolType)
            {
                default:
                    return $"Response {protocolType} > {JsonConvert.SerializeObject(protocol)}";
            }
        }
    }
}
