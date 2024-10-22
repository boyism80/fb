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

namespace fb.protocol.db
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
        public static Offset<fb.protocol.db.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol.db.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol.db.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol.db.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.LastLogin),
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
                builder.Build(value.Clan));
        }
        public static Offset<fb.protocol.db.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol.db.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol.db.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol.db.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol.db.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol.db.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol.db.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol.db.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.InitCharacter.CreateInitCharacter(builder,
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
        public static Offset<fb.protocol.db.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol.db.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol.db.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol.db.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol.db.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol.db.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol.db.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol.db.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Map));
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
                builder.Build(value.Id));
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
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Success),
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
                builder.Build(value.Code),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.Success),
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Item> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Spell> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.ArticleSummary> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.TransferResult Build(this FlatBufferBuilder builder, fb.protocol._internal.TransferResult value)
        {
            return (fb.protocol._internal.raw.TransferResult)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
        Position,
        Character,
        Item,
        Spell,
        ArticleSummary,
        Article
    }
}
namespace fb.protocol.db.request
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
        public static Offset<fb.protocol.db.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol.db.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol.db.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol.db.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.LastLogin),
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
                builder.Build(value.Clan));
        }
        public static Offset<fb.protocol.db.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol.db.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol.db.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol.db.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol.db.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol.db.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol.db.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol.db.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.InitCharacter.CreateInitCharacter(builder,
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
        public static Offset<fb.protocol.db.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol.db.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol.db.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol.db.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol.db.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol.db.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol.db.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol.db.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Map));
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
                builder.Build(value.Id));
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
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Success),
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
                builder.Build(value.Code),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.Success),
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Item> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Spell> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.ArticleSummary> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.TransferResult Build(this FlatBufferBuilder builder, fb.protocol._internal.TransferResult value)
        {
            return (fb.protocol._internal.raw.TransferResult)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
        Account,
        ChangePw,
        InitCharacter,
        Authenticate,
        Login,
        MakeCharacter,
        ReserveName,
        Save,
        GetArticle,
        GetArticleList,
        WriteArticle,
        DeleteArticle
    }
}
namespace fb.protocol.db.response
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
        public static Offset<fb.protocol.db.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol.db.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol.db.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol.db.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.LastLogin),
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
                builder.Build(value.Clan));
        }
        public static Offset<fb.protocol.db.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol.db.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol.db.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol.db.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol.db.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol.db.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol.db.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol.db.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.InitCharacter.CreateInitCharacter(builder,
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
        public static Offset<fb.protocol.db.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol.db.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol.db.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol.db.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol.db.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol.db.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol.db.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol.db.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Map));
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
                builder.Build(value.Id));
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
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Success),
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
                builder.Build(value.Code),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.Success),
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Item> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Spell> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.ArticleSummary> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.TransferResult Build(this FlatBufferBuilder builder, fb.protocol._internal.TransferResult value)
        {
            return (fb.protocol._internal.raw.TransferResult)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
        DeleteArticle,
        GetArticle,
        GetArticleList,
        WriteArticle,
        ChangePw,
        GetUid,
        InitCharacter,
        Authenticate,
        Login,
        MakeCharacter,
        ReserveName,
        Save
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
        public static Offset<fb.protocol.db.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol.db.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol.db.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol.db.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.LastLogin),
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
                builder.Build(value.Clan));
        }
        public static Offset<fb.protocol.db.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol.db.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol.db.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol.db.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol.db.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol.db.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol.db.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol.db.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.InitCharacter.CreateInitCharacter(builder,
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
        public static Offset<fb.protocol.db.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol.db.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol.db.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol.db.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol.db.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol.db.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol.db.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol.db.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Map));
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
                builder.Build(value.Id));
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
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Success),
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
                builder.Build(value.Code),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.Success),
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Item> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Spell> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.ArticleSummary> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.TransferResult Build(this FlatBufferBuilder builder, fb.protocol._internal.TransferResult value)
        {
            return (fb.protocol._internal.raw.TransferResult)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
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
        public static Offset<fb.protocol.db.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol.db.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol.db.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol.db.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.LastLogin),
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
                builder.Build(value.Clan));
        }
        public static Offset<fb.protocol.db.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol.db.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol.db.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol.db.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol.db.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol.db.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol.db.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol.db.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.InitCharacter.CreateInitCharacter(builder,
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
        public static Offset<fb.protocol.db.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol.db.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol.db.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol.db.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol.db.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol.db.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol.db.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol.db.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Map));
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
                builder.Build(value.Id));
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
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Success),
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
                builder.Build(value.Code),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.Success),
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Item> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Spell> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.ArticleSummary> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.TransferResult Build(this FlatBufferBuilder builder, fb.protocol._internal.TransferResult value)
        {
            return (fb.protocol._internal.raw.TransferResult)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
        Login,
        Logout,
        Ping,
        Transfer,
        Whisper,
        KickOut
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
        public static Offset<fb.protocol.db.raw.Position> Build(this FlatBufferBuilder builder, fb.protocol.db.Position value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Position.CreatePosition(builder,
                builder.Build(value.X),
                builder.Build(value.Y));
        }
        public static Offset<fb.protocol.db.raw.Character> Build(this FlatBufferBuilder builder, fb.protocol.db.Character value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Character.CreateCharacter(builder,
                builder.Build(value.Id),
                builder.Build(value.Name),
                builder.Build(value.LastLogin),
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
                builder.Build(value.Clan));
        }
        public static Offset<fb.protocol.db.raw.Item> Build(this FlatBufferBuilder builder, fb.protocol.db.Item value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Item.CreateItem(builder,
                builder.Build(value.User),
                builder.Build(value.Index),
                builder.Build(value.Parts),
                builder.Build(value.Deposited),
                builder.Build(value.Model),
                builder.Build(value.Count),
                builder.Build(value.Durability),
                builder.Build(value.CustomName));
        }
        public static Offset<fb.protocol.db.raw.Spell> Build(this FlatBufferBuilder builder, fb.protocol.db.Spell value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Spell.CreateSpell(builder,
                builder.Build(value.User),
                builder.Build(value.Slot),
                builder.Build(value.Model));
        }
        public static Offset<fb.protocol.db.raw.ArticleSummary> Build(this FlatBufferBuilder builder, fb.protocol.db.ArticleSummary value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.ArticleSummary.CreateArticleSummary(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.raw.Article> Build(this FlatBufferBuilder builder, fb.protocol.db.Article value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.raw.Article.CreateArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User),
                builder.Build(value.UserName),
                builder.Build(value.Title),
                builder.Build(value.Contents),
                builder.Build(value.CreatedDate));
        }
        public static Offset<fb.protocol.db.request.raw.Account> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Account value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Account.CreateAccount(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.Uid),
                builder.Build(value.Before),
                builder.Build(value.After),
                builder.Build(value.Birthday));
        }
        public static Offset<fb.protocol.db.request.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.InitCharacter.CreateInitCharacter(builder,
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
        public static Offset<fb.protocol.db.request.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Uid),
                builder.Build(value.Pw));
        }
        public static Offset<fb.protocol.db.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol.db.request.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.request.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Uid),
                builder.Build(value.Hair),
                builder.Build(value.Sex),
                builder.Build(value.Nation),
                builder.Build(value.Creature));
        }
        public static Offset<fb.protocol.db.request.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.request.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Name));
        }
        public static Offset<fb.protocol.db.request.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.request.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.Save.CreateSave(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.Article));
        }
        public static Offset<fb.protocol.db.request.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.request.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.Section),
                builder.Build(value.Position));
        }
        public static Offset<fb.protocol.db.request.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Section),
                builder.Build(value.User),
                builder.Build(value.Title),
                builder.Build(value.Contents));
        }
        public static Offset<fb.protocol.db.request.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.request.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.request.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Id),
                builder.Build(value.User));
        }
        public static Offset<fb.protocol.db.response.raw.DeleteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.DeleteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.DeleteArticle.CreateDeleteArticle(builder,
                builder.Build(value.Result));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticle.CreateGetArticle(builder,
                builder.Build(value.Article),
                builder.Build(value.Next),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.GetArticleList> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetArticleList value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetArticleList.CreateGetArticleList(builder,
                builder.Build(value.SummaryList));
        }
        public static Offset<fb.protocol.db.response.raw.WriteArticle> Build(this FlatBufferBuilder builder, fb.protocol.db.response.WriteArticle value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.WriteArticle.CreateWriteArticle(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ChangePw> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ChangePw value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ChangePw.CreateChangePw(builder,
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.GetUid> Build(this FlatBufferBuilder builder, fb.protocol.db.response.GetUid value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.GetUid.CreateGetUid(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.InitCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.InitCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.InitCharacter.CreateInitCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Authenticate> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Authenticate value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Authenticate.CreateAuthenticate(builder,
                builder.Build(value.Map),
                builder.Build(value.ErrorCode));
        }
        public static Offset<fb.protocol.db.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Character),
                builder.Build(value.Items),
                builder.Build(value.Spells));
        }
        public static Offset<fb.protocol.db.response.raw.MakeCharacter> Build(this FlatBufferBuilder builder, fb.protocol.db.response.MakeCharacter value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.MakeCharacter.CreateMakeCharacter(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.ReserveName> Build(this FlatBufferBuilder builder, fb.protocol.db.response.ReserveName value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.ReserveName.CreateReserveName(builder,
                builder.Build(value.Uid),
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol.db.response.raw.Save> Build(this FlatBufferBuilder builder, fb.protocol.db.response.Save value)
        {
            if (value == null)
                return default;

            return fb.protocol.db.response.raw.Save.CreateSave(builder,
                builder.Build(value.Success));
        }
        public static Offset<fb.protocol._internal.request.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.request.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.request.raw.Login.CreateLogin(builder,
                builder.Build(value.Uid),
                builder.Build(value.Name),
                builder.Build(value.Map));
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
                builder.Build(value.Id));
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
        public static Offset<fb.protocol._internal.response.raw.KickOut> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.KickOut value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.KickOut.CreateKickOut(builder,
                builder.Build(value.Uid));
        }
        public static Offset<fb.protocol._internal.response.raw.Login> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Login value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Login.CreateLogin(builder,
                builder.Build(value.Success),
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
                builder.Build(value.Code),
                builder.Build(value.Ip),
                builder.Build(value.Port));
        }
        public static Offset<fb.protocol._internal.response.raw.Whisper> Build(this FlatBufferBuilder builder, fb.protocol._internal.response.Whisper value)
        {
            if (value == null)
                return default;

            return fb.protocol._internal.response.raw.Whisper.CreateWhisper(builder,
                builder.Build(value.Success),
                builder.Build(value.From),
                builder.Build(value.To),
                builder.Build(value.Message));
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Item> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Item>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.Spell> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.Spell>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static VectorOffset Build(this FlatBufferBuilder builder, List<fb.protocol.db.ArticleSummary> value)
        {
            builder.CreateVectorOfTables<fb.protocol.db.raw.ArticleSummary>(value.Select(x => Build(builder, x)).ToArray());
            return default;
        }
        public static fb.protocol._internal.raw.Service Build(this FlatBufferBuilder builder, fb.protocol._internal.Service value)
        {
            return (fb.protocol._internal.raw.Service)value;
        }
        public static fb.protocol._internal.raw.TransferResult Build(this FlatBufferBuilder builder, fb.protocol._internal.TransferResult value)
        {
            return (fb.protocol._internal.raw.TransferResult)value;
        }
    }

    public enum FlatBufferProtocolType
    { 
        KickOut,
        Login,
        Logout,
        Pong,
        Transfer,
        Whisper
    }
}

namespace fb.protocol.db
{

    public class Position : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Position;
        public ushort X { get; set; } = 0;
        public ushort Y { get; set; } = 0;

        public Position()
        { }

        public Position(fb.protocol.db.raw.Position raw)
        {
            X = raw.X;
            Y = raw.Y;
        }

        public Position(byte[] bytes) : this(fb.protocol.db.raw.Position.GetRootAsPosition(new ByteBuffer(bytes)))
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
        public string LastLogin { get; set; } = string.Empty;
        public bool Admin { get; set; } = false;
        public ushort Look { get; set; } = 0;
        public ushort Color { get; set; } = 0;
        public ushort Sex { get; set; } = 0;
        public ushort Nation { get; set; } = 0;
        public ushort? Creature { get; set; } = null;
        public uint Map { get; set; } = 0;
        public fb.protocol.db.Position Position { get; set; }
        public byte Direction { get; set; } = 0;
        public byte State { get; set; } = 0;
        public byte ClassType { get; set; } = 0;
        public byte Promotion { get; set; } = 0;
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
        public uint? Clan { get; set; } = null;

        public Character()
        { }

        public Character(fb.protocol.db.raw.Character raw)
        {
            Id = raw.Id;
            Name = raw.Name;
            LastLogin = raw.LastLogin;
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
            Clan = raw.Clan != null ? (uint?)raw.Clan.Value.Value : null;
        }

        public Character(byte[] bytes) : this(fb.protocol.db.raw.Character.GetRootAsCharacter(new ByteBuffer(bytes)))
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

        public Item(fb.protocol.db.raw.Item raw)
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

        public Item(byte[] bytes) : this(fb.protocol.db.raw.Item.GetRootAsItem(new ByteBuffer(bytes)))
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

        public Spell(fb.protocol.db.raw.Spell raw)
        {
            User = raw.User;
            Slot = raw.Slot;
            Model = raw.Model;
        }

        public Spell(byte[] bytes) : this(fb.protocol.db.raw.Spell.GetRootAsSpell(new ByteBuffer(bytes)))
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

        public ArticleSummary(fb.protocol.db.raw.ArticleSummary raw)
        {
            Id = raw.Id;
            User = raw.User;
            UserName = raw.UserName;
            Title = raw.Title;
            CreatedDate = raw.CreatedDate;
        }

        public ArticleSummary(byte[] bytes) : this(fb.protocol.db.raw.ArticleSummary.GetRootAsArticleSummary(new ByteBuffer(bytes)))
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

        public Article(fb.protocol.db.raw.Article raw)
        {
            Id = raw.Id;
            User = raw.User;
            UserName = raw.UserName;
            Title = raw.Title;
            Contents = raw.Contents;
            CreatedDate = raw.CreatedDate;
        }

        public Article(byte[] bytes) : this(fb.protocol.db.raw.Article.GetRootAsArticle(new ByteBuffer(bytes)))
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

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Position => typeof(fb.protocol.db.Position),
                FlatBufferProtocolType.Character => typeof(fb.protocol.db.Character),
                FlatBufferProtocolType.Item => typeof(fb.protocol.db.Item),
                FlatBufferProtocolType.Spell => typeof(fb.protocol.db.Spell),
                FlatBufferProtocolType.ArticleSummary => typeof(fb.protocol.db.ArticleSummary),
                FlatBufferProtocolType.Article => typeof(fb.protocol.db.Article),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.db.request
{

    public class Account : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Account;
        public uint Uid { get; set; } = 0;

        public Account()
        { }

        public Account(fb.protocol.db.request.raw.Account raw)
        {
            Uid = raw.Uid;
        }

        public Account(byte[] bytes) : this(fb.protocol.db.request.raw.Account.GetRootAsAccount(new ByteBuffer(bytes)))
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

        public ChangePw(fb.protocol.db.request.raw.ChangePw raw)
        {
            Uid = raw.Uid;
            Before = raw.Before;
            After = raw.After;
            Birthday = raw.Birthday;
        }

        public ChangePw(byte[] bytes) : this(fb.protocol.db.request.raw.ChangePw.GetRootAsChangePw(new ByteBuffer(bytes)))
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

        public InitCharacter(fb.protocol.db.request.raw.InitCharacter raw)
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

        public InitCharacter(byte[] bytes) : this(fb.protocol.db.request.raw.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
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

        public Authenticate(fb.protocol.db.request.raw.Authenticate raw)
        {
            Uid = raw.Uid;
            Pw = raw.Pw;
        }

        public Authenticate(byte[] bytes) : this(fb.protocol.db.request.raw.Authenticate.GetRootAsAuthenticate(new ByteBuffer(bytes)))
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
    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public uint Uid { get; set; } = 0;

        public Login()
        { }

        public Login(fb.protocol.db.request.raw.Login raw)
        {
            Uid = raw.Uid;
        }

        public Login(byte[] bytes) : this(fb.protocol.db.request.raw.Login.GetRootAsLogin(new ByteBuffer(bytes)))
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

        public MakeCharacter(fb.protocol.db.request.raw.MakeCharacter raw)
        {
            Uid = raw.Uid;
            Hair = raw.Hair;
            Sex = raw.Sex;
            Nation = raw.Nation;
            Creature = raw.Creature;
        }

        public MakeCharacter(byte[] bytes) : this(fb.protocol.db.request.raw.MakeCharacter.GetRootAsMakeCharacter(new ByteBuffer(bytes)))
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

        public ReserveName(fb.protocol.db.request.raw.ReserveName raw)
        {
            Name = raw.Name;
        }

        public ReserveName(byte[] bytes) : this(fb.protocol.db.request.raw.ReserveName.GetRootAsReserveName(new ByteBuffer(bytes)))
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
        public fb.protocol.db.Character Character { get; set; }
        public List<fb.protocol.db.Item> Items { get; set; } = new List<fb.protocol.db.Item>();
        public List<fb.protocol.db.Spell> Spells { get; set; } = new List<fb.protocol.db.Spell>();

        public Save()
        { }

        public Save(fb.protocol.db.request.raw.Save raw)
        {
            Character = new Character(raw.Character.Value);
            Items = Enumerable.Range(0, raw.ItemsLength).Select(i => raw.Items(i)).Select(x => new Item(x.Value)).ToList();
            Spells = Enumerable.Range(0, raw.SpellsLength).Select(i => raw.Spells(i)).Select(x => new Spell(x.Value)).ToList();
        }

        public Save(byte[] bytes) : this(fb.protocol.db.request.raw.Save.GetRootAsSave(new ByteBuffer(bytes)))
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

        public GetArticle(fb.protocol.db.request.raw.GetArticle raw)
        {
            Section = raw.Section;
            Article = raw.Article;
        }

        public GetArticle(byte[] bytes) : this(fb.protocol.db.request.raw.GetArticle.GetRootAsGetArticle(new ByteBuffer(bytes)))
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

        public GetArticleList(fb.protocol.db.request.raw.GetArticleList raw)
        {
            Section = raw.Section;
            Position = raw.Position;
        }

        public GetArticleList(byte[] bytes) : this(fb.protocol.db.request.raw.GetArticleList.GetRootAsGetArticleList(new ByteBuffer(bytes)))
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

        public WriteArticle(fb.protocol.db.request.raw.WriteArticle raw)
        {
            Section = raw.Section;
            User = raw.User;
            Title = raw.Title;
            Contents = raw.Contents;
        }

        public WriteArticle(byte[] bytes) : this(fb.protocol.db.request.raw.WriteArticle.GetRootAsWriteArticle(new ByteBuffer(bytes)))
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

        public DeleteArticle(fb.protocol.db.request.raw.DeleteArticle raw)
        {
            Id = raw.Id;
            User = raw.User;
        }

        public DeleteArticle(byte[] bytes) : this(fb.protocol.db.request.raw.DeleteArticle.GetRootAsDeleteArticle(new ByteBuffer(bytes)))
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

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.Account => typeof(fb.protocol.db.request.Account),
                FlatBufferProtocolType.ChangePw => typeof(fb.protocol.db.request.ChangePw),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol.db.request.InitCharacter),
                FlatBufferProtocolType.Authenticate => typeof(fb.protocol.db.request.Authenticate),
                FlatBufferProtocolType.Login => typeof(fb.protocol.db.request.Login),
                FlatBufferProtocolType.MakeCharacter => typeof(fb.protocol.db.request.MakeCharacter),
                FlatBufferProtocolType.ReserveName => typeof(fb.protocol.db.request.ReserveName),
                FlatBufferProtocolType.Save => typeof(fb.protocol.db.request.Save),
                FlatBufferProtocolType.GetArticle => typeof(fb.protocol.db.request.GetArticle),
                FlatBufferProtocolType.GetArticleList => typeof(fb.protocol.db.request.GetArticleList),
                FlatBufferProtocolType.WriteArticle => typeof(fb.protocol.db.request.WriteArticle),
                FlatBufferProtocolType.DeleteArticle => typeof(fb.protocol.db.request.DeleteArticle),
                _ => throw new ArgumentException(),
            };
        }
    }
}
namespace fb.protocol.db.response
{

