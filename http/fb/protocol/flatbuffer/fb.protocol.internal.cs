using Google.FlatBuffers;

namespace fb.protocol.inter
{
    public enum Service : sbyte
    {
        Gateway = fb.protocol.inter.origin.Service.Gateway,
        Login = fb.protocol.inter.origin.Service.Login,
        Game = fb.protocol.inter.origin.Service.Game
    }

}
