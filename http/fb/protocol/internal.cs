using Google.FlatBuffers;

namespace fb.protocol.inter
{
    public enum Service : sbyte
    {
        Gateway = fb.protocol.inter.origin.Service.Gateway,
        Login = fb.protocol.inter.origin.Service.Login,
        Game = fb.protocol.inter.origin.Service.Game
    }
    public enum TransferResult : sbyte
    {
        Success = fb.protocol.inter.origin.TransferResult.Success,
        Failed = fb.protocol.inter.origin.TransferResult.Failed,
        LoggedIn = fb.protocol.inter.origin.TransferResult.LoggedIn
    }

}
