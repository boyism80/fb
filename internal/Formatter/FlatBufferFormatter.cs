using Google.FlatBuffers;
using Newtonsoft.Json;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Formatter
{
    /// <summary>
    /// Provides FlatBuffer input formatting for internal API requests.
    /// Handles deserialization of FlatBuffer protocol messages for internal communication.
    /// </summary>
    public class FlatBufferInputFormatter : Http.Formatter.FlatBufferInputFormatter
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FlatBufferInputFormatter"/> class.
        /// </summary>
        public FlatBufferInputFormatter()
        {

        }

        /// <summary>
        /// Deserializes a FlatBuffer protocol message from the binary reader.
        /// Reads the protocol type, size, and data to create the appropriate protocol instance.
        /// </summary>
        /// <param name="reader">The binary reader containing the FlatBuffer data.</param>
        /// <returns>A deserialized FlatBuffer protocol instance.</returns>
        protected override IFlatBufferEx GetProtocol(BinaryReader reader)
        {
            var protocolType = (Request.FlatBufferProtocolType)reader.ReadInt32();
            var size = reader.ReadInt32();
            var bytes = reader.ReadBytes(size);

            var type = Request.FlatBufferProtocolRouter.GetProtocolType(protocolType);
            return Activator.CreateInstance(type, bytes) as IFlatBufferEx;
        }

        /// <summary>
        /// Generates a log message for the incoming FlatBuffer protocol.
        /// Creates a formatted log entry with protocol type and serialized content.
        /// </summary>
        /// <param name="protocol">The FlatBuffer protocol to log.</param>
        /// <returns>A formatted log message string for the request.</returns>
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

    /// <summary>
    /// Provides FlatBuffer output formatting for internal API responses.
    /// Handles serialization and logging of FlatBuffer protocol messages for internal communication.
    /// </summary>
    public class FlatBufferOutputFormatter : Http.Formatter.FlatBufferOutputFormatter
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FlatBufferOutputFormatter"/> class.
        /// </summary>
        public FlatBufferOutputFormatter()
        {

        }

        /// <summary>
        /// Generates a log message for the outgoing FlatBuffer protocol.
        /// Creates a formatted log entry with protocol type and serialized content.
        /// </summary>
        /// <param name="protocol">The FlatBuffer protocol to log.</param>
        /// <returns>A formatted log message string for the response.</returns>
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
