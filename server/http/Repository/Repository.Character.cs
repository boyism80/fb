using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;
using System.Data;

namespace Http.Reepository
{
    public class CharacterRepository : RedisValueRepository<Character, CharacterKey>
    {
        public CharacterRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }


        public async Task<Character> Get(uint world, uint id)
        {
            return await base.Get(world, new CharacterKey { Id = id });
        }

        protected override string OnSelect(CharacterKey key)
        {
            return $"""
                SELECT * FROM `user`
                WHERE `id` = {key.Id}
                LIMIT 1;
                """;
        }

        protected override string OnSelectMany(IReadOnlyList<CharacterKey> keys)
        {
            return $"SELECT * FROM `user` WHERE `id` IN ({string.Join(",", keys.Select(k => k.Id))});";
        }

        protected override CharacterKey GetKeyFromRow(Character row)
        {
            return new CharacterKey { Id = row.Id };
        }

        public async Task<IReadOnlyDictionary<uint, Character>> GetMany(uint world, IReadOnlyList<uint> ids)
        {
            if (ids == null || ids.Count == 0)
                return new Dictionary<uint, Character>();

            var keys = ids.Distinct().Select(id => new CharacterKey { Id = id }).ToList();
            var list = await base.GetMany(world, keys);
            return list.ToDictionary(c => c.Id, c => c);
        }

        protected override string OnUpsert(Character value)
        {
            var sql = $"""
                INSERT INTO `user` (
                    `id`,
                    `world`,
                    `name`,
                    `pw`,
                    `role`,
                    `birth`,
                    `look`,
                    `color`,
                    `gender`,
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
                    `mimicry`,
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
                    `buffs`,
                    `title`,
                    `super_hide`,
                    `deleted`,
                    `created_date`,
                    `updated_date`,
                    `first_login_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.World.Escape()},
                    {value.Name.Escape()},
                    {value.Pw.Escape()},
                    {value.Role.Escape()},
                    {value.Birth.Escape()},
                    {value.Look.Escape()},
                    {value.Color.Escape()},
                    {value.Gender.Escape()},
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
                    {value.Mimicry.Escape()},
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
                    {value.Buffs.Escape()},
                    {value.Title.Escape()},
                    {value.SuperHide.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()},
                    {value.FirstLoginDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `pw`=VALUES(`pw`),
                    `role`=VALUES(`role`),
                    `birth`=VALUES(`birth`),
                    `look`=VALUES(`look`),
                    `color`=VALUES(`color`),
                    `gender`=VALUES(`gender`),
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
                    `mimicry`=VALUES(`mimicry`),
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
                    `buffs`=VALUES(`buffs`),
                    `title`=VALUES(`title`),
                    `super_hide`=VALUES(`super_hide`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`),
                    `first_login_date`=COALESCE(VALUES(`first_login_date`), `first_login_date`);
                """;

            return sql;
        }

        public async Task<uint?> GetCharacterId(uint world, string name)
        {
            await using var conn = _dbContext.GetGlobalConnection(world);
            var result = await conn.QueryAsync<uint>("USP_NAME_GET_ID", new
            {
                n = name
            }, commandType: CommandType.StoredProcedure);

            if (result.Any())
                return result.ElementAt(0);

            return null;
        }

        public async Task<string> GetName(uint world, uint id)
        {
            await using var conn = _dbContext.GetGlobalConnection(world);
            var result = await conn.QueryFirstOrDefaultAsync<CharacterName>($"SELECT id, name FROM name_registry WHERE id = {id}");
            return result?.Name;
        }

        public async Task<IReadOnlyDictionary<uint, string>> GetName(uint world, IEnumerable<uint> ids)
        {
            if (ids.Any() == false)
                return new Dictionary<uint, string>();

            await using var conn = _dbContext.GetGlobalConnection(world);
            var result = await conn.QueryAsync<CharacterName>($"SELECT id, name FROM name_registry WHERE id IN ({string.Join(',', ids)})");
            return result.ToDictionary(x => x.Id, x => x.Name);
        }
    }
}