    public class DeleteArticle : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.DeleteArticle;
        public int Result { get; set; } = 0;

        public DeleteArticle()
        { }

        public DeleteArticle(fb.protocol.db.response.raw.DeleteArticle raw)
        {
            Result = raw.Result;
        }

        public DeleteArticle(byte[] bytes) : this(fb.protocol.db.response.raw.DeleteArticle.GetRootAsDeleteArticle(new ByteBuffer(bytes)))
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
        public fb.protocol.db.Article Article { get; set; }
        public bool Next { get; set; } = false;
        public bool Success { get; set; } = false;

        public GetArticle()
        { }

        public GetArticle(fb.protocol.db.response.raw.GetArticle raw)
        {
            Article = new Article(raw.Article.Value);
            Next = raw.Next;
            Success = raw.Success;
        }

        public GetArticle(byte[] bytes) : this(fb.protocol.db.response.raw.GetArticle.GetRootAsGetArticle(new ByteBuffer(bytes)))
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
        public List<fb.protocol.db.ArticleSummary> SummaryList { get; set; } = new List<fb.protocol.db.ArticleSummary>();

        public GetArticleList()
        { }

        public GetArticleList(fb.protocol.db.response.raw.GetArticleList raw)
        {
            SummaryList = Enumerable.Range(0, raw.SummaryListLength).Select(i => raw.SummaryList(i)).Select(x => new ArticleSummary(x.Value)).ToList();
        }

