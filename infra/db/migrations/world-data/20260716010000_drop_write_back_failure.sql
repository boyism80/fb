-- write_back_failure moved to world-global scope.
-- It is accessed via GetGlobalConnection, so remove it from world-data shards.

DROP TABLE IF EXISTS `write_back_failure`;
