-- allied_clan / enemy_clan may reference a clan on another data shard.
-- Per-shard clan FK cannot enforce cross-shard diplomacy; disband clears both sides explicitly.

ALTER TABLE `clan_alliance` DROP FOREIGN KEY `fk.clan_alliance.allied_clan`;

ALTER TABLE `clan_enemy` DROP FOREIGN KEY `fk.clan_enemy.enemy_clan`;
