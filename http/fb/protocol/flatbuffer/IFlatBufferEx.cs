namespace Google.FlatBuffers
{
    public interface IFlatBufferEx
    {
        byte[] Serialize();
    }
}

namespace fb.protocol.db.request
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
                FlatBufferProtocolType.Ping => typeof(fb.protocol.db.request.model.Ping),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.db.request.model.Ping => FlatBufferProtocolType.Ping,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.internal.request
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
                FlatBufferProtocolType.Ping => typeof(fb.protocol.internal.request.model.Ping),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol.internal.request.model.Transfer),
                FlatBufferProtocolType.Whisper => typeof(fb.protocol.internal.request.model.Whisper),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.internal.request.model.Ping => FlatBufferProtocolType.Ping,
                fb.protocol.internal.request.model.Transfer => FlatBufferProtocolType.Transfer,
                fb.protocol.internal.request.model.Whisper => FlatBufferProtocolType.Whisper,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.internal.response
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
                FlatBufferProtocolType.Pong => typeof(fb.protocol.internal.response.model.Pong),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol.internal.response.model.Transfer),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.internal.response.model.Pong => FlatBufferProtocolType.Pong,
                fb.protocol.internal.response.model.Transfer => FlatBufferProtocolType.Transfer,
                _ => throw new ArgumentException(),
            };
        }
    }
}
