-- system_mail and system_storage_box moved to world-global scope.
-- They are accessed via GetGlobalConnection, so remove them from world-data shards.

DROP TABLE IF EXISTS `system_storage_box`;
DROP TABLE IF EXISTS `system_mail`;
