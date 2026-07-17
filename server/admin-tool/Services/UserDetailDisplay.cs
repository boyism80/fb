using Fb.Model;
using Fb.Model.EnumValue;
using Http.Model;
using System.Reflection;
using System.Runtime.Serialization;

namespace AdminTool.Services
{
    public static class UserDetailDisplay
    {
        public static string FormatRole(Role role) => role switch
        {
            Role.User => "일반 유저",
            Role.Moderator => "모더레이터",
            Role.Admin => "관리자",
            Role.Superadmin => "슈퍼관리자",
            Role.Owner => "오너",
            _ => role.ToString()
        };

        public static string FormatClass(byte classValue) => ((Class)classValue) switch
        {
            Class.None => "미설정",
            Class.Warrior => "전사",
            Class.Rogue => "도적",
            Class.Mage => "마법사",
            Class.Poet => "시인",
            _ => classValue.ToString()
        };

        public static string FormatPromotion(byte promotion) => promotion switch
        {
            0 => "미승급",
            1 => "1차",
            2 => "2차",
            _ => $"{promotion}차"
        };

        public static string FormatGender(byte gender) => gender switch
        {
            0 => "남",
            1 => "여",
            _ => gender.ToString()
        };

        public static string FormatNation(byte nation) => ((Nation)nation) switch
        {
            Nation.Goguryeo => "고구려",
            Nation.Buyeo => "부여",
            _ => nation.ToString()
        };

        public static string FormatCreature(byte? creature) => creature switch
        {
            null => "-",
            (byte)Creature.Phoenix => "주작",
            (byte)Creature.Tiger => "백호",
            (byte)Creature.Turtle => "현무",
            (byte)Creature.Dragon => "청룡",
            _ => creature.ToString()
        };

        public static string FormatState(byte state) => ((State)state) switch
        {
            State.Normal => "일반",
            State.Ghost => "사망",
            State.Cloack => "투명",
            State.Riding => "탑승",
            State.Disguise => "변장",
            State.HalfCloack => "반투명",
            State.AdvCloack => "강투명",
            _ => state.ToString()
        };

        public static string FormatClanRole(uint role) => ((ClanRole)role) switch
        {
            ClanRole.Mate => "일원",
            ClanRole.Officer => "장교",
            ClanRole.Deputy => "부군주",
            ClanRole.Master => "군주",
            _ => role.ToString()
        };

        public static string FormatMapName(uint mapId)
        {
            try
            {
                if (Table.Map.TryGetValue(mapId, out var map) && !string.IsNullOrWhiteSpace(map?.Name))
                    return map.Name;
            }
            catch
            {
                // ignore lookup errors
            }

            return $"Map {mapId}";
        }

        public static string FormatSpellName(int modelId)
        {
            try
            {
                if (Table.Spell.TryGetValue((uint)modelId, out var spell) && !string.IsNullOrWhiteSpace(spell?.Name))
                    return spell.Name;
            }
            catch
            {
                // ignore lookup errors
            }

            return $"#{modelId}";
        }

        public static string FormatItemName(uint modelId)
        {
            try
            {
                var item = Table.Item[modelId];
                return item?.Name ?? $"#{modelId}";
            }
            catch
            {
                return $"#{modelId}";
            }
        }

        public static string FormatBuffName(uint modelId)
        {
            var spellName = FormatSpellName((int)modelId);
            if (!spellName.StartsWith('#'))
                return spellName;

            return $"버프 #{modelId}";
        }

        public static string FormatBuffTime(uint seconds)
        {
            if (seconds <= 0)
                return "-";

            var ts = TimeSpan.FromSeconds(seconds);
            if (ts.TotalHours >= 1)
                return $"{(int)ts.TotalHours:D2}:{ts.Minutes:D2}:{ts.Seconds:D2}";

            return $"{ts.Minutes:D2}:{ts.Seconds:D2}";
        }

        public static string FormatPartsName(short parts)
        {
            var equipmentParts = (EquipmentParts)parts;
            return equipmentParts switch
            {
                EquipmentParts.Weapon => "무기",
                EquipmentParts.Armor => "갑옷",
                EquipmentParts.Shield => "방패",
                EquipmentParts.Helmet => "투구",
                EquipmentParts.LeftHand => "반지 (좌)",
                EquipmentParts.RightHand => "반지 (우)",
                EquipmentParts.LeftAux => "보조 (좌)",
                EquipmentParts.RightAux => "보조 (우)",
                EquipmentParts.Unknown => "알 수 없음",
                _ => $"알 수 없음 ({parts})"
            };
        }

        public static string FormatDateTime(DateTime? value)
        {
            if (!value.HasValue)
                return "-";

            return value.Value.ToLocalTime().ToString("yyyy-MM-dd HH:mm");
        }

        public static string FormatMoney(ulong value) => value.ToString("N0");