        public GetArticleList(byte[] bytes) : this(fb.protocol.db.response.raw.GetArticleList.GetRootAsGetArticleList(new ByteBuffer(bytes)))
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

        public WriteArticle(fb.protocol.db.response.raw.WriteArticle raw)
        {
            Success = raw.Success;
        }

        public WriteArticle(byte[] bytes) : this(fb.protocol.db.response.raw.WriteArticle.GetRootAsWriteArticle(new ByteBuffer(bytes)))
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

        public ChangePw(fb.protocol.db.response.raw.ChangePw raw)
        {
            ErrorCode = raw.ErrorCode;
        }

        public ChangePw(byte[] bytes) : this(fb.protocol.db.response.raw.ChangePw.GetRootAsChangePw(new ByteBuffer(bytes)))
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

        public GetUid(fb.protocol.db.response.raw.GetUid raw)
        {
            Uid = raw.Uid;
            Success = raw.Success;
        }

        public GetUid(byte[] bytes) : this(fb.protocol.db.response.raw.GetUid.GetRootAsGetUid(new ByteBuffer(bytes)))
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

        public InitCharacter(fb.protocol.db.response.raw.InitCharacter raw)
        {
            Success = raw.Success;
        }

        public InitCharacter(byte[] bytes) : this(fb.protocol.db.response.raw.InitCharacter.GetRootAsInitCharacter(new ByteBuffer(bytes)))
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

