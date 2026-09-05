local jar = require('lib.butterfly_jar')

return {
    on_mob_damaged = function(me, you)
        jar.try_catch(you, me)
    end,
}
