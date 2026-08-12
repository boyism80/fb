CREATE TABLE `friend` (
  `user` int unsigned NOT NULL,
  `friend_uid` int unsigned NOT NULL,
  `friend_name` varchar(16) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `mutual` tinyint NOT NULL DEFAULT '0',
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`user`,`friend_uid`),
  KEY `idx_friend_friend_uid` (`friend_uid`),
  CONSTRAINT `fk.friend.user` FOREIGN KEY (`user`) REFERENCES `user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
