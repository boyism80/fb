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
                FlatBufferProtocolType.Ping => typeof(fb.protocol.db.request.Ping),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.db.request.Ping => FlatBufferProtocolType.Ping,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.inter
{
    public enum FlatBufferProtocolType
    { 
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.inter.request
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
                FlatBufferProtocolType.Ping => typeof(fb.protocol.inter.request.Ping),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol.inter.request.Transfer),
                FlatBufferProtocolType.Whisper => typeof(fb.protocol.inter.request.Whisper),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.inter.request.Ping => FlatBufferProtocolType.Ping,
                fb.protocol.inter.request.Transfer => FlatBufferProtocolType.Transfer,
                fb.protocol.inter.request.Whisper => FlatBufferProtocolType.Whisper,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.inter.response
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
                FlatBufferProtocolType.Pong => typeof(fb.protocol.inter.response.Pong),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol.inter.response.Transfer),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.inter.response.Pong => FlatBufferProtocolType.Pong,
                fb.protocol.inter.response.Transfer => FlatBufferProtocolType.Transfer,
                _ => throw new ArgumentException(),
            };
        }
    }
}
