-- MySQL dump 10.13  Distrib 8.0.41, for Win64 (x86_64)
--
-- Host: 127.0.0.1    Database: fb
-- ------------------------------------------------------
-- Server version	8.0.41

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `fb`
--

/*!40000 DROP DATABASE IF EXISTS `fb`*/;

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `fb` /*!40100 DEFAULT CHARACTER SET euckr */ /*!80016 DEFAULT ENCRYPTION='N' */;

USE `fb`;

--
-- Table structure for table `achievement`
--

DROP TABLE IF EXISTS `achievement`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `achievement` (
  `uid` int unsigned NOT NULL,
  `model` int unsigned NOT NULL,
  `text` varchar(128) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  `icon` smallint DEFAULT NULL,
  `color` smallint DEFAULT NULL,
  `deleted` tinyint DEFAULT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`uid`,`model`),
  KEY `fk.legend.owner_idx` (`model`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bulletin`
--

DROP TABLE IF EXISTS `bulletin`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `bulletin` (
  `id` int unsigned NOT NULL,
  `section` int unsigned NOT NULL,
  `user` int unsigned NOT NULL,
  `title` varchar(64) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `contents` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`,`section`),
  KEY `fk.bulletin.owner_idx` (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bulletin_sequence`
--

DROP TABLE IF EXISTS `bulletin_sequence`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `bulletin_sequence` (
  `section` int unsigned NOT NULL,
  `id` int unsigned NOT NULL,
  PRIMARY KEY (`section`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_sync`
--

DROP TABLE IF EXISTS `character_sync`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_sync` (
  `uid` int NOT NULL,
  `group` int DEFAULT NULL,
  `clan` int DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `clan`
--

DROP TABLE IF EXISTS `clan`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `clan` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL,
  `title` varchar(45) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `clan_member`
--

DROP TABLE IF EXISTS `clan_member`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `clan_member` (
  `clan` int unsigned NOT NULL,
  `user` int NOT NULL,
  `role` int NOT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`clan`,`user`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `clan_name`
--

DROP TABLE IF EXISTS `clan_name`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `clan_name` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(256) NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name_UNIQUE` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `group`
--

DROP TABLE IF EXISTS `group`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `group` (
  `master` int unsigned NOT NULL,
  `members` varchar(256) NOT NULL,
  `deleted` tinyint NOT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`master`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item`
--

DROP TABLE IF EXISTS `item`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item` (
  `owner` int unsigned NOT NULL,
  `index` smallint NOT NULL DEFAULT '-1',
  `parts` smallint NOT NULL DEFAULT '0',
  `stored` smallint NOT NULL DEFAULT '-1',
  `model` int unsigned NOT NULL,
  `count` smallint unsigned DEFAULT '1',
  `durability` int unsigned DEFAULT NULL,
  `custom_name` varchar(32) DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`owner`,`index`,`parts`,`stored`),
  KEY `item_owner_idx` (`owner`),
  CONSTRAINT `fk.item.owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mail`
--

DROP TABLE IF EXISTS `mail`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `mail` (
  `id` int unsigned NOT NULL,
  `user` int unsigned NOT NULL,
  `sender` int unsigned NOT NULL,
  `title` varchar(64) NOT NULL,
  `contents` varchar(256) NOT NULL,
  `read` tinyint NOT NULL DEFAULT '0',
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`,`user`),
  KEY `IX_UNAME` (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mail_sequence`
--

DROP TABLE IF EXISTS `mail_sequence`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `mail_sequence` (
  `user` int NOT NULL,
  `id` int NOT NULL,
  PRIMARY KEY (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `name`
--

DROP TABLE IF EXISTS `name`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `name` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ban`
--

DROP TABLE IF EXISTS `ban`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `ban` (
  `user` int unsigned NOT NULL,
  `reason` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `expire_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user`),
  KEY `fk.ban.user_idx` (`user`),
  CONSTRAINT `fk.ban.user` FOREIGN KEY (`user`) REFERENCES `name` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `option`
--

DROP TABLE IF EXISTS `option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `option` (
  `uid` int unsigned NOT NULL,
  `whisper` tinyint unsigned NOT NULL,
  `group` tinyint unsigned NOT NULL,
  `roar` tinyint unsigned NOT NULL,
  `roar_worlds` tinyint unsigned NOT NULL,
  `magic_effect` tinyint unsigned NOT NULL,
  `weather_effect` tinyint unsigned NOT NULL,
  `fixed_move` tinyint unsigned NOT NULL,
  `trade` tinyint unsigned NOT NULL,
  `fast_move` tinyint unsigned NOT NULL,
  `effect_sound` tinyint unsigned NOT NULL,
  `pk_protect` tinyint unsigned NOT NULL,
  `deleted` tinyint unsigned NOT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`uid`),
  UNIQUE KEY `uid_UNIQUE` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `quest`
--

DROP TABLE IF EXISTS `quest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `quest` (
  `user` int NOT NULL,
  `id` int NOT NULL,
  `step` int NOT NULL,
  `progress` int NOT NULL,
  `param` varchar(64) DEFAULT NULL,
  `completed` tinyint NOT NULL,
  `deleted` tinyint NOT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`user`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `spell`
--

DROP TABLE IF EXISTS `spell`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `spell` (
  `owner` int unsigned NOT NULL,
  `slot` tinyint NOT NULL,
  `model` int NOT NULL,
  `next` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`owner`,`slot`),
  KEY `spell_owner_idx` (`owner`),
  CONSTRAINT `fk.spell.owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `system_mail`
--

DROP TABLE IF EXISTS `system_mail`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `system_mail` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `sender` int unsigned NOT NULL,
  `title` varchar(64) NOT NULL,
  `contents` varchar(256) NOT NULL,
  `expire_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=34 DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `system_mail_user`
--

DROP TABLE IF EXISTS `system_mail_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `system_mail_user` (
  `mail_id` int unsigned NOT NULL,
  `user` int unsigned NOT NULL,
  `read` tinyint NOT NULL DEFAULT '0',
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`mail_id`,`user`),
  KEY `IX_USER` (`user`),
  KEY `IX_MAIL_ID` (`mail_id`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `storage_box`
--

DROP TABLE IF EXISTS `storage_box`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `storage_box` (
  `user` int unsigned NOT NULL,
  `id` int unsigned NOT NULL,
  `message` varchar(256) NOT NULL,
  `attachments` json NOT NULL,
  `received` tinyint NOT NULL DEFAULT '0',
  `expired_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user`,`id`),
  KEY `idx_storage_box_expired` (`user`,`expired_date`),
  KEY `idx_storage_box_received` (`user`,`received`,`deleted`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `storage_pending_box`
--

DROP TABLE IF EXISTS `storage_pending_box`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `storage_pending_box` (
  `id` bigint unsigned NOT NULL,
  `user` int unsigned DEFAULT NULL,
  `message` varchar(256) NOT NULL,
  `attachments` json NOT NULL,
  `expired_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_storage_pending_user` (`user`,`deleted`,`expired_date`),
  KEY `idx_storage_pending_expired` (`deleted`,`expired_date`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `storage_pending_sequence`
--

DROP TABLE IF EXISTS `storage_pending_sequence`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `storage_pending_sequence` (
  `id` bigint unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `storage_reward_mark`
--

DROP TABLE IF EXISTS `storage_reward_mark`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `storage_reward_mark` (
  `user` int unsigned NOT NULL,
  `pending_id` bigint unsigned NOT NULL,
  `expired_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`pending_id`),
  KEY `idx_storage_reward_mark_user` (`user`,`deleted`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `user`
--

DROP TABLE IF EXISTS `user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user` (
  `id` int unsigned NOT NULL,
  `name` varchar(256) NOT NULL,
  `pw` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `birth` int unsigned DEFAULT NULL,
  `role` smallint unsigned NOT NULL DEFAULT '0',
  `look` smallint unsigned NOT NULL DEFAULT '0',
  `color` tinyint unsigned NOT NULL DEFAULT '0',
  `sex` tinyint unsigned NOT NULL DEFAULT '0',
  `nation` smallint unsigned NOT NULL DEFAULT '0',
  `creature` smallint unsigned DEFAULT NULL,
  `map` smallint unsigned NOT NULL DEFAULT '0',
  `position_x` smallint unsigned NOT NULL DEFAULT '0',
  `position_y` smallint unsigned NOT NULL DEFAULT '0',
  `direction` tinyint unsigned NOT NULL DEFAULT '0',
  `state` tinyint unsigned NOT NULL DEFAULT '0',
  `class` tinyint unsigned NOT NULL DEFAULT '0',
  `promotion` tinyint unsigned NOT NULL DEFAULT '0',
  `level` int unsigned NOT NULL DEFAULT '1',
  `exp` int unsigned NOT NULL DEFAULT '0',
  `money` int unsigned NOT NULL DEFAULT '0',
  `deposited_money` int unsigned NOT NULL DEFAULT '0',
  `disguise` smallint unsigned DEFAULT NULL,
  `hp` int unsigned NOT NULL,
  `base_hp` int unsigned NOT NULL,
  `additional_hp` int unsigned NOT NULL DEFAULT '0',
  `mp` int unsigned NOT NULL,
  `base_mp` int unsigned NOT NULL,
  `additional_mp` int unsigned NOT NULL DEFAULT '0',
  `weapon_color` tinyint unsigned DEFAULT NULL,
  `helmet_color` tinyint unsigned DEFAULT NULL,
  `armor_color` tinyint unsigned DEFAULT NULL,
  `shield_color` tinyint unsigned DEFAULT NULL,
  `ring_left_color` tinyint unsigned DEFAULT NULL,
  `ring_right_color` tinyint unsigned DEFAULT NULL,
  `aux_top_color` int unsigned DEFAULT NULL,
  `aux_bot_color` int unsigned DEFAULT NULL,
  `buffs` varchar(512) NOT NULL DEFAULT '[]',
  `title` varchar(32) NOT NULL DEFAULT '',
  `deleted` tinyint unsigned NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  UNIQUE KEY `name_UNIQUE` (`name`),
  KEY `INDEX_NAME` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping routines for database 'fb'
--
/*!50003 DROP PROCEDURE IF EXISTS `USP_BULLETIN_ADD` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BULLETIN_ADD`(section INT, uid INT, title NVARCHAR(64), contents NVARCHAR(256))
BEGIN
    DECLARE new_id INT UNSIGNED;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT;
    END;

    START TRANSACTION;

    SELECT id INTO new_id 
    FROM bulletin_sequence 
    WHERE bulletin_sequence.`section` = section FOR UPDATE;

    IF new_id IS NULL THEN
        SET new_id = 1;
        INSERT INTO bulletin_sequence (`section`, `id`) VALUES (section, new_id);
    ELSE
        SET new_id = new_id + 1;
        UPDATE bulletin_sequence SET id = new_id WHERE bulletin_sequence.`section` = section;
    END IF;

    INSERT INTO bulletin (`id`, `section`, `user`, `title`, `contents`)
    VALUES (new_id, section, uid, title, contents);

    COMMIT;

    SELECT 1 AS RESULT;
    SELECT * FROM bulletin WHERE bulletin.`id` = new_id AND bulletin.`section` = section;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_BULLETIN_DELETE` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BULLETIN_DELETE`(IN id INT, IN user INT, IN ignore_owner TINYINT)
BEGIN
    DECLARE _deleted TINYINT;
    DECLARE _owner INT;

    SELECT bulletin.deleted, bulletin.user
    INTO _deleted, _owner
    FROM bulletin
    WHERE bulletin.id = id
    LIMIT 1;

    IF _deleted IS NULL THEN
        SELECT -1 AS result;
    ELSEIF _deleted = 1 THEN
        SELECT -2 AS result;
    ELSEIF _owner != user AND ignore_owner = 0 THEN
        SELECT -3 AS result;
    ELSE
        IF ignore_owner = 1 THEN
            UPDATE bulletin SET deleted = 1, updated_date = NOW() WHERE bulletin.id = id;
        ELSE
            UPDATE bulletin SET deleted = 1, updated_date = NOW() WHERE bulletin.id = id AND bulletin.user = user;
        END IF;
        SELECT 1 AS result;
    END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_BULLETIN_UPDATE` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BULLETIN_UPDATE`(
    IN p_id INT,
    IN p_user INT,
    IN p_title NVARCHAR(64),
    IN p_contents NVARCHAR(256),
    IN p_ignore_owner TINYINT
)
BEGIN
    DECLARE _deleted TINYINT;
    DECLARE _owner INT;

    SELECT bulletin.deleted, bulletin.user
    INTO _deleted, _owner
    FROM bulletin
    WHERE bulletin.id = p_id
    LIMIT 1;

    IF _deleted IS NULL THEN
        SELECT -1 AS result;
    ELSEIF _deleted = 1 THEN
        SELECT -2 AS result;
    ELSEIF _owner != p_user AND p_ignore_owner = 0 THEN
        SELECT -3 AS result;
    ELSE
        IF p_ignore_owner = 1 THEN
            UPDATE bulletin
            SET title = p_title,
                contents = p_contents,
                updated_date = NOW()
            WHERE bulletin.id = p_id;
        ELSE
            UPDATE bulletin
            SET title = p_title,
                contents = p_contents,
                updated_date = NOW()
            WHERE bulletin.id = p_id AND bulletin.user = p_user;
        END IF;

        IF ROW_COUNT() = 0 THEN
            SELECT -4 AS result;
        ELSE
            SELECT 1 AS result;
        END IF;
    END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_BULLETIN_GET` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BULLETIN_GET`(IN section INT, IN article INT)
BEGIN
    SELECT A.`id`,
           A.`user`,
           A.`title`,
           A.`contents`,
           A.`created_date` 
    FROM `bulletin` AS A
    WHERE A.`id` = article AND A.`section` = section AND `deleted` = 0;
    
    SELECT EXISTS(SELECT * FROM `bulletin`
                  WHERE `bulletin`.`section` = section 
                    AND `bulletin`.`id` > article 
                    AND `bulletin`.`deleted` = 0 
                  LIMIT 1) as `next`;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_BULLETIN_GET_LIST` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BULLETIN_GET_LIST`(IN section INT, IN position INT)
BEGIN
    SELECT A.`id`,
           A.`section`,
           A.`user`,
           A.`title`,
           A.`created_date`
    FROM bulletin AS A
    WHERE A.`section` = section AND A.`deleted` = 0 AND position >= A.`id`
    ORDER BY A.`id` DESC
    LIMIT 0, 20;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_CLAN_NAME_DELETE` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_CLAN_NAME_DELETE`(id INT)
BEGIN
	DELETE FROM `clan_name` WHERE `clan_name`.`id` = id;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_MAIL_GET_LIST` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_MAIL_GET_LIST`(IN user INT, IN position INT, IN count INT)
BEGIN
    SELECT mail.`id`,
           mail.`user`,
           mail.`sender`,
           mail.`title`,
           mail.`read`,
           mail.`created_date`
    FROM mail
    WHERE mail.`user` = user AND mail.`deleted` = 0 AND position >= mail.`id`
    ORDER BY mail.`id` DESC
    LIMIT 0, count;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_MAIL_READ` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_MAIL_READ`(IN user INT, IN id INT)
BEGIN
	  START TRANSACTION;
      
      UPDATE mail
	  SET `read` = 1
	  WHERE mail.`id` = id AND 
			mail.`user` = user AND 
			mail.`deleted` = 0;

	SELECT * FROM `mail`
    WHERE mail.`id` = id AND
          mail.`user` = user AND
          mail.`deleted` = 0;
          
	COMMIT;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_MAIL_WRITE` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_MAIL_WRITE`(
    IN user INT, 
    IN sender INT, 
    IN title NVARCHAR(64), 
    IN contents NVARCHAR(256)
)
BEGIN
    DECLARE new_id INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT;
    END;

    START TRANSACTION;

    SELECT id INTO new_id 
    FROM mail_sequence 
    WHERE mail_sequence.`user` = user FOR UPDATE;

    IF new_id IS NULL THEN
        SET new_id = 1;
        INSERT INTO mail_sequence (`user`, `id`) VALUES (user, new_id);
    ELSE
        SET new_id = new_id + 1;
        UPDATE mail_sequence SET id = new_id WHERE mail_sequence.`user` = user;
    END IF;

    INSERT INTO mail (`id`, `user`, `sender`, `title`, `contents`)
    VALUES (new_id, user, sender, title, contents);

    COMMIT;

    SELECT 1 AS RESULT;
    SELECT * FROM mail WHERE mail.`id` = new_id AND mail.`user` = user;

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_STORAGE_PENDING_ADD` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_STORAGE_PENDING_ADD`(
    IN p_id BIGINT UNSIGNED,
    IN p_user INT UNSIGNED,
    IN p_message NVARCHAR(256),
    IN p_attachments JSON,
    IN p_expired_date DATETIME
)
BEGIN
    DECLARE v_error_code INT DEFAULT 0;
    DECLARE v_error_message VARCHAR(255) DEFAULT '';

    DECLARE CONTINUE HANDLER FOR SQLEXCEPTION
    BEGIN
        GET DIAGNOSTICS CONDITION 1
            v_error_code = MYSQL_ERRNO,
            v_error_message = MESSAGE_TEXT;
        ROLLBACK;
        SELECT 0 AS RESULT, NULL AS id, v_error_code AS error_code, v_error_message AS error_message;
    END;

    START TRANSACTION;

    INSERT INTO storage_pending_box (`id`, `user`, `message`, `attachments`, `expired_date`)
    VALUES (p_id, p_user, p_message, p_attachments, p_expired_date);

    IF v_error_code != 0 THEN
        SELECT 0 AS RESULT, NULL AS id, v_error_code AS error_code, v_error_message AS error_message;
    ELSE
        COMMIT;
        SELECT 1 AS RESULT, p_id AS id;
        SELECT * FROM storage_pending_box WHERE storage_pending_box.`id` = p_id;
    END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_STORAGE_PENDING_GET_NEXT_ID` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_STORAGE_PENDING_GET_NEXT_ID`()
BEGIN
    DECLARE new_id BIGINT UNSIGNED;
    DECLARE v_error_code INT DEFAULT 0;
    DECLARE v_error_message VARCHAR(255) DEFAULT '';

    DECLARE CONTINUE HANDLER FOR SQLEXCEPTION
    BEGIN
        GET DIAGNOSTICS CONDITION 1
            v_error_code = MYSQL_ERRNO,
            v_error_message = MESSAGE_TEXT;
        ROLLBACK;
        SELECT 0 AS RESULT, NULL AS id, v_error_code AS error_code, v_error_message AS error_message;
    END;

    START TRANSACTION;

    -- Get current sequence value (single global sequence)
    SELECT id INTO new_id 
    FROM storage_pending_sequence 
    LIMIT 1 FOR UPDATE;

    IF new_id IS NULL THEN
        -- Initialize sequence: get max id from entire table (id is globally unique), or 1 if no records exist
        SELECT COALESCE(MAX(id), 0) + 1 INTO new_id
        FROM storage_pending_box;
        
        -- Ensure new_id is at least 1
        IF new_id IS NULL OR new_id = 0 THEN
            SET new_id = 1;
        END IF;
        
        INSERT INTO storage_pending_sequence (`id`) VALUES (new_id);
    ELSE
        SET new_id = new_id + 1;
        UPDATE storage_pending_sequence SET id = new_id;
    END IF;

    IF v_error_code != 0 THEN
        SELECT 0 AS RESULT, NULL AS id, v_error_code AS error_code, v_error_message AS error_message;
    ELSE
        COMMIT;
        SELECT 1 AS RESULT, new_id AS id;
    END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_NAME_GET_ID` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_NAME_GET_ID`(n NVARCHAR(256))
BEGIN
	SELECT id FROM name WHERE name.name = n;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_NAME_SET` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`localhost` PROCEDURE `USP_NAME_SET`(IN uname NVARCHAR(256))
BEGIN
    DECLARE uid INT;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
		ROLLBACK;
        SELECT 0 AS result, 0;
    END;

	SET TRANSACTION ISOLATION LEVEL REPEATABLE READ;
    START TRANSACTION;
    
    SELECT id INTO uid FROM name WHERE name = uname FOR UPDATE;
    
    IF uid IS NULL THEN
        INSERT INTO name (name) VALUES (uname);
        SET uid = LAST_INSERT_ID();
        SELECT 1 AS result, uid;
    ELSE
        SELECT 0 AS result, uid;
    END IF;

    COMMIT;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2025-11-23 17:53:02
