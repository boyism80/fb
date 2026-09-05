-- name_registry and clan_name moved to unified MySQL.
-- Keep world-global DBs free of the old registry tables.

DROP PROCEDURE IF EXISTS `USP_NAME_SET`;
DROP PROCEDURE IF EXISTS `USP_NAME_GET_ID`;
DROP PROCEDURE IF EXISTS `USP_CLAN_NAME_DELETE`;
DROP TABLE IF EXISTS `name_registry`;
DROP TABLE IF EXISTS `clan_name`;