        public Authenticate(fb.protocol.db.response.raw.Authenticate raw)
        {
            Map = raw.Map;
            ErrorCode = raw.ErrorCode;
        }

        public Authenticate(byte[] bytes) : this(fb.protocol.db.response.raw.Authenticate.GetRootAsAuthenticate(new ByteBuffer(bytes)))
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
    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public fb.protocol.db.Character Character { get; set; }
        public List<fb.protocol.db.Item> Items { get; set; } = new List<fb.protocol.db.Item>();
        public List<fb.protocol.db.Spell> Spells { get; set; } = new List<fb.protocol.db.Spell>();

        public Login()
        { }

        public Login(fb.protocol.db.response.raw.Login raw)
        {
            Character = new Character(raw.Character.Value);
            Items = Enumerable.Range(0, raw.ItemsLength).Select(i => raw.Items(i)).Select(x => new Item(x.Value)).ToList();
            Spells = Enumerable.Range(0, raw.SpellsLength).Select(i => raw.Spells(i)).Select(x => new Spell(x.Value)).ToList();
        }

        public Login(byte[] bytes) : this(fb.protocol.db.response.raw.Login.GetRootAsLogin(new ByteBuffer(bytes)))
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
    public class MakeCharacter : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.MakeCharacter;
        public bool Success { get; set; } = false;

