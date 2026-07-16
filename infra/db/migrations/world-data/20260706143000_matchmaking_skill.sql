CREATE TABLE `matchmaking_skill` (
  `character_id` int unsigned NOT NULL,
  `match_type` int unsigned NOT NULL,
  `mu` double NOT NULL,
  `sigma` double NOT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`character_id`, `match_type`),
  KEY `matchmaking_skill_character_id_idx` (`character_id`),
  CONSTRAINT `fk.matchmaking_skill.character` FOREIGN KEY (`character_id`) REFERENCES `user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
