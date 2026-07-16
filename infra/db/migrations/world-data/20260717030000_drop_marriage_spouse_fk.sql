-- spouse_id may reference a character on another data shard.
-- Per-shard user FK cannot enforce cross-shard marriages.

ALTER TABLE `marriage` DROP FOREIGN KEY `fk.marriage.spouse`;
