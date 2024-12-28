using Google.FlatBuffers;
using nullable;

namespace Google.FlatBuffers
{
    public interface IFlatBufferEx
    {
        int ProtocolType { get; }
        byte[] Serialize();
    }
}

namespace fb.protocol._internal
{
    public static class FlatBufferBuilderEx
    {
        public static T Build<T>(this FlatBufferBuilder builder, T value)
        {
            return value;
        }

        public static StringOffset Build(this FlatBufferBuilder builder, string value)
        {
            return builder.CreateString(value);
        }

        public static Offset<nullable_ushort> Build(this FlatBufferBuilder builder, ushort? value)
        {
            if (value.HasValue)
                return nullable.nullable_ushort.Createnullable_ushort(builder, value.Value);
            else
                return default;
        }
        public static Offset<nullable_ubyte> Build(this FlatBufferBuilder builder, byte? value)
        {
            if (value.HasValue)
                return nullable.nullable_ubyte.Createnullable_ubyte(builder, value.Value);
            else
                return default;
        }
        public static Offset<nullable_uint> Build(this FlatBufferBuilder builder, uint? value)
        {
            if (value.HasValue)
                return nullable.nullable_uint.Createnullable_uint(builder, value.Value);
            else
                return default;
        }
        public static Offset<fb.protocol._internal.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol._internal.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol._internal.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol._internal.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Pw),
                builder.Build(value.Admin),
                builder.Build(value.Look),
                builder.Build(value.Color),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature),
                builder.Build(value.Map),
                builder.Build(value.Position),
                builder.Build(value.Direction),
                builder.Build(value.State),
                builder.Build(value.ClassType),
                builder.Build(value.Promotion),
                builder.Build(value.Level),
                builder.Build(value.Exp),
                builder.Build(value.Money),
                builder.Build(value.DepositedMoney),
                builder.Build(value.Disguise),
                builder.Build(value.Hp),
                builder.Build(value.BaseHp),
                builder.Build(value.AdditionalHp),
                builder.Build(value.Mp),
                builder.Build(value.BaseMp),
                builder.Build(value.AdditionalMp),
                builder.Build(value.WeaponColor),
                builder.Build(value.HelmetColor),
                builder.Build(value.ArmorColor),
                builder.Build(value.ShieldColor),
                builder.Build(value.RingLeftColor),
                builder.Build(value.RingRightColor),
                builder.Build(value.AuxTopColor),
                builder.Build(value.AuxBotColor),
                builder.Build(value.UpdatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol._internal.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol._internal.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol._internal.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol._internal.raw.Option> Build(this FlatBufferBuilder builder, fb.protocol._internal.Option value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Option.CreateOption(builder,
                builder.Build(value.Uid),
                builder.Build(value.Whisper),
                builder.Build(value.Group),
                builder.Build(value.Roar),
                builder.Build(value.RoarWorlds),
                builder.Build(value.MagicEffect),
                builder.Build(value.WeatherEffect),
                builder.Build(value.FixedMove),
                builder.Build(value.Trade),
                builder.Build(value.FastMove),
                builder.Build(value.EffectSound),
                builder.Build(value.PkProtect));
        }
        public static Offset<fb.protocol._internal.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol._internal.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol._internal.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Group> Build(this FlatBufferBuilder builder, fb.protocol._internal.Group value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Group.CreateGroup(builder,
                builder.Build(value.Id),
                builder.Build(value.Master),
                builder.Build(value.Members));
        }
        public static Offset<fb.protocol._internal.raw.Trace> Build(this FlatBufferBuilder builder, fb.protocol._internal.Trace value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Trace.CreateTrace(builder,
                builder.Build(value.User),
                builder.Build(value.Model),
                builder.Build(value.Text));
        }
        public static Offset<fb.protocol._internal.raw.Clan> Build(this FlatBufferBuilder builder, fb.protocol._internal.Clan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Clan.CreateClan(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Title));
        }
        public static Offset<fb.protocol._internal.raw.ClanMember> Build(this FlatBufferBuilder builder, fb.protocol._internal.ClanMember value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.ClanMember.CreateClanMember(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Host));
        }
        public static Offset<fb.protocol._internal.request.raw.Logout> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Logout value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Logout.CreateLogout(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.Ping> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Ping value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Ping.CreatePing(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Service),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.request.raw.Transfer> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Transfer value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Transfer.CreateTransfer(builder,
                builder.Build(value.Service),
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.ForceShutdown));
        }
        public static Offset<fb.protocol._internal.request.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static Offset<fb.protocol._internal.request.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol._internal.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Pw),
                builder.Build(value.Hp),
                builder.Build(value.Mp),
                builder.Build(value.Map),
                builder.Build(value.X),
                builder.Build(value.Y),
                builder.Build(value.Admin));
        }
        public static Offset<fb.protocol._internal.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol._internal.request.raw.Init> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Init value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Init.CreateInit(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol._internal.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells),
                builder.Build(value.Traces));
        }
        public static Offset<fb.protocol._internal.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol._internal.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol._internal.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol._internal.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol._internal.request.raw.SetOption> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.SetOption value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.SetOption.CreateSetOption(builder,
                builder.Build(value.User),
                builder.Build(value.Type),
                builder.Build(value.Enabled));
        }
        public static Offset<fb.protocol._internal.request.raw.EnterGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.EnterGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.EnterGroup.CreateEnterGroup(builder,
                builder.Build(value.Master),
                builder.Build(value.Member));
        }
        public static Offset<fb.protocol._internal.request.raw.LeaveGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.LeaveGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.LeaveGroup.CreateLeaveGroup(builder,
                builder.Build(value.Member));
        }
        public static Offset<fb.protocol._internal.request.raw.CreateClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.CreateClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.CreateClan.CreateCreateClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.DestroyClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.DestroyClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.DestroyClan.CreateDestroyClan(builder,
                builder.Build(value.Master));
        }
        public static Offset<fb.protocol._internal.request.raw.JoinClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.JoinClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.JoinClan.CreateJoinClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.LeaveClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.LeaveClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.LeaveClan.CreateLeaveClan(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.KickClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.KickClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.KickClan.CreateKickClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.SetClanTitle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.SetClanTitle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.SetClanTitle.CreateSetClanTitle(builder,
                builder.Build(value.Clan),
                builder.Build(value.Title));
        }
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Error),
                builder.Build(value.Logon),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Logout> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Logout value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Logout.CreateLogout(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Pong> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Pong value)
        {
            if (value == null)
                return default;

            fb.protocol._internal.response.raw.Pong.StartPong(builder);
            return fb.protocol._internal.response.raw.Pong.EndPong(builder);
        }
        public static Offset<fb.protocol._internal.response.raw.Transfer> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Transfer value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Transfer.CreateTransfer(builder,
                builder.Build(value.Error),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol._internal.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol._internal.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol._internal.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol._internal.response.raw.Init> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Init value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Init.CreateInit(builder,
                builder.Build(value.Character),
                builder.Build(value.Group),
                builder.Build(value.Clan),
                builder.Build(value.Items),
                builder.Build(value.Spells),
                builder.Build(value.Option),
                builder.Build(value.Traces));
        }
        public static Offset<fb.protocol._internal.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.SetOption> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.SetOption value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.SetOption.CreateSetOption(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.GetGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetGroup.CreateGetGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.EnterGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.EnterGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.EnterGroup.CreateEnterGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Member),
                builder.Build(value.Action),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.LeaveGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.LeaveGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.LeaveGroup.CreateLeaveGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Member),
                builder.Build(value.Action),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.GetClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetClan.CreateGetClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Members),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.CreateClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.CreateClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.CreateClan.CreateCreateClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Members),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.DestroyClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.DestroyClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.DestroyClan.CreateDestroyClan(builder,
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.JoinClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.JoinClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.JoinClan.CreateJoinClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.LeaveClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.LeaveClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.LeaveClan.CreateLeaveClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.KickClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickClan.CreateKickClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.SetClanTitle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.SetClanTitle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.SetClanTitle.CreateSetClanTitle(builder,
                builder.Build(value.Clan),
                builder.Build(value.Title),
                builder.Build(value.Error));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<string> value)
        {
            var offsets = value.ConvertAll(x => builder.Build(x).Value);
            builder.StartVector(4, value.Count, 4);
            foreach (var offset in offsets)
            {
                builder.AddOffset(offset);
            }
            return builder.EndVector();
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Item> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Spell> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Trace> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Trace>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.ArticleSummary> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.ClanMember> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.ClanMember>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.GroupAction Build(this FlatBufferBuilder builder, fb.protocol._internal.GroupAction value)
        {
            return (fb.protocol._internal.raw.GroupAction)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
        Position,
        Character,
        Item,
        Spell,
        Option,
        ArticleSummary,
        Article,
        Group,
        Trace,
        Clan,
        ClanMember
    }
}
namespace fb.protocol._internal.request
{
    public static class FlatBufferBuilderEx
    {
        public static T Build<T>(this FlatBufferBuilder builder, T value)
        {
            return value;
        }

        public static StringOffset Build(this FlatBufferBuilder builder, string value)
        {
            return builder.CreateString(value);
        }

