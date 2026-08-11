CREATE TABLE `castle` (
  `divine_beast` tinyint unsigned NOT NULL COMMENT 'fb::model::DIVINE_BEAST enum value (0=VERMILION_BIRD,1=WHITE_TIGER,2=BLACK_TORTOISE,3=AZURE_DRAGON)',
  `owner_clan` int unsigned DEFAULT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`divine_beast`),
  KEY `castle_owner_clan_idx` (`owner_clan`),
  CONSTRAINT `fk.castle.owner_clan` FOREIGN KEY (`owner_clan`) REFERENCES `clan` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

INSERT INTO `castle` (`divine_beast`, `owner_clan`) VALUES (0, NULL), (1, NULL), (2, NULL), (3, NULL);
-- 0=VERMILION_BIRD(주작) 1=WHITE_TIGER(백호) 2=BLACK_TORTOISE(현무) 3=AZURE_DRAGON(청룡)
