using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Newtonsoft.Json.Linq;
using System.Collections;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.Serialization;

namespace Fb.Model
{
    public struct Point<T>
    { 
        public T X { get; set; }
        public T Y { get; set; }
    }

    public struct Size<T>
    { 
        public T Width { get; set; }
        public T Height { get; set; }
    }

    public struct Range<T>
    { 
        public T Min { get; set; }
        public T Max { get; set; }
    }
}

namespace Fb.Model.EnumValue
{
    [JsonConverter(typeof(StringEnumConverter))]
    public enum Action
    {
        [EnumMember(Value = "ATTACK")]
        Attack = 0x01, 
        [EnumMember(Value = "ARROW")]
        Arrow = 0x03, 
        [EnumMember(Value = "PICKUP")]
        Pickup = 0x04, 
        [EnumMember(Value = "PICKUP_SILENT")]
        PickupSilent = 0x05, 
        [EnumMember(Value = "CAST_SPELL")]
        CastSpell = 0x06, 
        [EnumMember(Value = "EAT")]
        Eat = 0x08, 
        [EnumMember(Value = "EMOTION")]
        Emotion = 0x0b
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum BoardAction
    {
        [EnumMember(Value = "NONE")]
        None = 0x00, 
        [EnumMember(Value = "SECTIONS")]
        Sections = 0x01, 
        [EnumMember(Value = "ARTICLES")]
        Articles = 0x02, 
        [EnumMember(Value = "ARTICLE")]
        Article = 0x03, 
        [EnumMember(Value = "WRITE")]
        Write = 0x04, 
        [EnumMember(Value = "DELETE")]
        Delete = 0x05
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum BoardButtonEnable
    {
        [EnumMember(Value = "NONE")]
        None = 0x00, 
        [EnumMember(Value = "NEXT")]
        Next = 0x01, 
        [EnumMember(Value = "UP")]
        Up = 0x01, 
        [EnumMember(Value = "WRITE")]
        Write = 0x02
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum BundleType
    {
        [EnumMember(Value = "NONE")]
        None = 0, 
        [EnumMember(Value = "BUNDLE")]
        Bundle = 1, 
        [EnumMember(Value = "PACKAGE")]
        Package = 2
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum ChatType
    {
        [EnumMember(Value = "NORMAL")]
        Normal = 0x00, 
        [EnumMember(Value = "SHOUT")]
        Shout = 0x01, 
        [EnumMember(Value = "BLUE")]
        Blue = 0x02, 
        [EnumMember(Value = "LIGHT_BLUE")]
        LightBlue = 0x03
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Class
    {
        [EnumMember(Value = "NONE")]
        None = 0, 
        [EnumMember(Value = "WARRIOR")]
        Warrior = 1, 
        [EnumMember(Value = "THIEF")]
        Thief = 2, 
        [EnumMember(Value = "MAGICION")]
        Magicion = 3, 
        [EnumMember(Value = "ASCETIC")]
        Ascetic = 4
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Condition
    {
        [EnumMember(Value = "NONE")]
        None = 0x00, 
        [EnumMember(Value = "MOVE")]
        Move = 0x01, 
        [EnumMember(Value = "SIGHT")]
        Sight = 0x02, 
        [EnumMember(Value = "HEAR")]
        Hear = 0x04, 
        [EnumMember(Value = "ORAL")]
        Oral = 0x08, 
        [EnumMember(Value = "MAP")]
        Map = 0x10
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Creature
    {
        [EnumMember(Value = "PHOENIX")]
        Phoenix = 0x00, 
        [EnumMember(Value = "TIGER")]
        Tiger = 0x01, 
        [EnumMember(Value = "TURTLE")]
        Turtle = 0x02, 
        [EnumMember(Value = "DRAGON")]
        Dragon = 0x03
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum CustomSetting
    {
        [EnumMember(Value = "RIDE")]
        Ride = 0x00, 
        [EnumMember(Value = "WHISPER")]
        Whisper = 0x01, 
        [EnumMember(Value = "GROUP")]
        Group = 0x02, 
        [EnumMember(Value = "ROAR")]
        Roar = 0x03, 
        [EnumMember(Value = "ROAR_WORLDS")]
        RoarWorlds = 0x04, 
        [EnumMember(Value = "MAGIC_EFFECT")]
        MagicEffect = 0x05, 
        [EnumMember(Value = "WEATHER_EFFECT")]
        WeatherEffect = 0x06, 
        [EnumMember(Value = "FIXED_MOVE")]
        FixedMove = 0x07, 
        [EnumMember(Value = "TRADE")]
        Trade = 0x08, 
        [EnumMember(Value = "FAST_MOVE")]
        FastMove = 0x09, 
        [EnumMember(Value = "EFFECT_SOUND")]
        EffectSound = 0x0a, 
        [EnumMember(Value = "PK")]
        Pk = 0x0b
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum DeathPenalty
    {
        [EnumMember(Value = "NONE")]
        None = 0, 
        [EnumMember(Value = "DROP")]
        Drop = 1, 
        [EnumMember(Value = "DESTROY")]
        Destroy = 2
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum DestroyType
    {
        [EnumMember(Value = "DEFAULT")]
        Default = 0, 
        [EnumMember(Value = "DEAD")]
        Dead = 1
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Direction
    {
        [EnumMember(Value = "TOP")]
        Top = 0, 
        [EnumMember(Value = "RIGHT")]
        Right = 1, 
        [EnumMember(Value = "BOTTOM")]
        Bottom = 2, 
        [EnumMember(Value = "LEFT")]
        Left = 3
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Dsl
    {
        [EnumMember(Value = "item")]
        Item = 0, 
        [EnumMember(Value = "level")]
        Level = 1, 
        [EnumMember(Value = "sex")]
        Sex = 2, 
        [EnumMember(Value = "strength")]
        Strength = 3, 
        [EnumMember(Value = "intelligence")]
        Intelligence = 4, 
        [EnumMember(Value = "dexteritry")]
        Dexteritry = 5, 
        [EnumMember(Value = "promotion")]
        Promotion = 6, 
        [EnumMember(Value = "class_t")]
        ClassT = 7, 
        [EnumMember(Value = "admin")]
        Admin = 8, 
        [EnumMember(Value = "world")]
        World = 9, 
        [EnumMember(Value = "map")]
        Map = 10
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Duration
    {
        [EnumMember(Value = "FAST")]
        Fast = 0x0f, 
        [EnumMember(Value = "ATTACK")]
        Attack = 0x14, 
        [EnumMember(Value = "SPELL")]
        Spell = 0x20, 
        [EnumMember(Value = "EAT")]
        Eat = 0x27, 
        [EnumMember(Value = "THROW")]
        Throw = 0x28, 
        [EnumMember(Value = "PICKUP")]
        Pickup = 0x32, 
        [EnumMember(Value = "EMOTION")]
        Emotion = 0x4e
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum EquipmentParts
    {
        [EnumMember(Value = "UNKNOWN")]
        Unknown = 0, 
        [EnumMember(Value = "WEAPON")]
        Weapon = 1, 
        [EnumMember(Value = "ARMOR")]
        Armor = 2, 
        [EnumMember(Value = "SHIELD")]
        Shield = 3, 
        [EnumMember(Value = "HELMET")]
        Helmet = 4, 
        [EnumMember(Value = "LEFT_HAND")]
        LeftHand = 7, 
        [EnumMember(Value = "RIGHT_HAND")]
        RightHand = 8, 
        [EnumMember(Value = "LEFT_AUX")]
        LeftAux = 20, 
        [EnumMember(Value = "RIGHT_AUX")]
        RightAux = 21
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum EquipmentPosition
    {
        [EnumMember(Value = "LEFT")]
        Left = 0, 
        [EnumMember(Value = "RIGHT")]
        Right = 1
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum ItemAttribute
    {
        [EnumMember(Value = "NONE")]
        None = 0x00000000, 
        [EnumMember(Value = "CONSUME")]
        Consume = 0x00000001, 
        [EnumMember(Value = "BUNDLE")]
        Bundle = 0x00000002, 
        [EnumMember(Value = "SCRIPT")]
        Script = 0x00000003, 
        [EnumMember(Value = "CASH")]
        Cash = 0x00000004, 
        [EnumMember(Value = "EQUIPMENT")]
        Equipment = 0x10000000, 
        [EnumMember(Value = "PACK")]
        Pack = Consume | 0x00000100, 
        [EnumMember(Value = "WEAPON")]
        Weapon = Equipment | 0x00000100, 
        [EnumMember(Value = "ARMOR")]
        Armor = Equipment | 0x00000200, 
        [EnumMember(Value = "SHIELD")]
        Shield = Equipment | 0x00000400, 
        [EnumMember(Value = "HELMET")]
        Helmet = Equipment | 0x00000800, 
        [EnumMember(Value = "RING")]
        Ring = Equipment | 0x00001000, 
        [EnumMember(Value = "AUXILIARY")]
        Auxiliary = Equipment | 0x00002000, 
        [EnumMember(Value = "ARROW")]
        Arrow = Equipment | 0x00004000
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum ItemDeleteType
    {
        [EnumMember(Value = "REMOVED")]
        Removed = 0x00, 
        [EnumMember(Value = "DROP")]
        Drop = 0x01, 
        [EnumMember(Value = "EAT")]
        Eat = 0x02, 
        [EnumMember(Value = "SMOKE")]
        Smoke = 0x03, 
        [EnumMember(Value = "THROW")]
        Throw = 0x04, 
        [EnumMember(Value = "SHOOT")]
        Shoot = 0x05, 
        [EnumMember(Value = "REDUCE")]
        Reduce = 0x06, 
        [EnumMember(Value = "STICK")]
        Stick = 0x07, 
        [EnumMember(Value = "DECAY")]
        Decay = 0x08, 
        [EnumMember(Value = "GIVE")]
        Give = 0x09, 
        [EnumMember(Value = "SELL")]
        Sell = 0x0a, 
        [EnumMember(Value = "NONE")]
        None = 0x0c, 
        [EnumMember(Value = "DESTROY")]
        Destroy = 0x0d
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum ItemType
    {
        [EnumMember(Value = "STUFF")]
        Stuff = 1, 
        [EnumMember(Value = "CASH")]
        Cash = 2, 
        [EnumMember(Value = "CONSUME")]
        Consume = 3, 
        [EnumMember(Value = "WEAPON")]
        Weapon = 4, 
        [EnumMember(Value = "ARMOR")]
        Armor = 5, 
        [EnumMember(Value = "HELMET")]
        Helmet = 6, 
        [EnumMember(Value = "RING")]
        Ring = 7, 
        [EnumMember(Value = "SHIELD")]
        Shield = 8, 
        [EnumMember(Value = "AUXILIARY")]
        Auxiliary = 9, 
        [EnumMember(Value = "BOW")]
        Bow = 10, 
        [EnumMember(Value = "PACKAGE")]
        Package = 11
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum MapEffectType
    {
        [EnumMember(Value = "NONE")]
        None = 0x00, 
        [EnumMember(Value = "FIRE")]
        Fire = 0x01, 
        [EnumMember(Value = "WATER")]
        Water = 0x02
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum MapOption
    {
        [EnumMember(Value = "NONE")]
        None = 0x00, 
        [EnumMember(Value = "BUILD_IN")]
        BuildIn = 0x01, 
        [EnumMember(Value = "DISABLE_TALK")]
        DisableTalk = 0x02, 
        [EnumMember(Value = "DISABLE_WHISPER")]
        DisableWhisper = 0x04, 
        [EnumMember(Value = "DISABLE_SPELL")]
        DisableSpell = 0x08, 
        [EnumMember(Value = "HUNTING_GROUND")]
        HuntingGround = 0x10, 
        [EnumMember(Value = "ENABLE_PK")]
        EnablePk = 0x20, 
        [EnumMember(Value = "DISABLE_DIE_PENALTY")]
        DisableDiePenalty = 0x30
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum MessageType
    {
        [EnumMember(Value = "NOTIFY")]
        Notify = 0x00, 
        [EnumMember(Value = "BLUE")]
        Blue = 0x01, 
        [EnumMember(Value = "STATE")]
        State = 0x03, 
        [EnumMember(Value = "SHOUT")]
        Shout = 0x04, 
        [EnumMember(Value = "WORLD")]
        World = 0x05, 
        [EnumMember(Value = "POPUP")]
        Popup = 0x08, 
        [EnumMember(Value = "YELLOW")]
        Yellow = 0x0b, 
        [EnumMember(Value = "BROWN")]
        Brown = 0x0c
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum MobAttackType
    {
        [EnumMember(Value = "NONE")]
        None = 0, 
        [EnumMember(Value = "COUNTER")]
        Counter = 1, 
        [EnumMember(Value = "CONTAINMENT")]
        Containment = 2, 
        [EnumMember(Value = "RUN_AWAY")]
        RunAway = 3, 
        [EnumMember(Value = "NO_MOVE")]
        NoMove = 4
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum MobSize
    {
        [EnumMember(Value = "SMALL")]
        Small = 1, 
        [EnumMember(Value = "LARGE")]
        Large = 2, 
        [EnumMember(Value = "ALL")]
        All = Small | Large
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Nation
    {
        [EnumMember(Value = "GOGURYEO")]
        Goguryeo = 0x01, 
        [EnumMember(Value = "BUYEO")]
        Buyeo = 0x02
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum ObjectType
    {
        [EnumMember(Value = "UNKNOWN")]
        Unknown = 0x00, 
        [EnumMember(Value = "ITEM")]
        Item = 0x01, 
        [EnumMember(Value = "NPC")]
        Npc = 0x02, 
        [EnumMember(Value = "MOB")]
        Mob = 0x04, 
        [EnumMember(Value = "SESSION")]
        Session = 0x08, 
        [EnumMember(Value = "OBJECT")]
        Object = Item | Npc | Mob, 
        [EnumMember(Value = "LIFE")]
        Life = Mob | Session
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Regex
    {
        [EnumMember(Value = "SELL")]
        Sell = 0, 
        [EnumMember(Value = "BUY")]
        Buy = 1, 
        [EnumMember(Value = "REPAIR")]
        Repair = 2, 
        [EnumMember(Value = "DEPOSIT_MONEY")]
        DepositMoney = 3, 
        [EnumMember(Value = "WITHDRAW_MONEY")]
        WithdrawMoney = 4, 
        [EnumMember(Value = "DEPOSIT_ITEM")]
        DepositItem = 5, 
        [EnumMember(Value = "WITHDRAW_ITEM")]
        WithdrawItem = 6, 
        [EnumMember(Value = "SELL_LIST")]
        SellList = 7, 
        [EnumMember(Value = "BUY_LIST")]
        BuyList = 8, 
        [EnumMember(Value = "SELL_PRICE")]
        SellPrice = 9, 
        [EnumMember(Value = "BUY_PRICE")]
        BuyPrice = 10, 
        [EnumMember(Value = "DEPOSITED_MONEY")]
        DepositedMoney = 11, 
        [EnumMember(Value = "RENAME_WEAPON")]
        RenameWeapon = 12, 
        [EnumMember(Value = "HOLD_ITEM_LIST")]
        HoldItemList = 13, 
        [EnumMember(Value = "HOLD_ITEM_COUNT")]
        HoldItemCount = 14
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Sex
    {
        [EnumMember(Value = "MAN")]
        Man = 1, 
        [EnumMember(Value = "WOMAN")]
        Woman = 2, 
        [EnumMember(Value = "ALL")]
        All = Man | Woman
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum Sound
    {
        [EnumMember(Value = "EAT")]
        Eat = 0x0006, 
        [EnumMember(Value = "DISGUISE")]
        Disguise = 0x0019, 
        [EnumMember(Value = "SWING")]
        Swing = 0x014b, 
        [EnumMember(Value = "DAMAGE")]
        Damage = 0x015d, 
        [EnumMember(Value = "EQUIPMENT_OFF")]
        EquipmentOff = 0x019a, 
        [EnumMember(Value = "EQUIPMENT_ON")]
        EquipmentOn = 0x019b
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum SpellType
    {
        [EnumMember(Value = "INPUT")]
        Input = 0x01, 
        [EnumMember(Value = "TARGET")]
        Target = 0x02, 
        [EnumMember(Value = "NORMAL")]
        Normal = 0x05, 
        [EnumMember(Value = "UNKNOWN1")]
        Unknown1 = 8, 
        [EnumMember(Value = "UNKNOWN2")]
        Unknown2 = 9, 
        [EnumMember(Value = "UNKNOWN3")]
        Unknown3 = 12
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum State
    {
        [EnumMember(Value = "NORMAL")]
        Normal = 0x00, 
        [EnumMember(Value = "GHOST")]
        Ghost = 0x01, 
        [EnumMember(Value = "TRANSLUCENCY")]
        Translucency = 0x02, 
        [EnumMember(Value = "RIDING")]
        Riding = 0x03, 
        [EnumMember(Value = "DISGUISE")]
        Disguise = 0x04, 
        [EnumMember(Value = "HALF_CLOACK")]
        HalfCloack = 0x05, 
        [EnumMember(Value = "CLOACK")]
        Cloack = 0x06
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum StateLevel
    {
        [EnumMember(Value = "CONDITION")]
        Condition = 0x08, 
        [EnumMember(Value = "EXP_MONEY")]
        ExpMoney = 0x10, 
        [EnumMember(Value = "HP_MP")]
        HpMp = 0x20, 
        [EnumMember(Value = "BASED")]
        Based = 0x40, 
        [EnumMember(Value = "LEVEL_MAX")]
        LevelMax = Based | HpMp | ExpMoney | Condition, 
        [EnumMember(Value = "LEVEL_MIN")]
        LevelMin = ExpMoney | Condition, 
        [EnumMember(Value = "LEVEL_MIDDLE")]
        LevelMiddle = HpMp | ExpMoney | Condition
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum SwapType
    {
        [EnumMember(Value = "ITEM")]
        Item = 0x00, 
        [EnumMember(Value = "SPELL")]
        Spell = 0x01
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum TimerType
    {
        [EnumMember(Value = "INCREASE")]
        Increase = 0x01, 
        [EnumMember(Value = "DECREASE")]
        Decrease = 0x02
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum WeaponType
    {
        [EnumMember(Value = "NORMAL")]
        Normal = 0, 
        [EnumMember(Value = "SPEAR")]
        Spear = 1, 
        [EnumMember(Value = "BOW")]
        Bow = 2, 
        [EnumMember(Value = "FAN")]
        Fan = 3, 
        [EnumMember(Value = "UNKNOWN")]
        Unknown = 4
    }

    [JsonConverter(typeof(StringEnumConverter))]
    public enum WeatherType
    {
        [EnumMember(Value = "NORMAL")]
        Normal = 0x00, 
        [EnumMember(Value = "RAIN")]
        Rain = 0x01, 
        [EnumMember(Value = "SNOW")]
        Snow = 0x02, 
        [EnumMember(Value = "BIRD")]
        Bird = 0x03
    }

}

namespace Fb.Model.ConstValue
{
    public static class Item
    {
        public const uint Bronze = 60000;
        public const uint Silver = 60001;
        public const uint Gold = 60002;
        public const uint BronzeBundle = 60003;
        public const uint SilverBundle = 60004;
        public const uint GoldBundle = 60005;
    }

    public static class Mob
    {
        public const uint Horse = 301;
    }

    public static class Regex
    {
        public const string Sell = "(?P<name>\\S+)\\s+(?:(?:(?:(?P<count>\\d+)개)|(?P<all>다|전부))\\s+)?(?:판다|팜|팔게)";
        public const string Buy = "(?P<name>\\S+)\\s+(?:(?:(?:(?P<count>\\d+)개))\\s+)?(?:산다|줘|주세요)";
        public const string Repair = "(((?P<all>전부|모두|다)|(?P<name>\\S+))\\s+?(?:고쳐|수리\\s*해))\\s*줘";
        public const string DepositMoney = "(?:돈|금전)\\s+(?:(?P<money>\\d+)(?:원|전)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+맡아\\s*(?:줘|놔|주세요)";
        public const string WithdrawMoney = "(?:돈|금전)\\s+(?:(?P<money>\\d+)(?:원|전)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+돌려\\s*(?:줘|놔|주세요)";
        public const string DepositItem = "(?P<name>\\S+)\\s+(?:(?:(?P<count>\\d+)(?:개)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+)?맡아\\s*(?:줘|놔|주세요)";
        public const string WithdrawItem = "(?P<name>\\S+)\\s+(?:(?:(?P<count>\\d+)(?:개)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+)?돌려\\s*(?:줘|놔|주세요)";
        public const string SellList = "(?:뭐|뭘|무엇을|무얼)\\s*(?:파니|파냐|팔고\\s*(?:있니|있냐))";
        public const string BuyList = "(?:뭐|뭘|무엇을|무얼)\\s*(?:사니|사냐|사고\\s*(?:있니|있냐))";
        public const string SellPrice = "(?P<name>\\S+)\\s+(?:(?:얼마(?:니|야|임|냐|에\\s*파(?:니|냐)))|(?:파(?:니|냐|)))";
        public const string BuyPrice = "(?P<name>\\S+)\\s+(?:얼마에\\s?)?사(?:니|냐)";
        public const string DepositedMoney = "(?:돈|금전)\\s*얼마(?:나)?\\s*맡(?:아두)?고\\s*있(?:니|냐)";
        public const string RenameWeapon = "(?P<weapon>\\S+?)?(?:의|$)?\\s+이름을\\s+(?P<name>\\S+?)?(?:으|$)?로\\s+명명";
        public const string HoldItemList = "(?:뭐|뭘|무엇을|무얼)\\s*맡고\\s*(?:있니|있냐)";
        public const string HoldItemCount = "(?P<name>\\S+)\\s+(?:몇\\s*개|얼마나)\\s*맡고\\s*있(?:니|냐)";
    }

    public static class String
    {
        public const string AccountInvalidName = "이름이 길거나 적합하지 않습니다.";
        public const string AccountAlreadyLogin = "이미 접속중입니다.";
        public const string AccountNotFoundName = "존재하지 않는 이름입니다.";
        public const string AccountPasswordSize = "암호는 4자 이상 8자 이하";
        public const string AccountInvalidPassword = "비밀번호가 올바르지 않습니다.";
        public const string AccountSimplePassword = "암호가 단순합니다.";
        public const string AccountSuccessRegisterAccount = "등록완료, 이어하기를 선택하세요.";
        public const string AccountSuccessChangePassword = "변경됐다리";
        public const string AccountInvalidBirthday = "생년월일이 올바르지 않습니다.";
        public const string AccountNewPwEqualization = "기존 암호화 동일합니다.";
        public const string AccountAlreadyExists = "이미 존재하는 이름입니다.";
        public const string MessageAssetMapLoaded = "* [{:0.2f}%] 맵 정보를 읽었습니다. ({})";
        public const string MessageAssetRegexLoaded = "* [{:0.2f}%] 정규표현식 정보를 읽었습니다. ({})";
        public const string MessageAssetWorldMapLoaded = "* [{:0.2f}%] 월드맵 정보를 읽었습니다. ({})";
        public const string MessageAssetDoorLoaded = "* [{:0.2f}%] 도어 정보를 읽었습니다. ({})";
        public const string MessageAssetSpellLoaded = "* [{:0.2f}%] 스펠 정보를 읽었습니다. ({})";
        public const string MessageAssetWarpLoaded = "* [{:0.2f}%] 워프 정보를 읽었습니다. ({})";
        public const string MessageAssetItemLoaded = "* [{:0.2f}%] 아이템 정보를 읽었습니다. ({})";
        public const string MessageAssetItemMixLoaded = "* [{:0.2f}%] 조합 정보를 읽었습니다. ({})";
        public const string MessageAssetSellLoaded = "* [{:0.2f}%] 판매 정보를 읽었습니다. ({})";
        public const string MessageAssetBuyLoaded = "* [{:0.2f}%] 구매 정보를 읽었습니다. ({})";
        public const string MessageAssetNpcLoaded = "* [{:0.2f}%] NPC 정보를 읽었습니다. ({})";
        public const string MessageAssetMobLoaded = "* [{:0.2f}%] 몹 정보를 읽었습니다. ({})";
        public const string MessageAssetDropLoaded = "* [{:0.2f}%] 드롭 정보를 읽었습니다. ({})";
        public const string MessageAssetNpcSpawnLoaded = "* [{:0.2f}%] NPC 스폰 정보를 읽었습니다. ({})";
        public const string MessageAssetMobSpawnLoaded = "* [{:0.2f}%] 몹 스폰 정보를 읽었습니다. ({})";
        public const string MessageAssetClassLoaded = "* [{:0.2f}%] 클래스 정보를 읽었습니다. ({})";
        public const string MessageAssetBoardLoaded = "* [{:0.2f}%] 게시판 정보를 읽었습니다. ({})";
        public const string MessageAssetMapAllLoaded = "* [100%] 총 %d개의 맵 정보를 읽었습니다.";
        public const string MessageAssetRegexAllLoaded = "* [100%] 총 %d개의 정규표현식 정보를 읽었습니다.";
        public const string MessageAssetWorldMapAllLoaded = "* [100%] 총 %d개의 월드맵 정보를 읽었습니다.";
        public const string MessageAssetDoorAllLoaded = "* [100%] 총 %d개의 도어 정보를 읽었습니다.";
        public const string MessageAssetSpellAllLoaded = "* [100%] 총 %d개의 스펠 정보를 읽었습니다.";
        public const string MessageAssetWarpAllLoaded = "* [100%] 총 %d개의 워프 정보를 읽었습니다.";
        public const string MessageAssetItemAllLoaded = "* [100%] 총 %d개의 아이템 정보를 읽었습니다.";
        public const string MessageAssetItemMixAllLoaded = "* [100%] 총 %d개의 조합 정보를 읽었습니다.";
        public const string MessageAssetSellAllLoaded = "* [100%] 총 %d개의 판매 정보를 읽었습니다.";
        public const string MessageAssetBuyAllLoaded = "* [100%] 총 %d개의 구매 정보를 읽었습니다.";
        public const string MessageAssetNpcAllLoaded = "* [100%] 총 %d개의 NPC 정보를 읽었습니다.";
        public const string MessageAssetMobAllLoaded = "* [100%] 총 %d개의 몹 정보를 읽었습니다.";
        public const string MessageAssetDropAllLoaded = "* [100%] 총 %d개의 드롭 정보를 읽었습니다.";
        public const string MessageAssetNpcSpawnAllLoaded = "* [100%] 총 %d개의 NPC 스폰 정보를 읽었습니다.";
        public const string MessageAssetMobSpawnAllLoaded = "* [100%] 총 %d개의 몹 스폰 정보를 읽었습니다.";
        public const string MessageAssetClassAllLoaded = "* [100%] 총 %d개의 클래스 정보를 읽었습니다.";
        public const string MessageAssetBoardAllLoaded = "* [100%] 총 %d개의 게시판 정보를 읽었습니다.";
        public const string MessageAssetInvalidSex = "성별을 확인할 수 없습니다.";
        public const string MessageAssetInvalidDeathPenalty = "아이템 데스 패널티를 확인할 수 없습니다.";
        public const string MessageAssetInvalidMobSize = "몹 크기 형식을 확인할 수 없습니다.";
        public const string MessageAssetInvalidMobOffensive = "몹 공격 타입을 확인할 수 없습니다.";
        public const string MessageAssetCannotLoadMapData = "맵 데이터를 읽을 수 없습니다.";
        public const string MessageAssetCannotLoadMapBlock = "맵 블록 데이터를 읽을 수 없습니다.";
        public const string MessageAssetInvalidNpcName = "존재하지 않는 NPC입니다.";
        public const string MessageAssetInvalidMapName = "존재하지 않는 맵입니다.";
        public const string MessageAssetInvalidNpcDirection = "NPC의 방향이 올바르지 않습니다.";
        public const string MessageAssetInvalidNpcPosition = "NPC의 위치가 올바르지 않습니다.";
        public const string MessageAssetInvalidMobName = "올바르지 않은 몹 이름입니다.";
        public const string MessageAssetInvalidItemName = "올바르지 않은 아이템 이름입니다.";
        public const string MessageItemCannotPickupAnymore = "더 이상 가질 수 없습니다.";
        public const string MessageGroupCannotFindTarget = "대상을 찾을 수 없습니다.";
        public const string MessageGroupDisabledMine = "그룹 거부 상태입니다.";
        public const string MessageGroupDisabledTarget = "상대방이 그룹 거부 상태입니다.";
        public const string MessageGroupNotOwner = "그룹장만 할 수 있습니다.";
        public const string MessageGroupFullMember = "자리가 없습니다.";
        public const string MessageGroupAlreadyJoined = "님은 이미 그룹 참여 중입니다.";
        public const string MessageGroupJoined = "님 그룹에 참여";
        public const string MessageGroupLeft = "님 그룹 탈퇴";
        public const string MessageExceptionRidding = "말을 타고는 할 수 없습니다.";
        public const string MessageExceptionGhost = "귀신은 할 수 없습니다.";
        public const string MessageExceptionRequireClass = "직업을 선택해야 합니다.";
        public const string MessageExceptionNoConveyance = "탈 것이 없습니다.";
        public const string MessageExceptionDisguise = "변신 중에는 할 수 없습니다.";
        public const string MessageExceptionCannotDropItem = "버릴 수 없는 물건입니다.";
        public const string MessageExceptionCannotThrowItem = "던질 수 없는 물건입니다.";
        public const string MessageExceptionInventoryOverflow = "소지품이 꽉 찼습니다.";
        public const string MessageExceptionInvalidTarget = "대상이 올바르지 않습니다.";
        public const string MessageTradeNotTrading = "교환중이 아닙니다.";
        public const string MessageTradeNotSelected = "선택된 아이템이 없습니다.";
        public const string MessageTradeSuccess = "교환에 성공했습니다.";
        public const string MessageTradeFailed = "교환에 실패했습니다.";
        public const string MessageTradeNotifyLockToPartner = "상대방이 확인을 눌렀습니다.";
        public const string MessageTradeCancelledByMe = "내가 교환을 취소했습니다.";
        public const string MessageTradeCancelledByPartner = "상대방이 교환을 취소했습니다.";
        public const string MessageTradeInvalidCount = "갯수가 올바르지 않습니다.";
        public const string MessageTradeNotAllowedToTrade = "교환이 불가능한 아이템입니다.";
        public const string MessageTradeRefusedByMe = "교환 거부 상태입니다.";
        public const string MessageTradeRefusedByPartner = "님은 교환 거부 상태입니다.";
        public const string MessageTradePartnerAlreadyTrading = "님은 이미 교환 중입니다.";
        public const string MessageTradePartnerInvisible = "대상이 보이지 않습니다.";
        public const string MessageTradePartnerTooFar = "님과 너무 멀리 떨어져 있습니다.";
        public const string MessageMixSuccess = "성공하였습니다.";
        public const string MessageMixFailed = "실패하였습니다.";
        public const string MessageRideUnride = "말에 타고 있지 않습니다.";
        public const string MessageRideAlreadyRide = "이미 타고 있습니다.";
        public const string MessageRideOn = "말에 탔습니다.";
        public const string MessageRideOff = "말에서 내렸습니다.";
        public const string MessageMoneyDrop = "돈을 버렸습니다.";
        public const string MessageMoneyFull = "더 이상 돈을 가질 수 없습니다.";
        public const string MessageLevelUp = "레벨이 올랐습니다";
        public const string MessageBoardWrite = "글을 작성하였습니다";
        public const string MessageBoardArticleNotExist = "게시글이 존재하지 않습니다.";
        public const string MessageBoardSectionNotExist = "섹션이 존재하지 않습니다.";
        public const string MessageBoardNotAuth = "권한이 없습니다.";
        public const string MessageBoardSuccessDelete = "글이 삭제되었습니다.";
        public const string MessageBoardTooLongTitle = "게시글 제목이 너무 깁니다.";
        public const string MessageBoardTooLongContents = "게시글 내용이 너무 깁니다.";
        public const string MessageDoorOpen = "문을 열었습니다.";
        public const string MessageDoorClose = "문을 닫았습니다.";
        public const string MessageDoorLock = "문을 잠궜습니다.";
        public const string MessageDoorUnlock = "문을 열었습니다.";
        public const string MessageDoorLocked = "문이 잠겼습니다.";
        public const string MessageErrorUnknown = "올바르지 않은 명령입니다.";
    }

}

namespace Fb.Model
{
    public class Ability
    {
        [JsonProperty("parent")]
        public Fb.Model.EnumValue.Class Parent { get; set; }
        [JsonProperty("level")]
        public byte Level { get; set; }
        [JsonProperty("dexteritry")]
        public byte Dexteritry { get; set; }
        [JsonProperty("intelligence")]
        public byte Intelligence { get; set; }
        [JsonProperty("strength")]
        public byte Strength { get; set; }
        [JsonProperty("exp")]
        public uint Exp { get; set; }
        [JsonProperty("hp")]
        public uint Hp { get; set; }
        [JsonProperty("mp")]
        public uint Mp { get; set; }
    }

    public class AbilityAttribute
    {
        [JsonProperty("id")]
        public Fb.Model.EnumValue.Class Id { get; set; }
    }

    public class Board
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("name")]
        public string Name { get; set; }
        [JsonProperty("condition")]
        public List<Dsl> Condition { get; set; }
    }

    public class Buy
    {
        [JsonProperty("parent")]
        public uint Parent { get; set; }
        [JsonProperty("item")]
        public uint Item { get; set; }
        [JsonProperty("price")]
        public uint? Price { get; set; }
    }

    public class BuyAttribute
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
    }

    public class Door
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("pairs")]
        public List<uint> Pairs { get; set; }
    }

    public class DoorPair
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("open")]
        public uint Open { get; set; }
        [JsonProperty("close")]
        public uint Close { get; set; }
    }

    public class Drop
    {
        [JsonProperty("id")]
        public string Id { get; set; }
        [JsonProperty("dsl")]
        public List<Dsl> Dsl { get; set; }
    }

    public class Map
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("name")]
        public string Name { get; set; }
        [JsonProperty("root")]
        public uint Root { get; set; }
        [JsonProperty("bgm")]
        public ushort Bgm { get; set; }
        [JsonProperty("effect")]
        public Fb.Model.EnumValue.MapEffectType Effect { get; set; }
        [JsonProperty("host")]
        public byte Host { get; set; }
        [JsonProperty("option")]
        public Fb.Model.EnumValue.MapOption Option { get; set; }
    }

    public class MobSpawn
    {
        [JsonProperty("parent")]
        public uint Parent { get; set; }
        [JsonProperty("begin")]
        public Point<ushort> Begin { get; set; }
        [JsonProperty("end")]
        public Point<ushort> End { get; set; }
        [JsonProperty("count")]
        public ushort Count { get; set; }
        [JsonProperty("mob")]
        public uint Mob { get; set; }
        [JsonProperty("rezen")]
        public TimeSpan Rezen { get; set; }
    }

    public class MobSpawnAttribute
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
    }

    public class NpcSpawn
    {
        [JsonProperty("parent")]
        public uint Parent { get; set; }
        [JsonProperty("npc")]
        public uint Npc { get; set; }
        [JsonProperty("position")]
        public Point<ushort> Position { get; set; }
        [JsonProperty("direction")]
        public Fb.Model.EnumValue.Direction Direction { get; set; }
    }

    public class NpcSpawnAttribute
    {
        [JsonProperty("map")]
        public uint Map { get; set; }
    }

    public class Object
    {
        [JsonProperty("name")]
        public string Name { get; set; }
        [JsonProperty("look")]
        public ushort Look { get; set; }
        [JsonProperty("color")]
        public byte Color { get; set; }
    }

    public class Promotion
    {
        [JsonProperty("parent")]
        public Fb.Model.EnumValue.Class Parent { get; set; }
        [JsonProperty("step")]
        public byte Step { get; set; }
        [JsonProperty("name")]
        public string Name { get; set; }
    }

    public class PromotionAttribute
    {
        [JsonProperty("id")]
        public Fb.Model.EnumValue.Class Id { get; set; }
    }

    public class Recipe
    {
        [JsonProperty("source")]
        public List<Dsl> Source { get; set; }
        [JsonProperty("success")]
        public List<Dsl> Success { get; set; }
        [JsonProperty("failed")]
        public List<Dsl> Failed { get; set; }
        [JsonProperty("percent")]
        public double Percent { get; set; }
    }

    public class Reward
    {
        [JsonProperty("id")]
        public string Id { get; set; }
        [JsonProperty("dsl")]
        public List<Dsl> Dsl { get; set; }
    }

    public class Sell
    {
        [JsonProperty("parent")]
        public uint Parent { get; set; }
        [JsonProperty("item")]
        public uint Item { get; set; }
        [JsonProperty("price")]
        public uint? Price { get; set; }
    }

    public class SellAttribute
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("group")]
        public string Group { get; set; }
    }

    public class Spell
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("name")]
        public string Name { get; set; }
        [JsonProperty("type")]
        public Fb.Model.EnumValue.SpellType Type { get; set; }
        [JsonProperty("cast")]
        public string Cast { get; set; }
        [JsonProperty("uncast")]
        public string Uncast { get; set; }
        [JsonProperty("concast")]
        public string Concast { get; set; }
        [JsonProperty("message")]
        public string Message { get; set; }
    }

    public class Warp
    {
        [JsonProperty("parent")]
        public uint Parent { get; set; }
        [JsonProperty("before")]
        public Point<ushort> Before { get; set; }
        [JsonProperty("dest")]
        public Dsl Dest { get; set; }
        [JsonProperty("condition")]
        public List<Dsl> Condition { get; set; }
    }

    public class WarpAttribute
    {
        [JsonProperty("map")]
        public uint Map { get; set; }
    }

    public class World
    {
        [JsonProperty("parent")]
        public ushort Parent { get; set; }
        [JsonProperty("id")]
        public ushort Id { get; set; }
        [JsonProperty("name")]
        public string Name { get; set; }
        [JsonProperty("group")]
        public uint Group { get; set; }
        [JsonProperty("offset")]
        public Point<ushort> Offset { get; set; }
        [JsonProperty("map")]
        public uint Map { get; set; }
        [JsonProperty("position")]
        public Point<ushort> Position { get; set; }
    }

    public class WorldAttribute
    {
        [JsonProperty("id")]
        public ushort Id { get; set; }
        [JsonProperty("key")]
        public string Key { get; set; }
    }

    public class Item : Fb.Model.Object
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("price")]
        public uint Price { get; set; }
        [JsonProperty("deposit_price")]
        public uint? DepositPrice { get; set; }
        [JsonProperty("trade")]
        public bool Trade { get; set; }
        [JsonProperty("type")]
        public Fb.Model.EnumValue.ItemType Type { get; set; }
        [JsonProperty("desc")]
        public string Desc { get; set; }
        [JsonProperty("death_penalty")]
        public Fb.Model.EnumValue.DeathPenalty DeathPenalty { get; set; }
        [JsonProperty("capacity")]
        public ushort Capacity { get; set; }
        [JsonProperty("script_active")]
        public string ScriptActive { get; set; }
        [JsonProperty("condition")]
        public List<Dsl> Condition { get; set; }
    }

    public class Life : Fb.Model.Object
    {
        [JsonProperty("hp")]
        public uint Hp { get; set; }
        [JsonProperty("mp")]
        public uint Mp { get; set; }
        [JsonProperty("exp")]
        public uint Exp { get; set; }
        [JsonProperty("defensive_physical")]
        public int DefensivePhysical { get; set; }
        [JsonProperty("defensive_magical")]
        public int DefensiveMagical { get; set; }
    }

    public class Npc : Fb.Model.Object
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("script")]
        public string Script { get; set; }
        [JsonProperty("sell")]
        public List<uint> Sell { get; set; }
        [JsonProperty("buy")]
        public uint? Buy { get; set; }
        [JsonProperty("repair")]
        public bool Repair { get; set; }
        [JsonProperty("hold_money")]
        public bool HoldMoney { get; set; }
        [JsonProperty("hold_item")]
        public bool HoldItem { get; set; }
        [JsonProperty("rename")]
        public bool Rename { get; set; }
    }

    public class Cash : Fb.Model.Item
    {
    }

    public class Consume : Fb.Model.Item
    {
    }

    public class Equipment : Fb.Model.Item
    {
        [JsonProperty("dress")]
        public ushort Dress { get; set; }
        [JsonProperty("durability")]
        public uint Durability { get; set; }
        [JsonProperty("repair")]
        public double? Repair { get; set; }
        [JsonProperty("dress_script")]
        public string DressScript { get; set; }
        [JsonProperty("undress_script")]
        public string UndressScript { get; set; }
        [JsonProperty("hit")]
        public byte Hit { get; set; }
        [JsonProperty("damage")]
        public byte Damage { get; set; }
        [JsonProperty("strength")]
        public byte Strength { get; set; }
        [JsonProperty("intelligence")]
        public byte Intelligence { get; set; }
        [JsonProperty("dexteritry")]
        public byte Dexteritry { get; set; }
        [JsonProperty("base_hp")]
        public uint BaseHp { get; set; }
        [JsonProperty("base_mp")]
        public uint BaseMp { get; set; }
        [JsonProperty("hp_percentage")]
        public float HpPercentage { get; set; }
        [JsonProperty("mp_percentage")]
        public float MpPercentage { get; set; }
        [JsonProperty("healing_cycle")]
        public byte HealingCycle { get; set; }
        [JsonProperty("defensive_physical")]
        public short DefensivePhysical { get; set; }
        [JsonProperty("defensive_magical")]
        public short DefensiveMagical { get; set; }
    }

    public class Mob : Fb.Model.Life
    {
        [JsonProperty("id")]
        public uint Id { get; set; }
        [JsonProperty("size")]
        public Fb.Model.EnumValue.MobSize Size { get; set; }
        [JsonProperty("attack_type")]
        public Fb.Model.EnumValue.MobAttackType AttackType { get; set; }
        [JsonProperty("damage")]
        public Range<uint> Damage { get; set; }
        [JsonProperty("speed")]
        public TimeSpan Speed { get; set; }
        [JsonProperty("drop")]
        public string Drop { get; set; }
        [JsonProperty("attack_script")]
        public string AttackScript { get; set; }
        [JsonProperty("die_script")]
        public string DieScript { get; set; }
    }

    public class Pack : Fb.Model.Item
    {
        [JsonProperty("durability")]
        public uint Durability { get; set; }
    }

    public class Armor : Fb.Model.Equipment
    {
    }

    public class Auxiliary : Fb.Model.Equipment
    {
    }

    public class Bow : Fb.Model.Equipment
    {
    }

    public class Helmet : Fb.Model.Equipment
    {
    }

    public class Ring : Fb.Model.Equipment
    {
    }

    public class Shield : Fb.Model.Equipment
    {
    }

    public class Weapon : Fb.Model.Equipment
    {
        [JsonProperty("damage_small")]
        public Range<uint> DamageSmall { get; set; }
        [JsonProperty("damage_large")]
        public Range<uint> DamageLarge { get; set; }
        [JsonProperty("sound")]
        public ushort Sound { get; set; }
        [JsonProperty("spell")]
        public uint? Spell { get; set; }
        [JsonProperty("rename")]
        public uint? Rename { get; set; }
    }


    public class Dsl
    {
        public Fb.Model.EnumValue.Dsl Header { get; set; }
        public object[] Params { get; set; }

        public class Admin
        {
            [JsonProperty("value")]
            public bool Value { get; set; }

            public static Admin Parse(object[] parameters)
            {
                return new Admin
                {
                    Value = (bool)System.Convert.ChangeType(parameters[0], typeof(bool))
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Admin,
                    Params = new object[] {Value}
                };
            }
        }
        public class ClassT
        {
            [JsonProperty("value")]
            public Fb.Model.EnumValue.Class Value { get; set; }

            public static ClassT Parse(object[] parameters)
            {
                return new ClassT
                {
                    Value = (Fb.Model.EnumValue.Class)Enum.Parse(typeof(Fb.Model.EnumValue.Class), parameters[0].ToString())
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.ClassT,
                    Params = new object[] {Value}
                };
            }
        }
        public class Dexteritry
        {
            [JsonProperty("value")]
            public byte Value { get; set; }

            public static Dexteritry Parse(object[] parameters)
            {
                return new Dexteritry
                {
                    Value = (byte)parameters[0]
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Dexteritry,
                    Params = new object[] {Value}
                };
            }
        }
        public class Intelligence
        {
            [JsonProperty("value")]
            public byte Value { get; set; }

            public static Intelligence Parse(object[] parameters)
            {
                return new Intelligence
                {
                    Value = (byte)parameters[0]
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Intelligence,
                    Params = new object[] {Value}
                };
            }
        }
        public class Item
        {
            [JsonProperty("id")]
            public uint Id { get; set; }
            [JsonProperty("count")]
            public uint Count { get; set; }
            [JsonProperty("percent")]
            public double Percent { get; set; }

            public static Item Parse(object[] parameters)
            {
                return new Item
                {
                    Id = (uint)parameters[0],
                    Count = (uint)parameters[1],
                    Percent = (double)parameters[2]
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Item,
                    Params = new object[] {Id, Count, Percent}
                };
            }
        }
        public class Level
        {
            [JsonProperty("min")]
            public byte? Min { get; set; }
            [JsonProperty("max")]
            public byte? Max { get; set; }

            public static Level Parse(object[] parameters)
            {
                return new Level
                {
                    Min = parameters[0] == null ? null : (byte?)(byte)parameters[0],
                    Max = parameters[1] == null ? null : (byte?)(byte)parameters[1]
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Level,
                    Params = new object[] {Min, Max}
                };
            }
        }
        public class Map
        {
            [JsonProperty("id")]
            public uint Id { get; set; }
            [JsonProperty("x")]
            public ushort X { get; set; }
            [JsonProperty("y")]
            public ushort Y { get; set; }

            public static Map Parse(object[] parameters)
            {
                return new Map
                {
                    Id = (uint)parameters[0],
                    X = (ushort)parameters[1],
                    Y = (ushort)parameters[2]
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Map,
                    Params = new object[] {Id, X, Y}
                };
            }
        }
        public class Promotion
        {
            [JsonProperty("value")]
            public byte Value { get; set; }

            public static Promotion Parse(object[] parameters)
            {
                return new Promotion
                {
                    Value = (byte)parameters[0]
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Promotion,
                    Params = new object[] {Value}
                };
            }
        }
        public class Sex
        {
            [JsonProperty("value")]
            public Fb.Model.EnumValue.Sex Value { get; set; }

            public static Sex Parse(object[] parameters)
            {
                return new Sex
                {
                    Value = (Fb.Model.EnumValue.Sex)Enum.Parse(typeof(Fb.Model.EnumValue.Sex), parameters[0].ToString())
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Sex,
                    Params = new object[] {Value}
                };
            }
        }
        public class Strength
        {
            [JsonProperty("value")]
            public byte Value { get; set; }

            public static Strength Parse(object[] parameters)
            {
                return new Strength
                {
                    Value = (byte)parameters[0]
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.Strength,
                    Params = new object[] {Value}
                };
            }
        }
        public class World
        {
            [JsonProperty("id")]
            public ushort Id { get; set; }
            [JsonProperty("index")]
            public ushort Index { get; set; }

            public static World Parse(object[] parameters)
            {
                return new World
                {
                    Id = (ushort)parameters[0],
                    Index = (ushort)parameters[1]
                };
            }

            public Dsl ToDSL()
            {
                return new Dsl
                {
                    Header = Fb.Model.EnumValue.Dsl.World,
                    Params = new object[] {Id, Index}
                };
            }
        }
    }

    public class ContainerConverter : JsonConverter
    {
        public override bool CanConvert(Type objectType)
        {
            return false;
        }

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var token = JToken.Load(reader);
            var result = Activator.CreateInstance(objectType) as Container;
            result.Load(token.ToString());

            return result;
        }

        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            throw new NotImplementedException();
        }
    }

    [JsonConverter(typeof(ContainerConverter))]
    public abstract class Container
    {
        public class HookFuncs<T>
        {
            public Func<JToken, T> Build { get; set; }
            public Action<T> Built { get; set; }
        }

        public abstract void Load();
        public abstract void Load(string json);
    }

    public class KeyValueContainer<K, V> : Container, IReadOnlyDictionary<K, V> where V : class
    {
        private readonly Dictionary<K, V> _raw = new Dictionary<K, V>();

        public HookFuncs<V> Hook { get; private set; } = new HookFuncs<V>();

        public override void Load()
        {
            var attr = GetType().GetCustomAttribute<TableAttribute>(true);
            Load(File.ReadAllText(attr.Path));
        }

        public override void Load(string json)
        {
            foreach (var (key, value) in JObject.Parse(json))
            {
                var obj = Hook.Build?.Invoke(value) ?? value.ToObject<V>();
                _raw.Add(new JValue(key).ToObject<K>(), obj);

                if (Hook.Built != null)
                    Hook.Built.Invoke(obj);
            }
        }

        public V this[K key] => _raw.TryGetValue(key, out var value) ? value : default;

        public IEnumerable<K> Keys => _raw.Keys;

        public IEnumerable<V> Values => _raw.Values;

        public int Count => _raw.Count;

        public bool ContainsKey(K key) => _raw.ContainsKey(key);

        public IEnumerator<KeyValuePair<K, V>> GetEnumerator() => _raw.GetEnumerator();

        public bool TryGetValue(K key, [MaybeNullWhen(false)] out V value) => _raw.TryGetValue(key, out value);

        IEnumerator IEnumerable.GetEnumerator() => _raw.GetEnumerator();
    }

    public class ArrayContainer<T> : Container, IReadOnlyList<T> where T : class
    {
        private readonly List<T> _raw = new List<T>();

        public HookFuncs<T> Hook { get; private set; } = new HookFuncs<T>();

        public override void Load()
        {
            var attr = GetType().GetCustomAttribute<TableAttribute>(true);
            Load(File.ReadAllText(attr.Path));
        }

        public override void Load(string json)
        {
            foreach (var value in JArray.Parse(json))
            {
                var obj = Hook.Build?.Invoke(value) ?? value.ToObject<T>();
                _raw.Add(obj);
                Hook.Built?.Invoke(obj);
            }
        }

        public T this[int index] => _raw.ElementAtOrDefault(index);

        public int Count => _raw.Count;

        public IEnumerator<T> GetEnumerator() => _raw.GetEnumerator();

        IEnumerator IEnumerable.GetEnumerator() => _raw.GetEnumerator();
    }

    public class TableAttribute : Attribute
    {
        public string Path { get; private set; }

        public TableAttribute(string path)
        {
            Path = path;
        }
    }

    public class Model
    {
        [Table("json/ability.json")]
        public partial class AbilityTable : KeyValueContainer<Fb.Model.EnumValue.Class, KeyValueContainer<byte, Ability>>
        { }
        public AbilityTable Ability { get; private set; } = new AbilityTable();
        [Table("json/ability_attribute.json")]
        public partial class AbilityAttributeTable : KeyValueContainer<Fb.Model.EnumValue.Class, AbilityAttribute>
        { }
        public AbilityAttributeTable AbilityAttribute { get; private set; } = new AbilityAttributeTable();
        [Table("json/board.json")]
        public partial class BoardTable : KeyValueContainer<uint, Board>
        { }
        public BoardTable Board { get; private set; } = new BoardTable();
        [Table("json/buy.json")]
        public partial class BuyTable : KeyValueContainer<uint, KeyValueContainer<uint, Buy>>
        { }
        public BuyTable Buy { get; private set; } = new BuyTable();
        [Table("json/buy_attribute.json")]
        public partial class BuyAttributeTable : KeyValueContainer<uint, BuyAttribute>
        { }
        public BuyAttributeTable BuyAttribute { get; private set; } = new BuyAttributeTable();
        [Table("json/door.json")]
        public partial class DoorTable : KeyValueContainer<uint, Door>
        { }
        public DoorTable Door { get; private set; } = new DoorTable();
        [Table("json/door_pair.json")]
        public partial class DoorPairTable : KeyValueContainer<uint, DoorPair>
        { }
        public DoorPairTable DoorPair { get; private set; } = new DoorPairTable();
        [Table("json/drop.json")]
        public partial class DropTable : KeyValueContainer<string, Drop>
        { }
        public DropTable Drop { get; private set; } = new DropTable();
        [Table("json/equipment.json")]
        public partial class EquipmentTable : KeyValueContainer<uint, Equipment>
        { }
        public EquipmentTable Equipment { get; private set; } = new EquipmentTable();
        [Table("json/item.json")]
        public partial class ItemTable : KeyValueContainer<uint, Item>
        { }
        public ItemTable Item { get; private set; } = new ItemTable();
        [Table("json/life.json")]
        public partial class LifeTable : ArrayContainer<Life>
        { }
        public LifeTable Life { get; private set; } = new LifeTable();
        [Table("json/map.json")]
        public partial class MapTable : KeyValueContainer<uint, Map>
        { }
        public MapTable Map { get; private set; } = new MapTable();
        [Table("json/mob.json")]
        public partial class MobTable : KeyValueContainer<uint, Mob>
        { }
        public MobTable Mob { get; private set; } = new MobTable();
        [Table("json/mob_spawn.json")]
        public partial class MobSpawnTable : KeyValueContainer<uint, ArrayContainer<MobSpawn>>
        { }
        public MobSpawnTable MobSpawn { get; private set; } = new MobSpawnTable();
        [Table("json/mob_spawn_attribute.json")]
        public partial class MobSpawnAttributeTable : KeyValueContainer<uint, MobSpawnAttribute>
        { }
        public MobSpawnAttributeTable MobSpawnAttribute { get; private set; } = new MobSpawnAttributeTable();
        [Table("json/npc.json")]
        public partial class NpcTable : KeyValueContainer<uint, Npc>
        { }
        public NpcTable Npc { get; private set; } = new NpcTable();
        [Table("json/npc_spawn.json")]
        public partial class NpcSpawnTable : KeyValueContainer<uint, ArrayContainer<NpcSpawn>>
        { }
        public NpcSpawnTable NpcSpawn { get; private set; } = new NpcSpawnTable();
        [Table("json/npc_spawn_attribute.json")]
        public partial class NpcSpawnAttributeTable : KeyValueContainer<uint, NpcSpawnAttribute>
        { }
        public NpcSpawnAttributeTable NpcSpawnAttribute { get; private set; } = new NpcSpawnAttributeTable();
        [Table("json/object.json")]
        public partial class ObjectTable : ArrayContainer<Object>
        { }
        public ObjectTable Object { get; private set; } = new ObjectTable();
        [Table("json/promotion.json")]
        public partial class PromotionTable : KeyValueContainer<Fb.Model.EnumValue.Class, KeyValueContainer<byte, Promotion>>
        { }
        public PromotionTable Promotion { get; private set; } = new PromotionTable();
        [Table("json/promotion_attribute.json")]
        public partial class PromotionAttributeTable : KeyValueContainer<Fb.Model.EnumValue.Class, PromotionAttribute>
        { }
        public PromotionAttributeTable PromotionAttribute { get; private set; } = new PromotionAttributeTable();
        [Table("json/recipe.json")]
        public partial class RecipeTable : ArrayContainer<Recipe>
        { }
        public RecipeTable Recipe { get; private set; } = new RecipeTable();
        [Table("json/reward.json")]
        public partial class RewardTable : KeyValueContainer<string, Reward>
        { }
        public RewardTable Reward { get; private set; } = new RewardTable();
        [Table("json/sell.json")]
        public partial class SellTable : KeyValueContainer<uint, KeyValueContainer<uint, Sell>>
        { }
        public SellTable Sell { get; private set; } = new SellTable();
        [Table("json/sell_attribute.json")]
        public partial class SellAttributeTable : KeyValueContainer<uint, SellAttribute>
        { }
        public SellAttributeTable SellAttribute { get; private set; } = new SellAttributeTable();
        [Table("json/spell.json")]
        public partial class SpellTable : KeyValueContainer<uint, Spell>
        { }
        public SpellTable Spell { get; private set; } = new SpellTable();
        [Table("json/warp.json")]
        public partial class WarpTable : KeyValueContainer<uint, ArrayContainer<Warp>>
        { }
        public WarpTable Warp { get; private set; } = new WarpTable();
        [Table("json/warp_attribute.json")]
        public partial class WarpAttributeTable : KeyValueContainer<uint, WarpAttribute>
        { }
        public WarpAttributeTable WarpAttribute { get; private set; } = new WarpAttributeTable();
        [Table("json/world.json")]
        public partial class WorldTable : KeyValueContainer<ushort, KeyValueContainer<ushort, World>>
        { }
        public WorldTable World { get; private set; } = new WorldTable();
        [Table("json/world_attribute.json")]
        public partial class WorldAttributeTable : KeyValueContainer<ushort, WorldAttribute>
        { }
        public WorldAttributeTable WorldAttribute { get; private set; } = new WorldAttributeTable();

        public Container[] Containers { get; private set; }

        public Model()
        {
            Containers = new Container[]
            {
                Ability, 
                AbilityAttribute, 
                Board, 
                Buy, 
                BuyAttribute, 
                Door, 
                DoorPair, 
                Drop, 
                Equipment, 
                Item, 
                Life, 
                Map, 
                Mob, 
                MobSpawn, 
                MobSpawnAttribute, 
                Npc, 
                NpcSpawn, 
                NpcSpawnAttribute, 
                Object, 
                Promotion, 
                PromotionAttribute, 
                Recipe, 
                Reward, 
                Sell, 
                SellAttribute, 
                Spell, 
                Warp, 
                WarpAttribute, 
                World, 
                WorldAttribute
            };
        }
    }
}