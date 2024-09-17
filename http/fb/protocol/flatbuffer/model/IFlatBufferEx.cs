namespace Google.FlatBuffers
{
    public interface IFlatBufferEx
    {
        byte[] Serialize();
    }
}

namespace fb.protocol.flatbuffer.db.request
{
    public enum FlatBufferProtocolType
    { 
        Ping
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Ping => typeof(fb.protocol.flatbuffer.db.request.model.Ping),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.flatbuffer.db.request.model.Ping => FlatBufferProtocolType.Ping,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.flatbuffer.inter.request
{
    public enum FlatBufferProtocolType
    { 
        Ping,
        Transfer,
        Whisper
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Ping => typeof(fb.protocol.flatbuffer.inter.request.model.Ping),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol.flatbuffer.inter.request.model.Transfer),
                FlatBufferProtocolType.Whisper => typeof(fb.protocol.flatbuffer.inter.request.model.Whisper),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.flatbuffer.inter.request.model.Ping => FlatBufferProtocolType.Ping,
                fb.protocol.flatbuffer.inter.request.model.Transfer => FlatBufferProtocolType.Transfer,
                fb.protocol.flatbuffer.inter.request.model.Whisper => FlatBufferProtocolType.Whisper,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.flatbuffer.inter.response
{
    public enum FlatBufferProtocolType
    { 
        Pong,
        Transfer
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Pong => typeof(fb.protocol.flatbuffer.inter.response.model.Pong),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol.flatbuffer.inter.response.model.Transfer),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.flatbuffer.inter.response.model.Pong => FlatBufferProtocolType.Pong,
                fb.protocol.flatbuffer.inter.response.model.Transfer => FlatBufferProtocolType.Transfer,
                _ => throw new ArgumentException(),
            };
        }
    }
}