        public static Offset<nullable_ushort> Build(this FlatBufferBuilder builder, ushort? value)
        {
            if (value.HasValue)
                return nullable.nullable_ushort.Createnullable_ushort(builder, value.Value);
            else
                return default;
        }
        public static Offset<nullable_ubyte> Build(this FlatBufferBuilder builder, byte? value)
        {
            if (value.HasValue)
                return nullable.nullable_ubyte.Createnullable_ubyte(builder, value.Value);
            else
                return default;
        }
        public static Offset<nullable_uint> Build(this FlatBufferBuilder builder, uint? value)
        {
            if (value.HasValue)
                return nullable.nullable_uint.Createnullable_uint(builder, value.Value);
            else
                return default;
        }
        public static Offset<fb.protocol._internal.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol._internal.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol._internal.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol._internal.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Pw),
                builder.Build(value.Admin),
                builder.Build(value.Look),
                builder.Build(value.Color),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature),
                builder.Build(value.Map),
                builder.Build(value.Position),
                builder.Build(value.Direction),
                builder.Build(value.State),
                builder.Build(value.ClassType),
                builder.Build(value.Promotion),
                builder.Build(value.Level),
                builder.Build(value.Exp),
                builder.Build(value.Money),
                builder.Build(value.DepositedMoney),
                builder.Build(value.Disguise),
                builder.Build(value.Hp),
                builder.Build(value.BaseHp),
                builder.Build(value.AdditionalHp),
                builder.Build(value.Mp),
                builder.Build(value.BaseMp),
                builder.Build(value.AdditionalMp),
                builder.Build(value.WeaponColor),
                builder.Build(value.HelmetColor),
                builder.Build(value.ArmorColor),
                builder.Build(value.ShieldColor),
                builder.Build(value.RingLeftColor),
                builder.Build(value.RingRightColor),
                builder.Build(value.AuxTopColor),
                builder.Build(value.AuxBotColor),
                builder.Build(value.UpdatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol._internal.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol._internal.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol._internal.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol._internal.raw.Option> Build(this FlatBufferBuilder builder, fb.protocol._internal.Option value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Option.CreateOption(builder,
                builder.Build(value.Uid),
                builder.Build(value.Whisper),
                builder.Build(value.Group),
                builder.Build(value.Roar),
                builder.Build(value.RoarWorlds),
                builder.Build(value.MagicEffect),
                builder.Build(value.WeatherEffect),
                builder.Build(value.FixedMove),
                builder.Build(value.Trade),
                builder.Build(value.FastMove),
                builder.Build(value.EffectSound),
                builder.Build(value.PkProtect));
        }
        public static Offset<fb.protocol._internal.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol._internal.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol._internal.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Group> Build(this FlatBufferBuilder builder, fb.protocol._internal.Group value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Group.CreateGroup(builder,
                builder.Build(value.Id),
                builder.Build(value.Master),
                builder.Build(value.Members));
        }
        public static Offset<fb.protocol._internal.raw.Trace> Build(this FlatBufferBuilder builder, fb.protocol._internal.Trace value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Trace.CreateTrace(builder,
                builder.Build(value.User),
                builder.Build(value.Model),
                builder.Build(value.Text));
        }
        public static Offset<fb.protocol._internal.raw.Clan> Build(this FlatBufferBuilder builder, fb.protocol._internal.Clan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Clan.CreateClan(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Title));
        }
        public static Offset<fb.protocol._internal.raw.ClanMember> Build(this FlatBufferBuilder builder, fb.protocol._internal.ClanMember value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.ClanMember.CreateClanMember(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Host));
        }
        public static Offset<fb.protocol._internal.request.raw.Logout> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Logout value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Logout.CreateLogout(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.Ping> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Ping value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Ping.CreatePing(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Service),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.request.raw.Transfer> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Transfer value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Transfer.CreateTransfer(builder,
                builder.Build(value.Service),
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.ForceShutdown));
        }
        public static Offset<fb.protocol._internal.request.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static Offset<fb.protocol._internal.request.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol._internal.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Pw),
                builder.Build(value.Hp),
                builder.Build(value.Mp),
                builder.Build(value.Map),
                builder.Build(value.X),
                builder.Build(value.Y),
                builder.Build(value.Admin));
        }
        public static Offset<fb.protocol._internal.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol._internal.request.raw.Init> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Init value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Init.CreateInit(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol._internal.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells),
                builder.Build(value.Traces));
        }
        public static Offset<fb.protocol._internal.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol._internal.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol._internal.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol._internal.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol._internal.request.raw.SetOption> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.SetOption value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.SetOption.CreateSetOption(builder,
                builder.Build(value.User),
                builder.Build(value.Type),
                builder.Build(value.Enabled));
        }
        public static Offset<fb.protocol._internal.request.raw.EnterGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.EnterGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.EnterGroup.CreateEnterGroup(builder,
                builder.Build(value.Master),
                builder.Build(value.Member));
        }
        public static Offset<fb.protocol._internal.request.raw.LeaveGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.LeaveGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.LeaveGroup.CreateLeaveGroup(builder,
                builder.Build(value.Member));
        }
        public static Offset<fb.protocol._internal.request.raw.CreateClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.CreateClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.CreateClan.CreateCreateClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.DestroyClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.DestroyClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.DestroyClan.CreateDestroyClan(builder,
                builder.Build(value.Master));
        }
        public static Offset<fb.protocol._internal.request.raw.JoinClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.JoinClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.JoinClan.CreateJoinClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.LeaveClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.LeaveClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.LeaveClan.CreateLeaveClan(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.KickClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.KickClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.KickClan.CreateKickClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.SetClanTitle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.SetClanTitle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.SetClanTitle.CreateSetClanTitle(builder,
                builder.Build(value.Clan),
                builder.Build(value.Title));
        }
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Error),
                builder.Build(value.Logon),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Logout> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Logout value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Logout.CreateLogout(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Pong> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Pong value)
        {
            if (value == null)
                return default;

            fb.protocol._internal.response.raw.Pong.StartPong(builder);
            return fb.protocol._internal.response.raw.Pong.EndPong(builder);
        }
        public static Offset<fb.protocol._internal.response.raw.Transfer> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Transfer value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Transfer.CreateTransfer(builder,
                builder.Build(value.Error),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol._internal.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol._internal.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol._internal.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol._internal.response.raw.Init> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Init value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Init.CreateInit(builder,
                builder.Build(value.Character),
                builder.Build(value.Group),
                builder.Build(value.Clan),
                builder.Build(value.Items),
                builder.Build(value.Spells),
                builder.Build(value.Option),
                builder.Build(value.Traces));
        }
        public static Offset<fb.protocol._internal.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.SetOption> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.SetOption value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.SetOption.CreateSetOption(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.GetGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetGroup.CreateGetGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.EnterGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.EnterGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.EnterGroup.CreateEnterGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Member),
                builder.Build(value.Action),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.LeaveGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.LeaveGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.LeaveGroup.CreateLeaveGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Member),
                builder.Build(value.Action),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.GetClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetClan.CreateGetClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Members),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.CreateClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.CreateClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.CreateClan.CreateCreateClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Members),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.DestroyClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.DestroyClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.DestroyClan.CreateDestroyClan(builder,
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.JoinClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.JoinClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.JoinClan.CreateJoinClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.LeaveClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.LeaveClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.LeaveClan.CreateLeaveClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.KickClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickClan.CreateKickClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.SetClanTitle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.SetClanTitle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.SetClanTitle.CreateSetClanTitle(builder,
                builder.Build(value.Clan),
                builder.Build(value.Title),
                builder.Build(value.Error));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<string> value)
        {
            var offsets = value.ConvertAll(x => builder.Build(x).Value);
            builder.StartVector(4, value.Count, 4);
            foreach (var offset in offsets)
            {
                builder.AddOffset(offset);
            }
            return builder.EndVector();
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Item> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Spell> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Trace> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Trace>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.ArticleSummary> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.ClanMember> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.ClanMember>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.GroupAction Build(this FlatBufferBuilder builder, fb.protocol._internal.GroupAction value)
        {
            return (fb.protocol._internal.raw.GroupAction)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
        Login,
        Logout,
        Ping,
        Transfer,
        Whisper,
        KickOut,
        Account,
        ChangePw,
        InitCharacter,
        Authenticate,
        Init,
        MakeCharacter,
        ReserveName,
        Save,
        GetArticle,
        GetArticleList,
        WriteArticle,
        DeleteArticle,
        SetOption,
        EnterGroup,
        LeaveGroup,
        CreateClan,
        DestroyClan,
        JoinClan,
        LeaveClan,
        KickClan,
        SetClanTitle
    }
}
namespace fb.protocol._internal.response
{
    public static class FlatBufferBuilderEx
    {
        public static T Build<T>(this FlatBufferBuilder builder, T value)
        {
            return value;
        }

        public static StringOffset Build(this FlatBufferBuilder builder, string value)
        {
            return builder.CreateString(value);
        }

