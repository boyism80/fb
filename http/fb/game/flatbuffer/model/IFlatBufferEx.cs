namespace Google.FlatBuffers
{
    public interface IFlatBufferEx
    {
        byte[] Serialize();
    }
}

namespace fb.game.flatbuffer.db
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
                FlatBufferProtocolType.Ping => typeof(fb.game.flatbuffer.db.model.Ping),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.game.flatbuffer.db.model.Ping => FlatBufferProtocolType.Ping,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.game.flatbuffer.inter
{
    public enum FlatBufferProtocolType
    { 
        Ping,
        Pong,
        Whisper
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Ping => typeof(fb.game.flatbuffer.inter.model.Ping),
                FlatBufferProtocolType.Pong => typeof(fb.game.flatbuffer.inter.model.Pong),
                FlatBufferProtocolType.Whisper => typeof(fb.game.flatbuffer.inter.model.Whisper),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.game.flatbuffer.inter.model.Ping => FlatBufferProtocolType.Ping,
                fb.game.flatbuffer.inter.model.Pong => FlatBufferProtocolType.Pong,
                fb.game.flatbuffer.inter.model.Whisper => FlatBufferProtocolType.Whisper,
                _ => throw new ArgumentException(),
            };
        }
    }
}
