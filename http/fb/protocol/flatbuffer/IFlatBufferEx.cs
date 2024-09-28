namespace Google.FlatBuffers
{
    public interface IFlatBufferEx
    {
        int ProtocolType { get; }
        byte[] Serialize();
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
    }
}
