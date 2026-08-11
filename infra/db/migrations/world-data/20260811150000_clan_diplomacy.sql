CREATE TABLE `clan_alliance` (
  `clan` int unsigned NOT NULL,
  `allied_clan` int unsigned NOT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`clan`),
  KEY `clan_alliance_allied_clan_idx` (`allied_clan`),
  CONSTRAINT `fk.clan_alliance.clan` FOREIGN KEY (`clan`) REFERENCES `clan` (`id`) ON DELETE CASCADE,
  CONSTRAINT `fk.clan_alliance.allied_clan` FOREIGN KEY (`allied_clan`) REFERENCES `clan` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

CREATE TABLE `clan_enemy` (
  `clan` int unsigned NOT NULL,
  `enemy_clan` int unsigned NOT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`clan`,`enemy_clan`),
  KEY `clan_enemy_enemy_clan_idx` (`enemy_clan`),
  CONSTRAINT `fk.clan_enemy.clan` FOREIGN KEY (`clan`) REFERENCES `clan` (`id`) ON DELETE CASCADE,
  CONSTRAINT `fk.clan_enemy.enemy_clan` FOREIGN KEY (`enemy_clan`) REFERENCES `clan` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
