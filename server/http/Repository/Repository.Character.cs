using Dapper;
using Http.Extension;
using Http.Model;
using Http.Service;
using Newtonsoft.Json;
using System.Data;

namespace Http.Reepository
{
    /// <summary>
    /// Provides repository functionality for character data management.
    /// Implements Redis value-based caching with database persistence for character operations.
    /// </summary>
    public class CharacterRepository : RedisValueRepository<Character, CharacterKey>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CharacterRepository"/> class.
        /// </summary>
        /// <param name="dbContext">The database context for connection management.</param>
        /// <param name="redisService">The Redis service for cache operations.</param>
        /// <param name="distributedLock">The distributed lock service for concurrency control.</param>
        /// <param name="dbExecuteService">The write-back service for asynchronous database writes.</param>
        public CharacterRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }


        /// <summary>
        /// Retrieves a character by world and their unique identifier.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="id">The unique identifier of the character.</param>
        /// <returns>The character if found; otherwise, null.</returns>
        public async Task<Character> Get(uint world, uint id)
        {
            return await base.Get(world, new CharacterKey { Id = id });
        }

        /// <summary>
        /// Generates the SQL SELECT statement for retrieving a character by ID.
        /// </summary>
        /// <param name="key">The character key containing the character ID.</param>
        /// <returns>A SQL SELECT statement for the character.</returns>
        protected override string OnSelect(CharacterKey key)
        {
            return $"""
                SELECT * FROM `user`
                WHERE `id` = {key.Id}
                LIMIT 1;
                """;
        }

        /// <summary>
        /// Generates the SQL UPSERT statement for a character with all character properties.
        /// Includes comprehensive character data such as stats, appearance, position, and equipment colors.
        /// </summary>
        /// <param name="value">The character to upsert.</param>
        /// <returns>A SQL UPSERT statement for the character.</returns>
        protected override string OnUpsert(Character value)
        {
            var sql = $"""
                INSERT INTO `user` (
                    `id`,
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
                    `buffs`,
                    `title`,
                    `pending_listings`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Id.Escape()},
                    {value.Name.Escape()},
                    {value.Pw.Escape()},
                    {((byte)value.Role).Escape()},
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
                    {JsonConvert.SerializeObject(value.Buffs).Escape()},
                    {value.Title.Escape()},
                    {(value.PendingListings == null || value.PendingListings.Count == 0 ? null : JsonConvert.SerializeObject(value.PendingListings)).Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
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
                    `buffs`=VALUES(`buffs`),
                    `title`=VALUES(`title`),
                    `pending_listings`=VALUES(`pending_listings`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }

        /// <summary>
        /// Retrieves a character ID by world and their name using a stored procedure.
        /// Uses the world-specific database connection for name lookup operations.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="name">The character name to look up.</param>
        /// <returns>The character ID if found; otherwise, null.</returns>
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

        /// <summary>
        /// Retrieves a character name by world and their unique identifier.
        /// Uses the world-specific database connection for name lookup operations.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="id">The unique identifier of the character.</param>
        /// <returns>The character name if found; otherwise, null.</returns>
        public async Task<string> GetName(uint world, uint id)
        {
            await using var conn = _dbContext.GetGlobalConnection(world);
            var result = await conn.QueryFirstOrDefaultAsync<CharacterName>($"SELECT id, name FROM name WHERE id = {id}");
            return result?.Name;
        }

        /// <summary>
        /// Retrieves multiple character names by world and their unique identifiers in a single query.
        /// Uses the world-specific database connection for batch name lookup operations.
        /// </summary>
        /// <param name="world">The world identifier (e.g., 1, 2). Use 0 for unified-global.</param>
        /// <param name="ids">The collection of character IDs to look up.</param>
        /// <returns>A read-only dictionary mapping character IDs to their names.</returns>
        public async Task<IReadOnlyDictionary<uint, string>> GetName(uint world, IEnumerable<uint> ids)
        {
            if (ids.Any() == false)
                return new Dictionary<uint, string>();

            await using var conn = _dbContext.GetGlobalConnection(world);
            var result = await conn.QueryAsync<CharacterName>($"SELECT id, name FROM name WHERE id IN ({string.Join(',', ids)})");
            return result.ToDictionary(x => x.Id, x => x.Name);
        }
    }
}