        public MakeCharacter()
        { }

        public MakeCharacter(fb.protocol.db.response.raw.MakeCharacter raw)
        {
            Success = raw.Success;
        }

        public MakeCharacter(byte[] bytes) : this(fb.protocol.db.response.raw.MakeCharacter.GetRootAsMakeCharacter(new ByteBuffer(bytes)))
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

        public ReserveName(fb.protocol.db.response.raw.ReserveName raw)
        {
            Uid = raw.Uid;
            Success = raw.Success;
        }

        public ReserveName(byte[] bytes) : this(fb.protocol.db.response.raw.ReserveName.GetRootAsReserveName(new ByteBuffer(bytes)))
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

        public Save(fb.protocol.db.response.raw.Save raw)
        {
            Success = raw.Success;
        }

        public Save(byte[] bytes) : this(fb.protocol.db.response.raw.Save.GetRootAsSave(new ByteBuffer(bytes)))
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

    public static class FlatBufferProtocolRouter
    {
        public static Type GetProtocolType(FlatBufferProtocolType protocolType)
        {
            return protocolType switch
            {
                FlatBufferProtocolType.DeleteArticle => typeof(fb.protocol.db.response.DeleteArticle),
                FlatBufferProtocolType.GetArticle => typeof(fb.protocol.db.response.GetArticle),
                FlatBufferProtocolType.GetArticleList => typeof(fb.protocol.db.response.GetArticleList),
                FlatBufferProtocolType.WriteArticle => typeof(fb.protocol.db.response.WriteArticle),
                FlatBufferProtocolType.ChangePw => typeof(fb.protocol.db.response.ChangePw),
                FlatBufferProtocolType.GetUid => typeof(fb.protocol.db.response.GetUid),
                FlatBufferProtocolType.InitCharacter => typeof(fb.protocol.db.response.InitCharacter),
                FlatBufferProtocolType.Authenticate => typeof(fb.protocol.db.response.Authenticate),
                FlatBufferProtocolType.Login => typeof(fb.protocol.db.response.Login),
                FlatBufferProtocolType.MakeCharacter => typeof(fb.protocol.db.response.MakeCharacter),
                FlatBufferProtocolType.ReserveName => typeof(fb.protocol.db.response.ReserveName),
                FlatBufferProtocolType.Save => typeof(fb.protocol.db.response.Save),
                _ => throw new ArgumentException(),
            };
        }
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
    public enum TransferResult
    {
        Success,
        Failed,
        LoggedIn,
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
    }
}
namespace fb.protocol._internal.request
{

