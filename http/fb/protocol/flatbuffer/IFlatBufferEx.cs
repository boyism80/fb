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
        CreateCharacter,
        FinishCharacter,
        InitCharacter,
        Ping
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.CreateCharacter => typeof(fb.protocol.db.request.CreateCharacter),
                FlatBufferProtocolType.FinishCharacter => typeof(fb.protocol.db.request.FinishCharacter),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol.db.request.InitCharacter),
                FlatBufferProtocolType.Ping => typeof(fb.protocol.db.request.Ping),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.db.request.CreateCharacter => FlatBufferProtocolType.CreateCharacter,
                fb.protocol.db.request.FinishCharacter => FlatBufferProtocolType.FinishCharacter,
                fb.protocol.db.request.InitCharacter => FlatBufferProtocolType.InitCharacter,
                fb.protocol.db.request.Ping => FlatBufferProtocolType.Ping,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.db.response
{
    public enum FlatBufferProtocolType
    { 
        CreateCharacter,
        FinishCharacter,
        InitCharacter
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.CreateCharacter => typeof(fb.protocol.db.response.CreateCharacter),
                FlatBufferProtocolType.FinishCharacter => typeof(fb.protocol.db.response.FinishCharacter),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol.db.response.InitCharacter),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.db.response.CreateCharacter => FlatBufferProtocolType.CreateCharacter,
                fb.protocol.db.response.FinishCharacter => FlatBufferProtocolType.FinishCharacter,
                fb.protocol.db.response.InitCharacter => FlatBufferProtocolType.InitCharacter,
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.inter.request
{
    public enum FlatBufferProtocolType
    { 
        Login,
        Logout,
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
                FlatBufferProtocolType.Login => typeof(fb.protocol.inter.request.Login),
                FlatBufferProtocolType.Logout => typeof(fb.protocol.inter.request.Logout),
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
                fb.protocol.inter.request.Login => FlatBufferProtocolType.Login,
                fb.protocol.inter.request.Logout => FlatBufferProtocolType.Logout,
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
        Login,
        Logout,
        Pong,
        Transfer
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Login => typeof(fb.protocol.inter.response.Login),
                FlatBufferProtocolType.Logout => typeof(fb.protocol.inter.response.Logout),
                FlatBufferProtocolType.Pong => typeof(fb.protocol.inter.response.Pong),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol.inter.response.Transfer),
                _ => throw new ArgumentException(),
            };
        }

        public static FlatBufferProtocolType GetProtocolEnum<T>(T protocol)
        {
            return protocol switch
            {
                fb.protocol.inter.response.Login => FlatBufferProtocolType.Login,
                fb.protocol.inter.response.Logout => FlatBufferProtocolType.Logout,
                fb.protocol.inter.response.Pong => FlatBufferProtocolType.Pong,
                fb.protocol.inter.response.Transfer => FlatBufferProtocolType.Transfer,
                _ => throw new ArgumentException(),
            };
        }
    }
}
