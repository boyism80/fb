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
            return $"""
                UPDATE user
                SET look = {value.Look},
                    color = {value.Color},
                    sex = {value.Sex},
                    nation = {value.Nation},
                    creature = {value.Creature},
                    map = {value.Map},
                    position_x = {value.PositionX},
                    position_y = {value.PositionY},
                    direction = {value.Direction},
                    state = {value.State},
                    class = {value.Class},
                    promotion = {value.Promotion},
                    exp = {value.Exp},
                    money = {value.Money},
                    deposited_money = {value.DepositedMoney},
                    disguise = {value.Disguise?.ToString() ?? "NULL"},
                    hp = {value.Hp},
                    base_hp = {value.BaseHp},
                    additional_hp = {value.AdditionalHp},
                    mp = {value.Mp},
                    base_mp = {value.BaseMp},
                    additional_mp = {value.AdditionalMp},
                    weapon_color = {value.WeaponColor?.ToString() ?? "NULL"},
                    helmet_color = {value.HelmetColor?.ToString() ?? "NULL"},
                    armor_color = {value.ArmorColor?.ToString() ?? "NULL"},
                    shield_color = {value.ShieldColor?.ToString() ?? "NULL"},
                    ring_left_color = {value.RingLeftColor?.ToString() ?? "NULL"},
                    ring_right_color = {value.RingRightColor?.ToString() ?? "NULL"},
                    aux_top_color = {value.AuxTopColor?.ToString() ?? "NULL"},
                    aux_bot_color = {value.AuxBotColor?.ToString() ?? "NULL"},
                    clan = {value.Clan?.ToString() ?? "NULL"}
                WHERE user.id = {value.Id} LIMIT 1;
                """;
        }
    }
}