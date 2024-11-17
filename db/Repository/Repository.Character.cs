using Db.Model;
using Db.Service;
using http.Service;

namespace Db.Reepository
{
    public class CharacterRepository : RedisValueRepository<Character, CharacterKey>
    {
        public CharacterRepository(DbContext dbContext,
            RedisService redisService,
            DbExecuteService dbExecuteService) : base(dbContext, redisService, dbExecuteService)
        {
        }

        public async Task<Character> Get(uint id)
        {
            return await Get(new CharacterKey { Id = id });
        }

        public override async Task Set(Character value)
        {
            value.LastLogin = DateTime.Now;
            await base.Set(value);
        }

        protected override string OnSelect(CharacterKey key)
        {
            return $"""
                SELECT * FROM `user`
                WHERE `id` = {key.Id}
                LIMIT 1;
                """;
        }

        protected override string OnUpsert(Character value)
        {
            var sql = $"""
                INSERT INTO `user` (
                    `id`,
                    `name`,
                    `pw`,
                    `birth`,
                    `last_login`,
                    `look`,
                    `color`,
                    `sex`,
                    `nation`,
                    `creature`,
                    `map`,
                    `position_x`,
                    `position_y`,
                    `direction`,
                    `state`,
                    `class`,
                    `promotion`,
                    `exp`,
                    `money`,
                    `deposited_money`,
                    `disguise`,
                    `hp`,
                    `base_hp`,
                    `additional_hp`,
                    `mp`,
                    `base_mp`,
                    `additional_mp`,
                    `weapon_color`,
                    `helmet_color`,
                    `armor_color`,
                    `shield_color`,
                    `ring_left_color`,
                    `ring_right_color`,
                    `aux_top_color`,
                    `aux_bot_color`,
                    `clan`)
                VALUES (
                    '{value.Id}',
                    '{value.Name}',
                    '{value.Pw}',
                    {value.Birth?.ToString() ?? "NULL"},
                    '{value.LastLogin:yyyy-MM-dd HH:mm:ss}',
                    {value.Look},
                    {value.Color},
                    {value.Sex},
                    {value.Nation},
                    {value.Creature?.ToString() ?? "NULL"},
                    {value.Map},
                    {value.PositionX},
                    {value.PositionY},
                    {value.Direction},
                    {value.State},
                    {value.Class},
                    {value.Promotion},
                    {value.Exp},
                    {value.Money},
                    {value.DepositedMoney},
                    {value.Disguise?.ToString() ?? "NULL"},
                    {value.Hp},
                    {value.BaseHp},
                    {value.AdditionalHp},
                    {value.Mp},
                    {value.BaseMp},
                    {value.AdditionalMp},
                    {value.WeaponColor?.ToString() ?? "NULL"},
                    {value.HelmetColor?.ToString() ?? "NULL"},
                    {value.ArmorColor?.ToString() ?? "NULL"},
                    {value.ShieldColor?.ToString() ?? "NULL"},
                    {value.RingLeftColor?.ToString() ?? "NULL"},
                    {value.RingRightColor?.ToString() ?? "NULL"},
                    {value.AuxTopColor?.ToString() ?? "NULL"},
                    {value.AuxBotColor?.ToString() ?? "NULL"},
                    {value.Clan?.ToString() ?? "NULL"})
                ON DUPLICATE KEY UPDATE 
                    pw=VALUES(pw),
                    birth=VALUES(birth),
                    last_login=VALUES(last_login),
                    look=VALUES(look),
                    color=VALUES(color),
                    sex=VALUES(sex),
                    nation=VALUES(nation),
                    creature=VALUES(creature),
                    map=VALUES(map),
                    position_x=VALUES(position_x),
                    position_y=VALUES(position_y),
                    direction=VALUES(direction),
                    state=VALUES(state),
                    class=VALUES(class),
                    promotion=VALUES(promotion),
                    exp=VALUES(exp),
                    money=VALUES(money),
                    deposited_money=VALUES(deposited_money),
                    disguise=VALUES(disguise),
                    hp=VALUES(hp),
                    base_hp=VALUES(base_hp),
                    additional_hp=VALUES(additional_hp),
                    mp=VALUES(mp),
                    base_mp=VALUES(base_mp),
                    additional_mp=VALUES(additional_mp),
                    weapon_color=VALUES(weapon_color),
                    helmet_color=VALUES(helmet_color),
                    armor_color=VALUES(armor_color),
                    shield_color=VALUES(shield_color),
                    ring_left_color=VALUES(ring_left_color),
                    ring_right_color=VALUES(ring_right_color),
                    aux_top_color=VALUES(aux_top_color),
                    aux_bot_color=VALUES(aux_bot_color),
                    clan=VALUES(clan);
                """;

            return sql;
        }
    }
}