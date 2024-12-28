using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class CharacterRepository : RedisValueRepository<Character, CharacterKey>
    {
        public CharacterRepository(DbContext dbContext,
            RedisService redisService,
            WriteBackService dbExecuteService) : base(dbContext, redisService, dbExecuteService)
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
            var sql = $"""
                INSERT INTO `user` (
                    `id`,
                    `name`,
                    `pw`,
                    `admin`,
                    `birth`,
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
                    `level`,
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
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Name.Escape()},
                    {value.Pw.Escape()},
                    {value.Admin.Escape()},
                    {value.Birth.Escape()},
                    {value.Look.Escape()},
                    {value.Color.Escape()},
                    {value.Sex.Escape()},
                    {value.Nation.Escape()},
                    {value.Creature.Escape()},
                    {value.Map.Escape()},
                    {value.PositionX.Escape()},
                    {value.PositionY.Escape()},
                    {value.Direction.Escape()},
                    {value.State.Escape()},
                    {value.Class.Escape()},
                    {value.Promotion.Escape()},
                    {value.Level.Escape()},
                    {value.Exp.Escape()},
                    {value.Money.Escape()},
                    {value.DepositedMoney.Escape()},
                    {value.Disguise.Escape()},
                    {value.Hp.Escape()},
                    {value.BaseHp.Escape()},
                    {value.AdditionalHp.Escape()},
                    {value.Mp.Escape()},
                    {value.BaseMp.Escape()},
                    {value.AdditionalMp.Escape()},
                    {value.WeaponColor.Escape()},
                    {value.HelmetColor.Escape()},
                    {value.ArmorColor.Escape()},
                    {value.ShieldColor.Escape()},
                    {value.RingLeftColor.Escape()},
                    {value.RingRightColor.Escape()},
                    {value.AuxTopColor.Escape()},
                    {value.AuxBotColor.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `pw`=VALUES(`pw`),
                    `admin`=VALUES(`admin`),
                    `birth`=VALUES(`birth`),
                    `look`=VALUES(`look`),
                    `color`=VALUES(`color`),
                    `sex`=VALUES(`sex`),
                    `nation`=VALUES(`nation`),
                    `creature`=VALUES(`creature`),
                    `map`=VALUES(`map`),
                    `position_x`=VALUES(`position_x`),
                    `position_y`=VALUES(`position_y`),
                    `direction`=VALUES(`direction`),
                    `state`=VALUES(`state`),
                    `class`=VALUES(`class`),
                    `promotion`=VALUES(`promotion`),
                    `level`=VALUES(`level`),
                    `exp`=VALUES(`exp`),
                    `money`=VALUES(`money`),
                    `deposited_money`=VALUES(`deposited_money`),
                    `disguise`=VALUES(`disguise`),
                    `hp`=VALUES(`hp`),
                    `base_hp`=VALUES(`base_hp`),
                    `additional_hp`=VALUES(`additional_hp`),
                    `mp`=VALUES(`mp`),
                    `base_mp`=VALUES(`base_mp`),
                    `additional_mp`=VALUES(`additional_mp`),
                    `weapon_color`=VALUES(`weapon_color`),
                    `helmet_color`=VALUES(`helmet_color`),
                    `armor_color`=VALUES(`armor_color`),
                    `shield_color`=VALUES(`shield_color`),
                    `ring_left_color`=VALUES(`ring_left_color`),
                    `ring_right_color`=VALUES(`ring_right_color`),
                    `aux_top_color`=VALUES(`aux_top_color`),
                    `aux_bot_color`=VALUES(`aux_bot_color`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}