        public static Offset<nullable_ushort> Build(this FlatBufferBuilder builder, ushort? value)
        {
            if (value.HasValue)
                return nullable.nullable_ushort.Createnullable_ushort(builder, value.Value);
            else
                return default;
        }
        public static Offset<nullable_ubyte> Build(this FlatBufferBuilder builder, byte? value)
        {
            if (value.HasValue)
                return nullable.nullable_ubyte.Createnullable_ubyte(builder, value.Value);
            else
                return default;
        }
        public static Offset<nullable_uint> Build(this FlatBufferBuilder builder, uint? value)
        {
            if (value.HasValue)
                return nullable.nullable_uint.Createnullable_uint(builder, value.Value);
            else
                return default;
        }
        public static Offset<fb.protocol._internal.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol._internal.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol._internal.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol._internal.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Pw),
                builder.Build(value.Admin),
                builder.Build(value.Look),
                builder.Build(value.Color),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature),
                builder.Build(value.Map),
                builder.Build(value.Position),
                builder.Build(value.Direction),
                builder.Build(value.State),
                builder.Build(value.ClassType),
                builder.Build(value.Promotion),
                builder.Build(value.Level),
                builder.Build(value.Exp),
                builder.Build(value.Money),
                builder.Build(value.DepositedMoney),
                builder.Build(value.Disguise),
                builder.Build(value.Hp),
                builder.Build(value.BaseHp),
                builder.Build(value.AdditionalHp),
                builder.Build(value.Mp),
                builder.Build(value.BaseMp),
                builder.Build(value.AdditionalMp),
                builder.Build(value.WeaponColor),
                builder.Build(value.HelmetColor),
                builder.Build(value.ArmorColor),
                builder.Build(value.ShieldColor),
                builder.Build(value.RingLeftColor),
                builder.Build(value.RingRightColor),
                builder.Build(value.AuxTopColor),
                builder.Build(value.AuxBotColor),
                builder.Build(value.UpdatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol._internal.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol._internal.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol._internal.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol._internal.raw.Option> Build(this FlatBufferBuilder builder, fb.protocol._internal.Option value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Option.CreateOption(builder,
                builder.Build(value.Uid),
                builder.Build(value.Whisper),
                builder.Build(value.Group),
                builder.Build(value.Roar),
                builder.Build(value.RoarWorlds),
                builder.Build(value.MagicEffect),
                builder.Build(value.WeatherEffect),
                builder.Build(value.FixedMove),
                builder.Build(value.Trade),
                builder.Build(value.FastMove),
                builder.Build(value.EffectSound),
                builder.Build(value.PkProtect));
        }
        public static Offset<fb.protocol._internal.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol._internal.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol._internal.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol._internal.raw.Group> Build(this FlatBufferBuilder builder, fb.protocol._internal.Group value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Group.CreateGroup(builder,
                builder.Build(value.Id),
                builder.Build(value.Master),
                builder.Build(value.Members));
        }
        public static Offset<fb.protocol._internal.raw.Trace> Build(this FlatBufferBuilder builder, fb.protocol._internal.Trace value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Trace.CreateTrace(builder,
                builder.Build(value.User),
                builder.Build(value.Model),
                builder.Build(value.Text));
        }
        public static Offset<fb.protocol._internal.raw.Clan> Build(this FlatBufferBuilder builder, fb.protocol._internal.Clan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.Clan.CreateClan(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Title));
        }
        public static Offset<fb.protocol._internal.raw.ClanMember> Build(this FlatBufferBuilder builder, fb.protocol._internal.ClanMember value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.raw.ClanMember.CreateClanMember(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Host));
        }
        public static Offset<fb.protocol._internal.request.raw.Logout> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Logout value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Logout.CreateLogout(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.Ping> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Ping value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Ping.CreatePing(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.Service),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.request.raw.Transfer> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Transfer value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Transfer.CreateTransfer(builder,
                builder.Build(value.Service),
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.ForceShutdown));
        }
        public static Offset<fb.protocol._internal.request.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static Offset<fb.protocol._internal.request.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol._internal.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Pw),
                builder.Build(value.Hp),
                builder.Build(value.Mp),
                builder.Build(value.Map),
                builder.Build(value.X),
                builder.Build(value.Y),
                builder.Build(value.Admin));
        }
        public static Offset<fb.protocol._internal.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol._internal.request.raw.Init> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Init value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Init.CreateInit(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol._internal.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells),
                builder.Build(value.Traces));
        }
        public static Offset<fb.protocol._internal.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol._internal.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol._internal.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol._internal.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol._internal.request.raw.SetOption> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.SetOption value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.SetOption.CreateSetOption(builder,
                builder.Build(value.User),
                builder.Build(value.Type),
                builder.Build(value.Enabled));
        }
        public static Offset<fb.protocol._internal.request.raw.EnterGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.EnterGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.EnterGroup.CreateEnterGroup(builder,
                builder.Build(value.Master),
                builder.Build(value.Member));
        }
        public static Offset<fb.protocol._internal.request.raw.LeaveGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.LeaveGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.LeaveGroup.CreateLeaveGroup(builder,
                builder.Build(value.Member));
        }
        public static Offset<fb.protocol._internal.request.raw.CreateClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.CreateClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.CreateClan.CreateCreateClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.request.raw.DestroyClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.DestroyClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.DestroyClan.CreateDestroyClan(builder,
                builder.Build(value.Master));
        }
        public static Offset<fb.protocol._internal.request.raw.JoinClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.JoinClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.JoinClan.CreateJoinClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.LeaveClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.LeaveClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.LeaveClan.CreateLeaveClan(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.KickClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.KickClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.KickClan.CreateKickClan(builder,
                builder.Build(value.Master),
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.request.raw.SetClanTitle> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.SetClanTitle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.SetClanTitle.CreateSetClanTitle(builder,
                builder.Build(value.Clan),
                builder.Build(value.Title));
        }
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Error),
                builder.Build(value.Logon),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Logout> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Logout value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Logout.CreateLogout(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Pong> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Pong value)
        {
            if (value == null)
                return default;

            fb.protocol._internal.response.raw.Pong.StartPong(builder);
            return fb.protocol._internal.response.raw.Pong.EndPong(builder);
        }
        public static Offset<fb.protocol._internal.response.raw.Transfer> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Transfer value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Transfer.CreateTransfer(builder,
                builder.Build(value.Error),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol._internal.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol._internal.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol._internal.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol._internal.response.raw.Init> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Init value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Init.CreateInit(builder,
                builder.Build(value.Character),
                builder.Build(value.Group),
                builder.Build(value.Clan),
                builder.Build(value.Items),
                builder.Build(value.Spells),
                builder.Build(value.Option),
                builder.Build(value.Traces));
        }
        public static Offset<fb.protocol._internal.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.SetOption> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.SetOption value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.SetOption.CreateSetOption(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.response.raw.GetGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetGroup.CreateGetGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.EnterGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.EnterGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.EnterGroup.CreateEnterGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Member),
                builder.Build(value.Action),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.LeaveGroup> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.LeaveGroup value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.LeaveGroup.CreateLeaveGroup(builder,
                builder.Build(value.Group),
                builder.Build(value.Member),
                builder.Build(value.Action),
                builder.Build(value.Host),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.GetClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.GetClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.GetClan.CreateGetClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Members),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.CreateClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.CreateClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.CreateClan.CreateCreateClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Members),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.DestroyClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.DestroyClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.DestroyClan.CreateDestroyClan(builder,
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.JoinClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.JoinClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.JoinClan.CreateJoinClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.LeaveClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.LeaveClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.LeaveClan.CreateLeaveClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.KickClan> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickClan value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickClan.CreateKickClan(builder,
                builder.Build(value.Clan),
                builder.Build(value.Uid),
                builder.Build(value.Uname),
                builder.Build(value.Error));
        }
        public static Offset<fb.protocol._internal.response.raw.SetClanTitle> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.SetClanTitle value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.SetClanTitle.CreateSetClanTitle(builder,
                builder.Build(value.Clan),
                builder.Build(value.Title),
                builder.Build(value.Error));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<string> value)
        {
            var offsets = value.ConvertAll(x => builder.Build(x).Value);
            builder.StartVector(4, value.Count, 4);
            foreach (var offset in offsets)
            {
                builder.AddOffset(offset);
            }
            return builder.EndVector();
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Item> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Spell> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.Trace> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.Trace>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.ArticleSummary> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol._internal.ClanMember> value)
        {
            return builder.CreateVectorOfTables<fb.protocol._internal.raw.ClanMember>(value.Select(x => Build(builder, x)).ToArray());
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.GroupAction Build(this FlatBufferBuilder builder, fb.protocol._internal.GroupAction value)
        {
            return (fb.protocol._internal.raw.GroupAction)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
        KickOut,
        Login,
        Logout,
        Pong,
        Transfer,
        Whisper,
        DeleteArticle,
        GetArticle,
        GetArticleList,
        WriteArticle,
        ChangePw,
        GetUid,
        InitCharacter,
        Authenticate,
        Init,
        MakeCharacter,
        ReserveName,
        Save,
        SetOption,
        GetGroup,
        EnterGroup,
        LeaveGroup,
        GetClan,
        CreateClan,
        DestroyClan,
        JoinClan,
        LeaveClan,
        KickClan,
        SetClanTitle
    }
}

namespace fb.protocol._internal
{
    public enum Service
    {
        Gateway,
        Login,
        Game,
    }
    public enum GroupAction
    {
        Create,
        Enter,
        Leave,
        Kick,
        BreakUp,
    }

    public class Position : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Position;
        public ushort X { get; set; } = 0;
        public ushort Y { get; set; } = 0;

        public Position()
        { }

        public Position(fb.protocol._internal.raw.Position raw)
        {
            X = raw.X;
            Y = raw.Y;
        }