        public static string FormatMatchFormat(uint matchType)
        {
            try
            {
                if (!Enum.IsDefined(typeof(Fb.Model.EnumValue.MatchType), (int)matchType))
                    return "-";

                var enumValue = (Fb.Model.EnumValue.MatchType)matchType;
                if (Table.Matchmaking.TryGetValue(enumValue, out var config) && config != null)
                {
                    if (config.MemberCount > 0 && config.TeamCount == 2)
                        return $"{config.MemberCount}v{config.MemberCount}";
                }
            }
            catch
            {
                // ignore lookup errors
            }

            return "-";
        }

        public static string FormatMatchTypeName(uint matchType)
        {
            if (!Enum.IsDefined(typeof(Fb.Model.EnumValue.MatchType), (int)matchType))
                return $"UNKNOWN_{matchType}";

            var enumValue = (Fb.Model.EnumValue.MatchType)matchType;
            var member = typeof(Fb.Model.EnumValue.MatchType).GetMember(enumValue.ToString()).FirstOrDefault();
            var enumMember = member?.GetCustomAttribute<EnumMemberAttribute>();
            if (!string.IsNullOrWhiteSpace(enumMember?.Value))
                return enumMember.Value;

            return enumValue.ToString();
        }

        public static string FormatMatchType(uint matchType)
        {
            var name = FormatMatchTypeName(matchType);
            var format = FormatMatchFormat(matchType);
            if (format == "-")
                return name;

            return $"{name} ({format})";
        }

        public static string FormatMatchmakingRating(double value) => value.ToString("0.###");

        public static double GetEffectiveMatchmakingRating(double mu, double sigma, double sigmaFactor = 3.0)
        {
            return mu - (sigmaFactor * sigma);
        }

        /// <summary>
        /// Resolves display class name from promotion table (same as game server table::promotion[class][promotion].name).
        /// </summary>
        public static string FormatClassName(byte classValue, byte promotion)
        {
            var cls = (Class)classValue;
            try
            {
                if (Table.Promotion.TryGetValue(cls, out var promotions)
                    && promotions.TryGetValue(promotion, out var promo)
                    && !string.IsNullOrWhiteSpace(promo?.Name))
                {
                    return promo.Name;
                }
            }
            catch
            {
                // ignore lookup errors
            }

            return FormatClass(classValue);
        }

        /// <summary>
        /// Returns level ability row from ability table (HP/MP caps for the level).
        /// </summary>
        public static Ability GetAbilityForLevel(byte classValue, byte level)
        {
            var cls = (Class)classValue;
            if (Table.Ability.TryGetValue(cls, out var classAbilities)
                && classAbilities.TryGetValue(level, out var ability))
            {
                return ability;
            }

            return null;
        }

        /// <summary>
        /// Mirrors game server character::experience_percent and exp-to-next calculations.
        /// </summary>
        public static CharacterExpInfo GetExpInfo(byte classValue, byte level, ulong currentExp)
        {
            if (IsMaxLevel(classValue, level))
            {
                return new CharacterExpInfo
                {
                    CurrentExp = currentExp,
                    IsMaxLevel = true,
                    Percent = 100f
                };
            }

            var cls = (Class)classValue;
            if (!Table.Ability.TryGetValue(cls, out var classAbilities)
                || !classAbilities.TryGetValue(level, out var ability))
            {
                return new CharacterExpInfo { CurrentExp = currentExp };
            }

            ulong prevStackedExp = 0;
            if (classAbilities.ContainsKey((byte)(level - 1)))
            {
                prevStackedExp = AbilityStackedExp.GetStackedExp(cls, (byte)(level - 1));
            }
            else if (Table.Ability.TryGetValue(Class.None, out var noneAbilities)
                     && noneAbilities.ContainsKey((byte)(level - 1)))
            {
                prevStackedExp = AbilityStackedExp.GetStackedExp(Class.None, (byte)(level - 1));
            }

            var required = ability.Exp;
            var stackedExp = AbilityStackedExp.GetStackedExp(cls, level);
            var expInLevel = currentExp > prevStackedExp ? currentExp - prevStackedExp : 0;
            var remaining = stackedExp > currentExp ? stackedExp - currentExp : 0;
            var percent = required > 0
                ? Math.Min(100f, (expInLevel / (float)required) * 100f)
                : 0f;

            return new CharacterExpInfo
            {
                CurrentExp = currentExp,
                RequiredExpForLevel = required,
                ExpInCurrentLevel = expInLevel,
                RemainingExpToNextLevel = remaining,
                Percent = percent
            };
        }

        public static bool IsMaxLevel(byte classValue, byte level)
        {
            var cls = (Class)classValue;
            if (!Table.Ability.TryGetValue(cls, out var classAbilities))
                return true;

            return !classAbilities.ContainsKey((byte)(level + 1));
        }
    }

    public sealed class CharacterExpInfo
    {
        public bool IsMaxLevel { get; init; }

        public ulong CurrentExp { get; init; }

        public ulong ExpInCurrentLevel { get; init; }

        public ulong RequiredExpForLevel { get; init; }

        public ulong RemainingExpToNextLevel { get; init; }

        public float Percent { get; init; }

        public string PercentText => $"{Percent:0.#}%";
    }
}
