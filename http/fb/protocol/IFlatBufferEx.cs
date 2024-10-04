namespace Google.FlatBuffers
{
    public interface IFlatBufferEx
    {
        int ProtocolType { get; }
        byte[] Serialize();
    }
}

namespace fb.protocol.db.request
{
    public enum FlatBufferProtocolType
    { 
        Account,
        ChangePw,
        InitCharacter,
        MakeCharacter,
        ReserveName
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Account => typeof(fb.protocol.db.request.Account),
                FlatBufferProtocolType.ChangePw => typeof(fb.protocol.db.request.ChangePw),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol.db.request.InitCharacter),
                FlatBufferProtocolType.MakeCharacter => typeof(fb.protocol.db.request.MakeCharacter),
                FlatBufferProtocolType.ReserveName => typeof(fb.protocol.db.request.ReserveName),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.db.response
{
    public enum FlatBufferProtocolType
    { 
        Account,
        ChangePw,
        GetUid,
        InitCharacter,
        MakeCharacter,
        ReserveName
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Account => typeof(fb.protocol.db.response.Account),
                FlatBufferProtocolType.ChangePw => typeof(fb.protocol.db.response.ChangePw),
                FlatBufferProtocolType.GetUid => typeof(fb.protocol.db.response.GetUid),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol.db.response.InitCharacter),
                FlatBufferProtocolType.MakeCharacter => typeof(fb.protocol.db.response.MakeCharacter),
                FlatBufferProtocolType.ReserveName => typeof(fb.protocol.db.response.ReserveName),
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
    }
}
namespace fb.protocol.inter.response
{
    public enum FlatBufferProtocolType
    { 
        KickOut,
        Login,
        Logout,
        Pong,
        Transfer,
        Whisper
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.KickOut => typeof(fb.protocol.inter.response.KickOut),
                FlatBufferProtocolType.Login => typeof(fb.protocol.inter.response.Login),
                FlatBufferProtocolType.Logout => typeof(fb.protocol.inter.response.Logout),
                FlatBufferProtocolType.Pong => typeof(fb.protocol.inter.response.Pong),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol.inter.response.Transfer),
                FlatBufferProtocolType.Whisper => typeof(fb.protocol.inter.response.Whisper),
                _ => throw new ArgumentException(),
            };
        }
    }
}
