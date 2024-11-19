using Db.Extension;
using Db.Model;
using Db.Service;
using http.Service;

namespace Db.Reepository
{
    public class OptionRepository : RedisValueRepository<Option, OptionKey>
    {
        public OptionRepository(DbContext dbContext,
            RedisService redisService,
            DbExecuteService dbExecuteService) : base(dbContext, redisService, dbExecuteService)
        {
        }

        public async Task<Option> Get(uint uid)
        {
            return await Get(new OptionKey { Uid = uid });
        }

        protected override string OnSelect(OptionKey key)
        {
            return $"""
                SELECT * FROM `option`
                WHERE `uid` = {key.Uid}
                LIMIT 1;
                """;
        }

        protected override string OnUpsert(Option value)
        {
            var sql = $"""
                INSERT INTO `option` (
                    `uid`,
                    `whisper`,
                    `group`,
                    `roar`,
                    `roar_worlds`,
                    `magic_effect`,
                    `weather_effect`,
                    `fixed_move`,
                    `trade`,
                    `fast_move`,
                    `effect_sound`,
                    `pk_protect`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Uid.Escape()},
                    {value.Whisper.Escape()},
                    {value.Group.Escape()},
                    {value.Roar.Escape()},
                    {value.RoarWorlds.Escape()},
                    {value.MagicEffect.Escape()},
                    {value.WeatherEffect.Escape()},
                    {value.FixedMove.Escape()},
                    {value.Trade.Escape()},
                    {value.FastMove.Escape()},
                    {value.EffectSound.Escape()},
                    {value.PkProtect.Escape()},
                    {value.Deleted.Escape()},
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `whisper`=VALUES(`whisper`),
                    `group`=VALUES(`group`),
                    `roar`=VALUES(`roar`),
                    `roar_worlds`=VALUES(`roar_worlds`),
                    `magic_effect`=VALUES(`magic_effect`),
                    `weather_effect`=VALUES(`weather_effect`),
                    `fixed_move`=VALUES(`fixed_move`),
                    `trade`=VALUES(`trade`),
                    `fast_move`=VALUES(`fast_move`),
                    `effect_sound`=VALUES(`effect_sound`),
                    `pk_protect`=VALUES(`pk_protect`),
                    `deleted`=VALUES(`deleted`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}