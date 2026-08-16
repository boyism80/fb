using Http.Extension;
using Http.Model;
using Http.Service;

namespace Http.Reepository
{
    public class OptionRepository : RedisValueRepository<Option, OptionKey>
    {
        public OptionRepository(DbContext dbContext,
            RedisService redisService,
            RedisDistributedLockService distributedLock,
            WriteBackService dbExecuteService) : base(dbContext, redisService, distributedLock, dbExecuteService)
        {
        }


        public async Task<Option> Get(uint world, uint uid)
        {
            return await base.Get(world, new OptionKey { Uid = uid });
        }

        protected override string OnSelect(OptionKey key)
        {
            return $"""
                SELECT * FROM `option`
                WHERE `uid` = {key.Uid} AND `deleted` = 0
                LIMIT 1;
                """;
        }

        protected override OptionKey GetKeyFromRow(Option row)
        {
            return new OptionKey { Uid = row.Uid };
        }

        protected override string OnUpsert(Option value)
        {
            var sql = $"""
                INSERT INTO `option` (
                    `uid`,
                    `whisper`,
                    `group`,
                    `roar`,
                    `news`,
                    `magic_effect`,
                    `weather_effect`,
                    `fixed_move`,
                    `trade`,
                    `fast_move`,
                    `effect_sound`,
                    `pk_protect`,
                    `visible_helmet`,
                    `deleted`,
                    `created_date`,
                    `updated_date`)
                VALUES (
                    {value.Uid.Escape()},
                    {value.Whisper.Escape()},
                    {value.Group.Escape()},
                    {value.Roar.Escape()},
                    {value.News.Escape()},
                    {value.MagicEffect.Escape()},
                    {value.WeatherEffect.Escape()},
                    {value.FixedMove.Escape()},
                    {value.Trade.Escape()},
                    {value.FastMove.Escape()},
                    {value.EffectSound.Escape()},
                    {value.PkProtect.Escape()},
                    {value.VisibleHelmet.Escape()},
                    0,
                    {value.CreatedDate.Escape()},
                    {value.UpdatedDate.Escape()})
                ON DUPLICATE KEY UPDATE 
                    `whisper`=VALUES(`whisper`),
                    `group`=VALUES(`group`),
                    `roar`=VALUES(`roar`),
                    `news`=VALUES(`news`),
                    `magic_effect`=VALUES(`magic_effect`),
                    `weather_effect`=VALUES(`weather_effect`),
                    `fixed_move`=VALUES(`fixed_move`),
                    `trade`=VALUES(`trade`),
                    `fast_move`=VALUES(`fast_move`),
                    `effect_sound`=VALUES(`effect_sound`),
                    `pk_protect`=VALUES(`pk_protect`),
                    `visible_helmet`=VALUES(`visible_helmet`),
                    `updated_date`=VALUES(`updated_date`);
                """;

            return sql;
        }
    }
}
