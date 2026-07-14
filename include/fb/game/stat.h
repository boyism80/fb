
#ifndef __STAT_H__
#define __STAT_H__

#include <limits>
#include <stdint.h>
#include <memory>
#include <async/task.h>

namespace fb::game {

class life;
class object;
class character;
class mob;

template <typename T>
class stat_value
{
public:
    T base = 0;
    T buff = 0;

public:
    T value() const
    {
        auto limit = std::numeric_limits<T>::max();
        if (limit - base < buff)
            return limit;

        return base + buff;
    }
};

class stat
{
private:
    life&    owner;
    uint32_t _hp                = 0;
    uint32_t _mp                = 0;
    int32_t  _buff_hp           = 0;
    int32_t  _buff_mp           = 0;
    uint8_t  _buff_str          = 0;
    uint8_t  _buff_dex          = 0;
    uint8_t  _buff_int          = 0;
    int8_t   _buff_phydef       = 0;
    int8_t   _buff_magdef       = 0;
    int8_t   _buff_dam          = 0;
    int8_t   _buff_hit          = 0;
    uint32_t _buff_regenerative = 0;

protected:
    stat(life& owner);

public:
    stat(const stat& other);
    stat(stat&& other);
    virtual ~stat() = default;

    stat& operator= (const stat& other) = delete;
    stat& operator= (stat&& other)      = delete;

public:
    // clang-format off
    virtual uint32_t              base_hp() const = 0;
    virtual uint32_t              base_mp() const = 0;
    virtual uint8_t               base_str() const = 0;
    virtual uint8_t               base_dex() const = 0;
    virtual uint8_t               base_int() const = 0;
    virtual int8_t                base_phydef() const = 0;
    virtual int8_t                base_magdef() const = 0;
    virtual uint8_t               base_dam() const = 0;
    virtual uint8_t               base_hit() const = 0;
    virtual uint32_t              base_regenerative() const = 0;
    virtual int32_t               buff_hp() const;
    virtual void                  buff_hp(int32_t value);
    virtual int32_t               buff_mp() const;
    virtual void                  buff_mp(int32_t value);
    virtual uint8_t               buff_str() const;
    virtual void                  buff_str(uint8_t value);
    virtual uint8_t               buff_dex() const;
    virtual void                  buff_dex(uint8_t value);
    virtual uint8_t               buff_int() const;
    virtual void                  buff_int(uint8_t value);
    virtual int8_t                buff_phydef() const;
    virtual void                  buff_phydef(int8_t value);
    virtual int8_t                buff_magdef() const;
    virtual void                  buff_magdef(int8_t value);
    virtual int8_t                buff_dam() const;
    virtual void                  buff_dam(int8_t value);
    virtual int8_t                buff_hit() const;
    virtual void                  buff_hit(int8_t value);
    virtual uint32_t              buff_regenerative() const;
    virtual void                  buff_regenerative(uint32_t value);
    virtual uint32_t              hp() const;
    virtual async::task<void>     hp(uint32_t value, bool notify = true);
    virtual async::task<uint32_t> heal(uint32_t value, fb::game::object* from = nullptr, bool notify = true);
    virtual async::task<uint32_t> damage(uint32_t value, std::shared_ptr<fb::game::object> from = nullptr, bool critical = false, float rate = 1.0f, bool physical = true, bool fixed = false, bool notify = true);
    virtual uint32_t              mp() const;
    virtual async::task<void>     mp(uint32_t value, bool notify = true);
    virtual async::task<uint32_t> mp_up(uint32_t value, fb::game::object* from = nullptr, bool notify = true);
    virtual async::task<uint32_t> mp_down(uint32_t value, fb::game::object* from = nullptr, bool notify = true);
    virtual uint32_t              maxhp() const;
    virtual uint32_t              maxmp() const;
    virtual uint8_t               str() const;
    virtual uint8_t               dex() const;
    virtual uint8_t               intelligence() const;
    virtual int8_t                phydef() const;
    virtual int8_t                magdef() const;
    virtual int8_t                dam() const;
    virtual int8_t                hit() const;
    virtual uint32_t              regenerative() const;
    // clang-format on
};

class character_stat : public stat
{
private:
    uint32_t _max_hp       = 0;
    uint32_t _max_mp       = 0;
    uint8_t  _str          = 0;
    uint8_t  _dex          = 0;
    uint8_t  _int          = 0;
    int32_t  _phydef       = 0;
    int32_t  _magdef       = 0;
    int32_t  _dam          = 0;
    int32_t  _hit          = 0;
    uint32_t _regenerative = 0;

public:
    character& owner;

public:
    character_stat(character& owner);
    ~character_stat() = default;

public:
    // clang-format off
    async::task<void>     base_hp(uint32_t value, bool notify = true);
    async::task<void>     base_mp(uint32_t value, bool notify = true);
    async::task<void>     base_str(uint8_t value, bool notify = true);
    async::task<void>     base_dex(uint8_t value, bool notify = true);
    async::task<void>     base_int(uint8_t value, bool notify = true);
    async::task<void>     base_phydef(int8_t value, bool notify = true);
    void                  base_magdef(int8_t value, bool notify = true);
    void                  base_dam(uint8_t value, bool notify = true);
    void                  base_hit(uint8_t value, bool notify = true);
    void                  base_regenerative(uint32_t value, bool notify = true);
    uint32_t              base_hp() const;
    uint32_t              base_mp() const;
    uint8_t               base_str() const;
    virtual uint8_t       str() const;
    uint8_t               base_dex() const;
    virtual uint8_t       dex() const;
    uint8_t               base_int() const;
    virtual uint8_t       intelligence() const;
    int8_t                base_phydef() const;
    virtual int8_t        phydef() const;
    int8_t                base_magdef() const;
    virtual int8_t        magdef() const;
    uint8_t               base_dam() const;
    virtual int8_t        dam() const;
    uint8_t               base_hit() const;
    virtual int8_t        hit() const;
    uint32_t              base_regenerative() const;
    virtual uint32_t      maxhp() const override;
    virtual uint32_t      maxmp() const override;
    virtual uint32_t      regenerative() const override;
    async::task<uint32_t> damage(uint32_t value, std::shared_ptr<fb::game::object> from = nullptr, bool critical = false, float rate = 1.0f, bool physical = true, bool fixed = false, bool notify = true) override final;
    // clang-format on
};

class mob_stat : public stat
{
public:
    mob& owner;

public:
    mob_stat(mob& owner);
    mob_stat(const mob_stat& other);
    mob_stat(mob_stat&& other);
    ~mob_stat() = default;

public:
    // clang-format off
    uint32_t              base_hp() const override final;
    uint32_t              base_mp() const override final;
    uint8_t               base_str() const override final;
    uint8_t               base_dex() const override final;
    uint8_t               base_int() const override final;
    int8_t                base_phydef() const override final;
    int8_t                base_magdef() const override final;
    uint8_t               base_dam() const override final;
    uint8_t               base_hit() const override final;
    uint32_t              base_regenerative() const override final;
    async::task<uint32_t> damage(uint32_t value, std::shared_ptr<fb::game::object> from = nullptr, bool critical = false, float rate = 1.0f, bool physical = true, bool fixed = false, bool notify = true) override final;
    // clang-format on
};

} // namespace fb::game

#endif