    public class Login : IFlatBufferEx
    {
        public int ProtocolType => (int)FlatBufferProtocolType.Login;
        public uint Uid { get; set; } = 0;
        public string Name { get; set; } = string.Empty;
        public ushort Map { get; set; } = 0;

        public Login()
        { }

        public Login(fb.protocol._internal.request.raw.Login raw)
        {
            Uid = raw.Uid;
            Name = raw.Name;
            Map = raw.Map;
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

        public Transfer()
        { }

        public Transfer(fb.protocol._internal.request.raw.Transfer raw)
        {
            Service = (fb.protocol._internal.Service)raw.Service;
            Id = raw.Id;
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

        public KickOut()
        { }

        public KickOut(fb.protocol._internal.response.raw.KickOut raw)
        {
            Uid = raw.Uid;
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
        public bool Success { get; set; } = false;
        public bool Logon { get; set; } = false;
        public string Ip { get; set; } = string.Empty;
        public ushort Port { get; set; } = 0;

        public Login()
        { }

        public Login(fb.protocol._internal.response.raw.Login raw)
        {
            Success = raw.Success;
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
        public fb.protocol._internal.TransferResult Code { get; set; }
        public string Ip { get; set; } = string.Empty;
        public ushort Port { get; set; } = 0;

        public Transfer()
        { }

        public Transfer(fb.protocol._internal.response.raw.Transfer raw)
        {
            Code = (fb.protocol._internal.TransferResult)raw.Code;
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
        public bool Success { get; set; } = false;
        public string From { get; set; } = string.Empty;
        public uint To { get; set; } = 0;
        public string Message { get; set; } = string.Empty;

        public Whisper()
        { }

        public Whisper(fb.protocol._internal.response.raw.Whisper raw)
        {
            Success = raw.Success;
            From = raw.From;
            To = raw.To;
            Message = raw.Message;
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
                _ => throw new ArgumentException(),
            };
        }
    }
}