        public Position(byte[] bytes) : this(fb.protocol._internal.raw.Position.GetRootAsPosition(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Position Deserialize(byte[] bytes)
        {
            return new Position(bytes);
        }
    }
    public class Character : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Character;
        public uint Id { get; set; } = 0;
        public string Name { get; set; } = string.Empty;
        public string Pw { get; set; } = string.Empty;
        public bool Admin { get; set; } = false;
        public ushort Look { get; set; } = 0;
        public ushort Color { get; set; } = 0;
        public ushort Sex { get; set; } = 0;
        public ushort Nation { get; set; } = 0;
        public ushort? Creature { get; set; } = null;
        public uint Map { get; set; } = 0;
        public fb.protocol._internal.Position Position { get; set; } = new fb.protocol._internal.Position();
        public byte Direction { get; set; } = 0;
        public byte State { get; set; } = 0;
        public byte ClassType { get; set; } = 0;
        public byte Promotion { get; set; } = 0;
        public byte Level { get; set; } = 0;
        public uint Exp { get; set; } = 0;
        public uint Money { get; set; } = 0;
        public uint DepositedMoney { get; set; } = 0;
        public ushort? Disguise { get; set; } = null;
        public uint Hp { get; set; } = 0;
        public uint BaseHp { get; set; } = 0;
        public uint AdditionalHp { get; set; } = 0;
        public uint Mp { get; set; } = 0;
        public uint BaseMp { get; set; } = 0;
        public uint AdditionalMp { get; set; } = 0;
        public byte? WeaponColor { get; set; } = null;
        public byte? HelmetColor { get; set; } = null;
        public byte? ArmorColor { get; set; } = null;
        public byte? ShieldColor { get; set; } = null;
        public byte? RingLeftColor { get; set; } = null;
        public byte? RingRightColor { get; set; } = null;
        public byte? AuxTopColor { get; set; } = null;
        public byte? AuxBotColor { get; set; } = null;
        public string UpdatedDate { get; set; } = string.Empty;

        public Character()
        { }

        public Character(fb.protocol._internal.raw.Character raw)
        {
            Id = raw.Id;
            Name = raw.Name;
            Pw = raw.Pw;
            Admin = raw.Admin;
            Look = raw.Look;
            Color = raw.Color;
            Sex = raw.Sex;
            Nation = raw.Nation;
            Creature = raw.Creature != null ? (ushort?)raw.Creature.Value.Value : null;
            Map = raw.Map;
            Position = new Position(raw.Position.Value);
            Direction = raw.Direction;
            State = raw.State;
            ClassType = raw.ClassType;
            Promotion = raw.Promotion;
            Level = raw.Level;
            Exp = raw.Exp;
            Money = raw.Money;
            DepositedMoney = raw.DepositedMoney;
            Disguise = raw.Disguise != null ? (ushort?)raw.Disguise.Value.Value : null;
            Hp = raw.Hp;
            BaseHp = raw.BaseHp;
            AdditionalHp = raw.AdditionalHp;
            Mp = raw.Mp;
            BaseMp = raw.BaseMp;
            AdditionalMp = raw.AdditionalMp;
            WeaponColor = raw.WeaponColor != null ? (byte?)raw.WeaponColor.Value.Value : null;
            HelmetColor = raw.HelmetColor != null ? (byte?)raw.HelmetColor.Value.Value : null;
            ArmorColor = raw.ArmorColor != null ? (byte?)raw.ArmorColor.Value.Value : null;
            ShieldColor = raw.ShieldColor != null ? (byte?)raw.ShieldColor.Value.Value : null;
            RingLeftColor = raw.RingLeftColor != null ? (byte?)raw.RingLeftColor.Value.Value : null;
            RingRightColor = raw.RingRightColor != null ? (byte?)raw.RingRightColor.Value.Value : null;
            AuxTopColor = raw.AuxTopColor != null ? (byte?)raw.AuxTopColor.Value.Value : null;
            AuxBotColor = raw.AuxBotColor != null ? (byte?)raw.AuxBotColor.Value.Value : null;
            UpdatedDate = raw.UpdatedDate;
        }

        public Character(byte[] bytes) : this(fb.protocol._internal.raw.Character.GetRootAsCharacter(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Character Deserialize(byte[] bytes)
        {
            return new Character(bytes);
        }
    }
    public class Item : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Item;
        public uint User { get; set; } = 0;
        public short Index { get; set; } = 0;
        public short Parts { get; set; } = 0;
        public short Deposited { get; set; } = 0;
        public uint Model { get; set; } = 0;
        public ushort Count { get; set; } = 0;
        public uint? Durability { get; set; } = null;
        public string CustomName { get; set; } = null;

        public Item()
        { }

        public Item(fb.protocol._internal.raw.Item raw)
        {
            User = raw.User;
            Index = raw.Index;
            Parts = raw.Parts;
            Deposited = raw.Deposited;
            Model = raw.Model;
            Count = raw.Count;
            Durability = raw.Durability != null ? (uint?)raw.Durability.Value.Value : null;
            CustomName = raw.CustomName;
        }

        public Item(byte[] bytes) : this(fb.protocol._internal.raw.Item.GetRootAsItem(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Item Deserialize(byte[] bytes)
        {
            return new Item(bytes);
        }
    }
    public class Spell : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Spell;
        public uint User { get; set; } = 0;
        public byte Slot { get; set; } = 0;
        public uint Model { get; set; } = 0;

        public Spell()
        { }

        public Spell(fb.protocol._internal.raw.Spell raw)
        {
            User = raw.User;
            Slot = raw.Slot;
            Model = raw.Model;
        }

        public Spell(byte[] bytes) : this(fb.protocol._internal.raw.Spell.GetRootAsSpell(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Spell Deserialize(byte[] bytes)
        {
            return new Spell(bytes);
        }
    }
    public class Option : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Option;
        public uint Uid { get; set; } = 0;
        public bool Whisper { get; set; } = false;
        public bool Group { get; set; } = false;
        public bool Roar { get; set; } = false;
        public bool RoarWorlds { get; set; } = false;
        public bool MagicEffect { get; set; } = false;
        public bool WeatherEffect { get; set; } = false;
        public bool FixedMove { get; set; } = false;
        public bool Trade { get; set; } = false;
        public bool FastMove { get; set; } = false;
        public bool EffectSound { get; set; } = false;
        public bool PkProtect { get; set; } = false;

        public Option()
        { }

        public Option(fb.protocol._internal.raw.Option raw)
        {
            Uid = raw.Uid;
            Whisper = raw.Whisper;
            Group = raw.Group;
            Roar = raw.Roar;
            RoarWorlds = raw.RoarWorlds;
            MagicEffect = raw.MagicEffect;
            WeatherEffect = raw.WeatherEffect;
            FixedMove = raw.FixedMove;
            Trade = raw.Trade;
            FastMove = raw.FastMove;
            EffectSound = raw.EffectSound;
            PkProtect = raw.PkProtect;
        }

        public Option(byte[] bytes) : this(fb.protocol._internal.raw.Option.GetRootAsOption(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Option Deserialize(byte[] bytes)
        {
            return new Option(bytes);
        }
    }
    public class ArticleSummary : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ArticleSummary;
        public uint Id { get; set; } = 0;
        public uint User { get; set; } = 0;
        public string UserName { get; set; } = string.Empty;
        public string Title { get; set; } = string.Empty;
        public string CreatedDate { get; set; } = string.Empty;

        public ArticleSummary()
        { }

        public ArticleSummary(fb.protocol._internal.raw.ArticleSummary raw)
        {
            Id = raw.Id;
            User = raw.User;
            UserName = raw.UserName;
            Title = raw.Title;
            CreatedDate = raw.CreatedDate;
        }

        public ArticleSummary(byte[] bytes) : this(fb.protocol._internal.raw.ArticleSummary.GetRootAsArticleSummary(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ArticleSummary Deserialize(byte[] bytes)
        {
            return new ArticleSummary(bytes);
        }
    }
    public class Article : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Article;
        public uint Id { get; set; } = 0;
        public uint User { get; set; } = 0;
        public string UserName { get; set; } = string.Empty;
        public string Title { get; set; } = string.Empty;
        public string Contents { get; set; } = string.Empty;
        public string CreatedDate { get; set; } = string.Empty;

        public Article()
        { }

        public Article(fb.protocol._internal.raw.Article raw)
        {
            Id = raw.Id;
            User = raw.User;
            UserName = raw.UserName;
            Title = raw.Title;
            Contents = raw.Contents;
            CreatedDate = raw.CreatedDate;
        }

        public Article(byte[] bytes) : this(fb.protocol._internal.raw.Article.GetRootAsArticle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Article Deserialize(byte[] bytes)
        {
            return new Article(bytes);
        }
    }
    public class Group : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Group;
        public uint Id { get; set; } = 0;
        public string Master { get; set; } = string.Empty;
        public List<string> Members { get; set; } = new List<string>();

        public Group()
        { }

        public Group(fb.protocol._internal.raw.Group raw)
        {
            Id = raw.Id;
            Master = raw.Master;
            Members = Enumerable.Range(0, raw.MembersLength).Select(i => raw.Members(i)).Select(x => x).ToList();
        }

        public Group(byte[] bytes) : this(fb.protocol._internal.raw.Group.GetRootAsGroup(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Group Deserialize(byte[] bytes)
        {
            return new Group(bytes);
        }
    }
    public class Trace : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Trace;
        public uint User { get; set; } = 0;
        public uint Model { get; set; } = 0;
        public string Text { get; set; } = null;

        public Trace()
        { }

        public Trace(fb.protocol._internal.raw.Trace raw)
        {
            User = raw.User;
            Model = raw.Model;
            Text = raw.Text;
        }

        public Trace(byte[] bytes) : this(fb.protocol._internal.raw.Trace.GetRootAsTrace(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Trace Deserialize(byte[] bytes)
        {
            return new Trace(bytes);
        }
    }
    public class Clan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Clan;
        public uint Id { get; set; } = 0;
        public string Name { get; set; } = string.Empty;
        public string Title { get; set; } = null;

        public Clan()
        { }

        public Clan(fb.protocol._internal.raw.Clan raw)
        {
            Id = raw.Id;
            Name = raw.Name;
            Title = raw.Title;
        }

        public Clan(byte[] bytes) : this(fb.protocol._internal.raw.Clan.GetRootAsClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Clan Deserialize(byte[] bytes)
        {
            return new Clan(bytes);
        }
    }
    public class ClanMember : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ClanMember;
        public uint Uid { get; set; } = 0;
        public string Name { get; set; } = string.Empty;
        public uint Position { get; set; } = 0;

        public ClanMember()
        { }

        public ClanMember(fb.protocol._internal.raw.ClanMember raw)
        {
            Uid = raw.Uid;
            Name = raw.Name;
            Position = raw.Position;
        }

        public ClanMember(byte[] bytes) : this(fb.protocol._internal.raw.ClanMember.GetRootAsClanMember(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ClanMember Deserialize(byte[] bytes)
        {
            return new ClanMember(bytes);
        }
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Position => typeof(fb.protocol._internal.Position),
                FlatBufferProtocolType.Character => typeof(fb.protocol._internal.Character),
                FlatBufferProtocolType.Item => typeof(fb.protocol._internal.Item),
                FlatBufferProtocolType.Spell => typeof(fb.protocol._internal.Spell),
                FlatBufferProtocolType.Option => typeof(fb.protocol._internal.Option),
                FlatBufferProtocolType.ArticleSummary => typeof(fb.protocol._internal.ArticleSummary),
                FlatBufferProtocolType.Article => typeof(fb.protocol._internal.Article),
                FlatBufferProtocolType.Group => typeof(fb.protocol._internal.Group),
                FlatBufferProtocolType.Trace => typeof(fb.protocol._internal.Trace),
                FlatBufferProtocolType.Clan => typeof(fb.protocol._internal.Clan),
                FlatBufferProtocolType.ClanMember => typeof(fb.protocol._internal.ClanMember),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol._internal.request
{

    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public uint Uid { get; set; } = 0;
        public string Name { get; set; } = string.Empty;
        public byte Host { get; set; } = 0;

        public Login()
        { }

        public Login(fb.protocol._internal.request.raw.Login raw)
        {
            Uid = raw.Uid;
            Name = raw.Name;
            Host = raw.Host;
        }

        public Login(byte[] bytes) : this(fb.protocol._internal.request.raw.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Login Deserialize(byte[] bytes)
        {
            return new Login(bytes);
        }
    }
    public class Logout : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Logout;
        public string Name { get; set; } = string.Empty;

        public Logout()
        { }

        public Logout(fb.protocol._internal.request.raw.Logout raw)
        {
            Name = raw.Name;
        }

        public Logout(byte[] bytes) : this(fb.protocol._internal.request.raw.Logout.GetRootAsLogout(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Logout Deserialize(byte[] bytes)
        {
            return new Logout(bytes);
        }
    }
    public class Ping : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Ping;
        public byte Id { get; set; } = 0;
        public string Name { get; set; } = string.Empty;
        public fb.protocol._internal.Service Service { get; set; }
        public string Ip { get; set; } = string.Empty;
        public ushort Port { get; set; } = 0;

        public Ping()
        { }

        public Ping(fb.protocol._internal.request.raw.Ping raw)
        {
            Id = raw.Id;
            Name = raw.Name;
            Service = (fb.protocol._internal.Service)raw.Service;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Ping(byte[] bytes) : this(fb.protocol._internal.request.raw.Ping.GetRootAsPing(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Ping Deserialize(byte[] bytes)
        {
            return new Ping(bytes);
        }
    }
    public class Transfer : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Transfer;
        public fb.protocol._internal.Service Service { get; set; }
        public byte Id { get; set; } = 0;
        public string Name { get; set; } = null;
        public bool ForceShutdown { get; set; } = false;

        public Transfer()
        { }

        public Transfer(fb.protocol._internal.request.raw.Transfer raw)
        {
            Service = (fb.protocol._internal.Service)raw.Service;
            Id = raw.Id;
            Name = raw.Name;
            ForceShutdown = raw.ForceShutdown;
        }

        public Transfer(byte[] bytes) : this(fb.protocol._internal.request.raw.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Transfer Deserialize(byte[] bytes)
        {
            return new Transfer(bytes);
        }
    }
    public class Whisper : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Whisper;
        public string From { get; set; } = string.Empty;
        public string To { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;

        public Whisper()
        { }

        public Whisper(fb.protocol._internal.request.raw.Whisper raw)
        {
            From = raw.From;
            To = raw.To;
            Message = raw.Message;
        }

        public Whisper(byte[] bytes) : this(fb.protocol._internal.request.raw.Whisper.GetRootAsWhisper(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Whisper Deserialize(byte[] bytes)
        {
            return new Whisper(bytes);
        }
    }
    public class KickOut : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.KickOut;
        public uint Uid { get; set; } = 0;

        public KickOut()
        { }

        public KickOut(fb.protocol._internal.request.raw.KickOut raw)
        {
            Uid = raw.Uid;
        }

        public KickOut(byte[] bytes) : this(fb.protocol._internal.request.raw.KickOut.GetRootAsKickOut(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static KickOut Deserialize(byte[] bytes)
        {
            return new KickOut(bytes);
        }
    }
    public class Account : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Account;
        public uint Uid { get; set; } = 0;

        public Account()
        { }

        public Account(fb.protocol._internal.request.raw.Account raw)
        {
            Uid = raw.Uid;
        }

        public Account(byte[] bytes) : this(fb.protocol._internal.request.raw.Account.GetRootAsAccount(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Account Deserialize(byte[] bytes)
        {
            return new Account(bytes);
        }
    }
    public class ChangePw : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ChangePw;
        public uint Uid { get; set; } = 0;
        public string Before { get; set; } = string.Empty;
        public string After { get; set; } = string.Empty;
        public uint Birthday { get; set; } = 0;

        public ChangePw()
        { }

        public ChangePw(fb.protocol._internal.request.raw.ChangePw raw)
        {
            Uid = raw.Uid;
            Before = raw.Before;
            After = raw.After;
            Birthday = raw.Birthday;
        }

        public ChangePw(byte[] bytes) : this(fb.protocol._internal.request.raw.ChangePw.GetRootAsChangePw(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ChangePw Deserialize(byte[] bytes)
        {
            return new ChangePw(bytes);
        }
    }
    public class InitCharacter : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.InitCharacter;
        public uint Uid { get; set; } = 0;
        public string Name { get; set; } = string.Empty;
        public string Pw { get; set; } = string.Empty;
        public uint Hp { get; set; } = 0;
        public uint Mp { get; set; } = 0;
        public ushort Map { get; set; } = 0;
        public ushort X { get; set; } = 0;
        public ushort Y { get; set; } = 0;
        public bool Admin { get; set; } = false;

        public InitCharacter()
        { }

        public InitCharacter(fb.protocol._internal.request.raw.InitCharacter raw)
        {
            Uid = raw.Uid;
            Name = raw.Name;
            Pw = raw.Pw;
            Hp = raw.Hp;
            Mp = raw.Mp;
            Map = raw.Map;
            X = raw.X;
            Y = raw.Y;
            Admin = raw.Admin;
        }

        public InitCharacter(byte[] bytes) : this(fb.protocol._internal.request.raw.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static InitCharacter Deserialize(byte[] bytes)
        {
            return new InitCharacter(bytes);
        }
    }
    public class Authenticate : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Authenticate;
        public uint Uid { get; set; } = 0;
        public string Pw { get; set; } = string.Empty;

        public Authenticate()
        { }

        public Authenticate(fb.protocol._internal.request.raw.Authenticate raw)
        {
            Uid = raw.Uid;
            Pw = raw.Pw;
        }

        public Authenticate(byte[] bytes) : this(fb.protocol._internal.request.raw.Authenticate.GetRootAsAuthenticate(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Authenticate Deserialize(byte[] bytes)
        {
            return new Authenticate(bytes);
        }
    }
    public class Init : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Init;
        public uint Uid { get; set; } = 0;

        public Init()
        { }

        public Init(fb.protocol._internal.request.raw.Init raw)
        {
            Uid = raw.Uid;
        }

        public Init(byte[] bytes) : this(fb.protocol._internal.request.raw.Init.GetRootAsInit(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Init Deserialize(byte[] bytes)
        {
            return new Init(bytes);
        }
    }
    public class MakeCharacter : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.MakeCharacter;
        public uint Uid { get; set; } = 0;
        public ushort Hair { get; set; } = 0;
        public byte Sex { get; set; } = 0;
        public byte Nation { get; set; } = 0;
        public byte Creature { get; set; } = 0;

        public MakeCharacter()
        { }

        public MakeCharacter(fb.protocol._internal.request.raw.MakeCharacter raw)
        {
            Uid = raw.Uid;
            Hair = raw.Hair;
            Sex = raw.Sex;
            Nation = raw.Nation;
            Creature = raw.Creature;
        }

        public MakeCharacter(byte[] bytes) : this(fb.protocol._internal.request.raw.MakeCharacter.GetRootAsMakeCharacter(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static MakeCharacter Deserialize(byte[] bytes)
        {
            return new MakeCharacter(bytes);
        }
    }
    public class ReserveName : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ReserveName;
        public string Name { get; set; } = string.Empty;

        public ReserveName()
        { }

        public ReserveName(fb.protocol._internal.request.raw.ReserveName raw)
        {
            Name = raw.Name;
        }

        public ReserveName(byte[] bytes) : this(fb.protocol._internal.request.raw.ReserveName.GetRootAsReserveName(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ReserveName Deserialize(byte[] bytes)
        {
            return new ReserveName(bytes);
        }
    }
    public class Save : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Save;
        public fb.protocol._internal.Character Character { get; set; } = new fb.protocol._internal.Character();
        public List<fb.protocol._internal.Item> Items { get; set; } = new List<fb.protocol._internal.Item>();
        public List<fb.protocol._internal.Spell> Spells { get; set; } = new List<fb.protocol._internal.Spell>();
        public List<fb.protocol._internal.Trace> Traces { get; set; } = new List<fb.protocol._internal.Trace>();

        public Save()
        { }

        public Save(fb.protocol._internal.request.raw.Save raw)
        {
            Character = new Character(raw.Character.Value);
            Items = Enumerable.Range(0, raw.ItemsLength).Select(i => raw.Items(i)).Select(x => new Item(x.Value)).ToList();
            Spells = Enumerable.Range(0, raw.SpellsLength).Select(i => raw.Spells(i)).Select(x => new Spell(x.Value)).ToList();
            Traces = Enumerable.Range(0, raw.TracesLength).Select(i => raw.Traces(i)).Select(x => new Trace(x.Value)).ToList();
        }

        public Save(byte[] bytes) : this(fb.protocol._internal.request.raw.Save.GetRootAsSave(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Save Deserialize(byte[] bytes)
        {
            return new Save(bytes);
        }
    }
    public class GetArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetArticle;
        public uint Section { get; set; } = 0;
        public uint Article { get; set; } = 0;

        public GetArticle()
        { }

        public GetArticle(fb.protocol._internal.request.raw.GetArticle raw)
        {
            Section = raw.Section;
            Article = raw.Article;
        }

        public GetArticle(byte[] bytes) : this(fb.protocol._internal.request.raw.GetArticle.GetRootAsGetArticle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetArticle Deserialize(byte[] bytes)
        {
            return new GetArticle(bytes);
        }
    }
    public class GetArticleList : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetArticleList;
        public uint Section { get; set; } = 0;
        public uint Position { get; set; } = 0;

        public GetArticleList()
        { }

        public GetArticleList(fb.protocol._internal.request.raw.GetArticleList raw)
        {
            Section = raw.Section;
            Position = raw.Position;
        }

        public GetArticleList(byte[] bytes) : this(fb.protocol._internal.request.raw.GetArticleList.GetRootAsGetArticleList(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetArticleList Deserialize(byte[] bytes)
        {
            return new GetArticleList(bytes);
        }
    }
    public class WriteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.WriteArticle;
        public uint Section { get; set; } = 0;
        public uint User { get; set; } = 0;
        public string Title { get; set; } = string.Empty;
        public string Contents { get; set; } = string.Empty;

        public WriteArticle()
        { }

        public WriteArticle(fb.protocol._internal.request.raw.WriteArticle raw)
        {
            Section = raw.Section;
            User = raw.User;
            Title = raw.Title;
            Contents = raw.Contents;
        }

        public WriteArticle(byte[] bytes) : this(fb.protocol._internal.request.raw.WriteArticle.GetRootAsWriteArticle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static WriteArticle Deserialize(byte[] bytes)
        {
            return new WriteArticle(bytes);
        }
    }
    public class DeleteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.DeleteArticle;
        public uint Id { get; set; } = 0;
        public uint User { get; set; } = 0;

        public DeleteArticle()
        { }

        public DeleteArticle(fb.protocol._internal.request.raw.DeleteArticle raw)
        {
            Id = raw.Id;
            User = raw.User;
        }

        public DeleteArticle(byte[] bytes) : this(fb.protocol._internal.request.raw.DeleteArticle.GetRootAsDeleteArticle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static DeleteArticle Deserialize(byte[] bytes)
        {
            return new DeleteArticle(bytes);
        }
    }
    public class SetOption : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.SetOption;
        public uint User { get; set; } = 0;
        public byte Type { get; set; } = 0;
        public bool Enabled { get; set; } = false;

        public SetOption()
        { }

        public SetOption(fb.protocol._internal.request.raw.SetOption raw)
        {
            User = raw.User;
            Type = raw.Type;
            Enabled = raw.Enabled;
        }

        public SetOption(byte[] bytes) : this(fb.protocol._internal.request.raw.SetOption.GetRootAsSetOption(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static SetOption Deserialize(byte[] bytes)
        {
            return new SetOption(bytes);
        }
    }
    public class EnterGroup : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.EnterGroup;
        public uint Master { get; set; } = 0;
        public string Member { get; set; } = string.Empty;

        public EnterGroup()
        { }

        public EnterGroup(fb.protocol._internal.request.raw.EnterGroup raw)
        {
            Master = raw.Master;
            Member = raw.Member;
        }

        public EnterGroup(byte[] bytes) : this(fb.protocol._internal.request.raw.EnterGroup.GetRootAsEnterGroup(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static EnterGroup Deserialize(byte[] bytes)
        {
            return new EnterGroup(bytes);
        }
    }
    public class LeaveGroup : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.LeaveGroup;
        public string Member { get; set; } = string.Empty;

        public LeaveGroup()
        { }

        public LeaveGroup(fb.protocol._internal.request.raw.LeaveGroup raw)
        {
            Member = raw.Member;
        }

        public LeaveGroup(byte[] bytes) : this(fb.protocol._internal.request.raw.LeaveGroup.GetRootAsLeaveGroup(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static LeaveGroup Deserialize(byte[] bytes)
        {
            return new LeaveGroup(bytes);
        }
    }
    public class CreateClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.CreateClan;
        public uint Master { get; set; } = 0;
        public string Name { get; set; } = string.Empty;

        public CreateClan()
        { }

        public CreateClan(fb.protocol._internal.request.raw.CreateClan raw)
        {
            Master = raw.Master;
            Name = raw.Name;
        }

        public CreateClan(byte[] bytes) : this(fb.protocol._internal.request.raw.CreateClan.GetRootAsCreateClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static CreateClan Deserialize(byte[] bytes)
        {
            return new CreateClan(bytes);
        }
    }
    public class DestroyClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.DestroyClan;
        public uint Master { get; set; } = 0;

        public DestroyClan()
        { }

        public DestroyClan(fb.protocol._internal.request.raw.DestroyClan raw)
        {
            Master = raw.Master;
        }

        public DestroyClan(byte[] bytes) : this(fb.protocol._internal.request.raw.DestroyClan.GetRootAsDestroyClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static DestroyClan Deserialize(byte[] bytes)
        {
            return new DestroyClan(bytes);
        }
    }
    public class JoinClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.JoinClan;
        public uint Master { get; set; } = 0;
        public uint Uid { get; set; } = 0;

        public JoinClan()
        { }

        public JoinClan(fb.protocol._internal.request.raw.JoinClan raw)
        {
            Master = raw.Master;
            Uid = raw.Uid;
        }

        public JoinClan(byte[] bytes) : this(fb.protocol._internal.request.raw.JoinClan.GetRootAsJoinClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static JoinClan Deserialize(byte[] bytes)
        {
            return new JoinClan(bytes);
        }
    }
    public class LeaveClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.LeaveClan;
        public uint Uid { get; set; } = 0;

        public LeaveClan()
        { }

        public LeaveClan(fb.protocol._internal.request.raw.LeaveClan raw)
        {
            Uid = raw.Uid;
        }

        public LeaveClan(byte[] bytes) : this(fb.protocol._internal.request.raw.LeaveClan.GetRootAsLeaveClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static LeaveClan Deserialize(byte[] bytes)
        {
            return new LeaveClan(bytes);
        }
    }
    public class KickClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.KickClan;
        public uint Master { get; set; } = 0;
        public uint Uid { get; set; } = 0;

        public KickClan()
        { }

        public KickClan(fb.protocol._internal.request.raw.KickClan raw)
        {
            Master = raw.Master;
            Uid = raw.Uid;
        }

        public KickClan(byte[] bytes) : this(fb.protocol._internal.request.raw.KickClan.GetRootAsKickClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static KickClan Deserialize(byte[] bytes)
        {
            return new KickClan(bytes);
        }
    }
    public class SetClanTitle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.SetClanTitle;
        public uint Clan { get; set; } = 0;
        public string Title { get; set; } = null;

        public SetClanTitle()
        { }

        public SetClanTitle(fb.protocol._internal.request.raw.SetClanTitle raw)
        {
            Clan = raw.Clan;
            Title = raw.Title;
        }

        public SetClanTitle(byte[] bytes) : this(fb.protocol._internal.request.raw.SetClanTitle.GetRootAsSetClanTitle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static SetClanTitle Deserialize(byte[] bytes)
        {
            return new SetClanTitle(bytes);
        }
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Login => typeof(fb.protocol._internal.request.Login),
                FlatBufferProtocolType.Logout => typeof(fb.protocol._internal.request.Logout),
                FlatBufferProtocolType.Ping => typeof(fb.protocol._internal.request.Ping),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol._internal.request.Transfer),
                FlatBufferProtocolType.Whisper => typeof(fb.protocol._internal.request.Whisper),
                FlatBufferProtocolType.KickOut => typeof(fb.protocol._internal.request.KickOut),
                FlatBufferProtocolType.Account => typeof(fb.protocol._internal.request.Account),
                FlatBufferProtocolType.ChangePw => typeof(fb.protocol._internal.request.ChangePw),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol._internal.request.InitCharacter),
                FlatBufferProtocolType.Authenticate => typeof(fb.protocol._internal.request.Authenticate),
                FlatBufferProtocolType.Init => typeof(fb.protocol._internal.request.Init),
                FlatBufferProtocolType.MakeCharacter => typeof(fb.protocol._internal.request.MakeCharacter),
                FlatBufferProtocolType.ReserveName => typeof(fb.protocol._internal.request.ReserveName),
                FlatBufferProtocolType.Save => typeof(fb.protocol._internal.request.Save),
                FlatBufferProtocolType.GetArticle => typeof(fb.protocol._internal.request.GetArticle),
                FlatBufferProtocolType.GetArticleList => typeof(fb.protocol._internal.request.GetArticleList),
                FlatBufferProtocolType.WriteArticle => typeof(fb.protocol._internal.request.WriteArticle),
                FlatBufferProtocolType.DeleteArticle => typeof(fb.protocol._internal.request.DeleteArticle),
                FlatBufferProtocolType.SetOption => typeof(fb.protocol._internal.request.SetOption),
                FlatBufferProtocolType.EnterGroup => typeof(fb.protocol._internal.request.EnterGroup),
                FlatBufferProtocolType.LeaveGroup => typeof(fb.protocol._internal.request.LeaveGroup),
                FlatBufferProtocolType.CreateClan => typeof(fb.protocol._internal.request.CreateClan),
                FlatBufferProtocolType.DestroyClan => typeof(fb.protocol._internal.request.DestroyClan),
                FlatBufferProtocolType.JoinClan => typeof(fb.protocol._internal.request.JoinClan),
                FlatBufferProtocolType.LeaveClan => typeof(fb.protocol._internal.request.LeaveClan),
                FlatBufferProtocolType.KickClan => typeof(fb.protocol._internal.request.KickClan),
                FlatBufferProtocolType.SetClanTitle => typeof(fb.protocol._internal.request.SetClanTitle),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol._internal.response
{

    public class KickOut : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.KickOut;
        public uint Uid { get; set; } = 0;
        public string Name { get; set; } = string.Empty;

        public KickOut()
        { }

        public KickOut(fb.protocol._internal.response.raw.KickOut raw)
        {
            Uid = raw.Uid;
            Name = raw.Name;
        }

        public KickOut(byte[] bytes) : this(fb.protocol._internal.response.raw.KickOut.GetRootAsKickOut(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static KickOut Deserialize(byte[] bytes)
        {
            return new KickOut(bytes);
        }
    }
    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public uint Error { get; set; } = 0;
        public bool Logon { get; set; } = false;
        public string Ip { get; set; } = string.Empty;
        public ushort Port { get; set; } = 0;

        public Login()
        { }

        public Login(fb.protocol._internal.response.raw.Login raw)
        {
            Error = raw.Error;
            Logon = raw.Logon;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Login(byte[] bytes) : this(fb.protocol._internal.response.raw.Login.GetRootAsLogin(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Login Deserialize(byte[] bytes)
        {
            return new Login(bytes);
        }
    }
    public class Logout : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Logout;
        public bool Success { get; set; } = false;

        public Logout()
        { }

        public Logout(fb.protocol._internal.response.raw.Logout raw)
        {
            Success = raw.Success;
        }

        public Logout(byte[] bytes) : this(fb.protocol._internal.response.raw.Logout.GetRootAsLogout(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Logout Deserialize(byte[] bytes)
        {
            return new Logout(bytes);
        }
    }
    public class Pong : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Pong;

        public Pong()
        { }

        public Pong(fb.protocol._internal.response.raw.Pong raw)
        {
        }

        public Pong(byte[] bytes) : this(fb.protocol._internal.response.raw.Pong.GetRootAsPong(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Pong Deserialize(byte[] bytes)
        {
            return new Pong(bytes);
        }
    }
    public class Transfer : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Transfer;
        public uint Error { get; set; } = 0;
        public string Ip { get; set; } = string.Empty;
        public ushort Port { get; set; } = 0;

        public Transfer()
        { }

        public Transfer(fb.protocol._internal.response.raw.Transfer raw)
        {
            Error = raw.Error;
            Ip = raw.Ip;
            Port = raw.Port;
        }

        public Transfer(byte[] bytes) : this(fb.protocol._internal.response.raw.Transfer.GetRootAsTransfer(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Transfer Deserialize(byte[] bytes)
        {
            return new Transfer(bytes);
        }
    }
    public class Whisper : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Whisper;
        public string From { get; set; } = string.Empty;
        public string To { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;
        public uint Host { get; set; } = 0;
        public uint Error { get; set; } = 0;

        public Whisper()
        { }

        public Whisper(fb.protocol._internal.response.raw.Whisper raw)
        {
            From = raw.From;
            To = raw.To;
            Message = raw.Message;
            Host = raw.Host;
            Error = raw.Error;
        }

        public Whisper(byte[] bytes) : this(fb.protocol._internal.response.raw.Whisper.GetRootAsWhisper(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Whisper Deserialize(byte[] bytes)
        {
            return new Whisper(bytes);
        }
    }
    public class DeleteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.DeleteArticle;
        public int Result { get; set; } = 0;

        public DeleteArticle()
        { }

        public DeleteArticle(fb.protocol._internal.response.raw.DeleteArticle raw)
        {
            Result = raw.Result;
        }

        public DeleteArticle(byte[] bytes) : this(fb.protocol._internal.response.raw.DeleteArticle.GetRootAsDeleteArticle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static DeleteArticle Deserialize(byte[] bytes)
        {
            return new DeleteArticle(bytes);
        }
    }
    public class GetArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetArticle;
        public fb.protocol._internal.Article Article { get; set; } = new fb.protocol._internal.Article();
        public bool Next { get; set; } = false;
        public bool Success { get; set; } = false;

        public GetArticle()
        { }

        public GetArticle(fb.protocol._internal.response.raw.GetArticle raw)
        {
            Article = new Article(raw.Article.Value);
            Next = raw.Next;
            Success = raw.Success;
        }

        public GetArticle(byte[] bytes) : this(fb.protocol._internal.response.raw.GetArticle.GetRootAsGetArticle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetArticle Deserialize(byte[] bytes)
        {
            return new GetArticle(bytes);
        }
    }
    public class GetArticleList : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetArticleList;
        public List<fb.protocol._internal.ArticleSummary> SummaryList { get; set; } = new List<fb.protocol._internal.ArticleSummary>();

        public GetArticleList()
        { }

        public GetArticleList(fb.protocol._internal.response.raw.GetArticleList raw)
        {
            SummaryList = Enumerable.Range(0, raw.SummaryListLength).Select(i => raw.SummaryList(i)).Select(x => new ArticleSummary(x.Value)).ToList();
        }

        public GetArticleList(byte[] bytes) : this(fb.protocol._internal.response.raw.GetArticleList.GetRootAsGetArticleList(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetArticleList Deserialize(byte[] bytes)
        {
            return new GetArticleList(bytes);
        }
    }
    public class WriteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.WriteArticle;
        public bool Success { get; set; } = false;

        public WriteArticle()
        { }

        public WriteArticle(fb.protocol._internal.response.raw.WriteArticle raw)
        {
            Success = raw.Success;
        }

        public WriteArticle(byte[] bytes) : this(fb.protocol._internal.response.raw.WriteArticle.GetRootAsWriteArticle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static WriteArticle Deserialize(byte[] bytes)
        {
            return new WriteArticle(bytes);
        }
    }
    public class ChangePw : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ChangePw;
        public uint ErrorCode { get; set; } = 0;

        public ChangePw()
        { }

        public ChangePw(fb.protocol._internal.response.raw.ChangePw raw)
        {
            ErrorCode = raw.ErrorCode;
        }

        public ChangePw(byte[] bytes) : this(fb.protocol._internal.response.raw.ChangePw.GetRootAsChangePw(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ChangePw Deserialize(byte[] bytes)
        {
            return new ChangePw(bytes);
        }
    }
    public class GetUid : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetUid;
        public uint Uid { get; set; } = 0;
        public bool Success { get; set; } = false;

        public GetUid()
        { }

        public GetUid(fb.protocol._internal.response.raw.GetUid raw)
        {
            Uid = raw.Uid;
            Success = raw.Success;
        }

        public GetUid(byte[] bytes) : this(fb.protocol._internal.response.raw.GetUid.GetRootAsGetUid(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetUid Deserialize(byte[] bytes)
        {
            return new GetUid(bytes);
        }
    }
    public class InitCharacter : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.InitCharacter;
        public bool Success { get; set; } = false;

        public InitCharacter()
        { }

        public InitCharacter(fb.protocol._internal.response.raw.InitCharacter raw)
        {
            Success = raw.Success;
        }

        public InitCharacter(byte[] bytes) : this(fb.protocol._internal.response.raw.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static InitCharacter Deserialize(byte[] bytes)
        {
            return new InitCharacter(bytes);
        }
    }
    public class Authenticate : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Authenticate;
        public uint Map { get; set; } = 0;
        public uint ErrorCode { get; set; } = 0;

        public Authenticate()
        { }

        public Authenticate(fb.protocol._internal.response.raw.Authenticate raw)
        {
            Map = raw.Map;
            ErrorCode = raw.ErrorCode;
        }

        public Authenticate(byte[] bytes) : this(fb.protocol._internal.response.raw.Authenticate.GetRootAsAuthenticate(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Authenticate Deserialize(byte[] bytes)
        {
            return new Authenticate(bytes);
        }
    }
    public class Init : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Init;
        public fb.protocol._internal.Character Character { get; set; } = new fb.protocol._internal.Character();
        public uint? Group { get; set; } = null;
        public uint? Clan { get; set; } = null;
        public List<fb.protocol._internal.Item> Items { get; set; } = new List<fb.protocol._internal.Item>();
        public List<fb.protocol._internal.Spell> Spells { get; set; } = new List<fb.protocol._internal.Spell>();
        public fb.protocol._internal.Option Option { get; set; } = new fb.protocol._internal.Option();
        public List<fb.protocol._internal.Trace> Traces { get; set; } = new List<fb.protocol._internal.Trace>();

        public Init()
        { }

        public Init(fb.protocol._internal.response.raw.Init raw)
        {
            Character = new Character(raw.Character.Value);
            Group = raw.Group != null ? (uint?)raw.Group.Value.Value : null;
            Clan = raw.Clan != null ? (uint?)raw.Clan.Value.Value : null;
            Items = Enumerable.Range(0, raw.ItemsLength).Select(i => raw.Items(i)).Select(x => new Item(x.Value)).ToList();
            Spells = Enumerable.Range(0, raw.SpellsLength).Select(i => raw.Spells(i)).Select(x => new Spell(x.Value)).ToList();
            Option = new Option(raw.Option.Value);
            Traces = Enumerable.Range(0, raw.TracesLength).Select(i => raw.Traces(i)).Select(x => new Trace(x.Value)).ToList();
        }

        public Init(byte[] bytes) : this(fb.protocol._internal.response.raw.Init.GetRootAsInit(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Init Deserialize(byte[] bytes)
        {
            return new Init(bytes);
        }
    }
    public class MakeCharacter : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.MakeCharacter;
        public bool Success { get; set; } = false;

        public MakeCharacter()
        { }

        public MakeCharacter(fb.protocol._internal.response.raw.MakeCharacter raw)
        {
            Success = raw.Success;
        }

        public MakeCharacter(byte[] bytes) : this(fb.protocol._internal.response.raw.MakeCharacter.GetRootAsMakeCharacter(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static MakeCharacter Deserialize(byte[] bytes)
        {
            return new MakeCharacter(bytes);
        }
    }
    public class ReserveName : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.ReserveName;
        public uint Uid { get; set; } = 0;
        public bool Success { get; set; } = false;

        public ReserveName()
        { }

        public ReserveName(fb.protocol._internal.response.raw.ReserveName raw)
        {
            Uid = raw.Uid;
            Success = raw.Success;
        }

        public ReserveName(byte[] bytes) : this(fb.protocol._internal.response.raw.ReserveName.GetRootAsReserveName(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static ReserveName Deserialize(byte[] bytes)
        {
            return new ReserveName(bytes);
        }
    }
    public class Save : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Save;
        public bool Success { get; set; } = false;

        public Save()
        { }

        public Save(fb.protocol._internal.response.raw.Save raw)
        {
            Success = raw.Success;
        }

        public Save(byte[] bytes) : this(fb.protocol._internal.response.raw.Save.GetRootAsSave(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static Save Deserialize(byte[] bytes)
        {
            return new Save(bytes);
        }
    }
    public class SetOption : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.SetOption;
        public bool Success { get; set; } = false;

        public SetOption()
        { }

        public SetOption(fb.protocol._internal.response.raw.SetOption raw)
        {
            Success = raw.Success;
        }

        public SetOption(byte[] bytes) : this(fb.protocol._internal.response.raw.SetOption.GetRootAsSetOption(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static SetOption Deserialize(byte[] bytes)
        {
            return new SetOption(bytes);
        }
    }
    public class GetGroup : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetGroup;
        public fb.protocol._internal.Group Group { get; set; } = new fb.protocol._internal.Group();
        public uint Error { get; set; } = 0;

        public GetGroup()
        { }

        public GetGroup(fb.protocol._internal.response.raw.GetGroup raw)
        {
            Group = new Group(raw.Group.Value);
            Error = raw.Error;
        }

        public GetGroup(byte[] bytes) : this(fb.protocol._internal.response.raw.GetGroup.GetRootAsGetGroup(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetGroup Deserialize(byte[] bytes)
        {
            return new GetGroup(bytes);
        }
    }
    public class EnterGroup : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.EnterGroup;
        public fb.protocol._internal.Group Group { get; set; } = new fb.protocol._internal.Group();
        public string Member { get; set; } = string.Empty;
        public fb.protocol._internal.GroupAction Action { get; set; }
        public uint Host { get; set; } = 0;
        public uint Error { get; set; } = 0;

        public EnterGroup()
        { }

        public EnterGroup(fb.protocol._internal.response.raw.EnterGroup raw)
        {
            Group = new Group(raw.Group.Value);
            Member = raw.Member;
            Action = (fb.protocol._internal.GroupAction)raw.Action;
            Host = raw.Host;
            Error = raw.Error;
        }

        public EnterGroup(byte[] bytes) : this(fb.protocol._internal.response.raw.EnterGroup.GetRootAsEnterGroup(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static EnterGroup Deserialize(byte[] bytes)
        {
            return new EnterGroup(bytes);
        }
    }
    public class LeaveGroup : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.LeaveGroup;
        public fb.protocol._internal.Group Group { get; set; } = new fb.protocol._internal.Group();
        public string Member { get; set; } = string.Empty;
        public fb.protocol._internal.GroupAction Action { get; set; }
        public uint Host { get; set; } = 0;
        public uint Error { get; set; } = 0;

        public LeaveGroup()
        { }

        public LeaveGroup(fb.protocol._internal.response.raw.LeaveGroup raw)
        {
            Group = new Group(raw.Group.Value);
            Member = raw.Member;
            Action = (fb.protocol._internal.GroupAction)raw.Action;
            Host = raw.Host;
            Error = raw.Error;
        }

        public LeaveGroup(byte[] bytes) : this(fb.protocol._internal.response.raw.LeaveGroup.GetRootAsLeaveGroup(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static LeaveGroup Deserialize(byte[] bytes)
        {
            return new LeaveGroup(bytes);
        }
    }
    public class GetClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.GetClan;
        public fb.protocol._internal.Clan Clan { get; set; } = new fb.protocol._internal.Clan();
        public List<fb.protocol._internal.ClanMember> Members { get; set; } = new List<fb.protocol._internal.ClanMember>();
        public uint Error { get; set; } = 0;

        public GetClan()
        { }

        public GetClan(fb.protocol._internal.response.raw.GetClan raw)
        {
            Clan = new Clan(raw.Clan.Value);
            Members = Enumerable.Range(0, raw.MembersLength).Select(i => raw.Members(i)).Select(x => new ClanMember(x.Value)).ToList();
            Error = raw.Error;
        }

        public GetClan(byte[] bytes) : this(fb.protocol._internal.response.raw.GetClan.GetRootAsGetClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static GetClan Deserialize(byte[] bytes)
        {
            return new GetClan(bytes);
        }
    }
    public class CreateClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.CreateClan;
        public fb.protocol._internal.Clan Clan { get; set; } = new fb.protocol._internal.Clan();
        public List<fb.protocol._internal.ClanMember> Members { get; set; } = new List<fb.protocol._internal.ClanMember>();
        public uint Error { get; set; } = 0;

        public CreateClan()
        { }

        public CreateClan(fb.protocol._internal.response.raw.CreateClan raw)
        {
            Clan = new Clan(raw.Clan.Value);
            Members = Enumerable.Range(0, raw.MembersLength).Select(i => raw.Members(i)).Select(x => new ClanMember(x.Value)).ToList();
            Error = raw.Error;
        }

        public CreateClan(byte[] bytes) : this(fb.protocol._internal.response.raw.CreateClan.GetRootAsCreateClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static CreateClan Deserialize(byte[] bytes)
        {
            return new CreateClan(bytes);
        }
    }
    public class DestroyClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.DestroyClan;
        public uint Error { get; set; } = 0;

        public DestroyClan()
        { }

        public DestroyClan(fb.protocol._internal.response.raw.DestroyClan raw)
        {
            Error = raw.Error;
        }

        public DestroyClan(byte[] bytes) : this(fb.protocol._internal.response.raw.DestroyClan.GetRootAsDestroyClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static DestroyClan Deserialize(byte[] bytes)
        {
            return new DestroyClan(bytes);
        }
    }
    public class JoinClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.JoinClan;
        public uint Clan { get; set; } = 0;
        public uint Uid { get; set; } = 0;
        public string Uname { get; set; } = string.Empty;
        public uint Error { get; set; } = 0;

        public JoinClan()
        { }

        public JoinClan(fb.protocol._internal.response.raw.JoinClan raw)
        {
            Clan = raw.Clan;
            Uid = raw.Uid;
            Uname = raw.Uname;
            Error = raw.Error;
        }

        public JoinClan(byte[] bytes) : this(fb.protocol._internal.response.raw.JoinClan.GetRootAsJoinClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static JoinClan Deserialize(byte[] bytes)
        {
            return new JoinClan(bytes);
        }
    }
    public class LeaveClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.LeaveClan;
        public uint Clan { get; set; } = 0;
        public uint Uid { get; set; } = 0;
        public string Uname { get; set; } = string.Empty;
        public uint Error { get; set; } = 0;

        public LeaveClan()
        { }

        public LeaveClan(fb.protocol._internal.response.raw.LeaveClan raw)
        {
            Clan = raw.Clan;
            Uid = raw.Uid;
            Uname = raw.Uname;
            Error = raw.Error;
        }

        public LeaveClan(byte[] bytes) : this(fb.protocol._internal.response.raw.LeaveClan.GetRootAsLeaveClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static LeaveClan Deserialize(byte[] bytes)
        {
            return new LeaveClan(bytes);
        }
    }
    public class KickClan : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.KickClan;
        public uint Clan { get; set; } = 0;
        public uint Uid { get; set; } = 0;
        public string Uname { get; set; } = string.Empty;
        public uint Error { get; set; } = 0;

        public KickClan()
        { }

        public KickClan(fb.protocol._internal.response.raw.KickClan raw)
        {
            Clan = raw.Clan;
            Uid = raw.Uid;
            Uname = raw.Uname;
            Error = raw.Error;
        }

        public KickClan(byte[] bytes) : this(fb.protocol._internal.response.raw.KickClan.GetRootAsKickClan(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static KickClan Deserialize(byte[] bytes)
        {
            return new KickClan(bytes);
        }
    }
    public class SetClanTitle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.SetClanTitle;
        public uint Clan { get; set; } = 0;
        public string Title { get; set; } = null;
        public uint Error { get; set; } = 0;

        public SetClanTitle()
        { }

        public SetClanTitle(fb.protocol._internal.response.raw.SetClanTitle raw)
        {
            Clan = raw.Clan;
            Title = raw.Title;
            Error = raw.Error;
        }

        public SetClanTitle(byte[] bytes) : this(fb.protocol._internal.response.raw.SetClanTitle.GetRootAsSetClanTitle(new ByteBuffer(bytes)))
        { }

        public byte[] Serialize()
        {
            var builder = new FlatBufferBuilder(1);
            var offset = builder.Build(this);
            builder.Finish(offset.Value);
            return builder.SizedByteArray();
        }

        public static SetClanTitle Deserialize(byte[] bytes)
        {
            return new SetClanTitle(bytes);
        }
    }

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.KickOut => typeof(fb.protocol._internal.response.KickOut),
                FlatBufferProtocolType.Login => typeof(fb.protocol._internal.response.Login),
                FlatBufferProtocolType.Logout => typeof(fb.protocol._internal.response.Logout),
                FlatBufferProtocolType.Pong => typeof(fb.protocol._internal.response.Pong),
                FlatBufferProtocolType.Transfer => typeof(fb.protocol._internal.response.Transfer),
                FlatBufferProtocolType.Whisper => typeof(fb.protocol._internal.response.Whisper),
                FlatBufferProtocolType.DeleteArticle => typeof(fb.protocol._internal.response.DeleteArticle),
                FlatBufferProtocolType.GetArticle => typeof(fb.protocol._internal.response.GetArticle),
                FlatBufferProtocolType.GetArticleList => typeof(fb.protocol._internal.response.GetArticleList),
                FlatBufferProtocolType.WriteArticle => typeof(fb.protocol._internal.response.WriteArticle),
                FlatBufferProtocolType.ChangePw => typeof(fb.protocol._internal.response.ChangePw),
                FlatBufferProtocolType.GetUid => typeof(fb.protocol._internal.response.GetUid),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol._internal.response.InitCharacter),
                FlatBufferProtocolType.Authenticate => typeof(fb.protocol._internal.response.Authenticate),
                FlatBufferProtocolType.Init => typeof(fb.protocol._internal.response.Init),
                FlatBufferProtocolType.MakeCharacter => typeof(fb.protocol._internal.response.MakeCharacter),
                FlatBufferProtocolType.ReserveName => typeof(fb.protocol._internal.response.ReserveName),
                FlatBufferProtocolType.Save => typeof(fb.protocol._internal.response.Save),
                FlatBufferProtocolType.SetOption => typeof(fb.protocol._internal.response.SetOption),
                FlatBufferProtocolType.GetGroup => typeof(fb.protocol._internal.response.GetGroup),
                FlatBufferProtocolType.EnterGroup => typeof(fb.protocol._internal.response.EnterGroup),
                FlatBufferProtocolType.LeaveGroup => typeof(fb.protocol._internal.response.LeaveGroup),
                FlatBufferProtocolType.GetClan => typeof(fb.protocol._internal.response.GetClan),
                FlatBufferProtocolType.CreateClan => typeof(fb.protocol._internal.response.CreateClan),
                FlatBufferProtocolType.DestroyClan => typeof(fb.protocol._internal.response.DestroyClan),
                FlatBufferProtocolType.JoinClan => typeof(fb.protocol._internal.response.JoinClan),
                FlatBufferProtocolType.LeaveClan => typeof(fb.protocol._internal.response.LeaveClan),
                FlatBufferProtocolType.KickClan => typeof(fb.protocol._internal.response.KickClan),
                FlatBufferProtocolType.SetClanTitle => typeof(fb.protocol._internal.response.SetClanTitle),
                _ => throw new ArgumentException(),
            };
        }
    }
}
