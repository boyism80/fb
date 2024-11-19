-- MySQL dump 10.13  Distrib 8.0.38, for Win64 (x86_64)
--
-- Host: localhost    Database: fb
-- ------------------------------------------------------
-- Server version	8.4.3

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
-- Table structure for table `board`
--

DROP TABLE IF EXISTS `board`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `board` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `section` int unsigned NOT NULL,
  `user` int unsigned NOT NULL,
  `title` varchar(64) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `contents` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `fk.board.owner_idx` (`user`),
  CONSTRAINT `fk.board.user` FOREIGN KEY (`user`) REFERENCES `name` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=308 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `clan`
--

DROP TABLE IF EXISTS `clan`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `clan` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `owner` int unsigned NOT NULL,
  `name` varchar(45) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `title` varchar(45) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk.clan.owner_idx` (`owner`),
  CONSTRAINT `fk.clan.owner` FOREIGN KEY (`owner`) REFERENCES `name` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
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
  `deposited` smallint NOT NULL DEFAULT '-1',
  `model` int unsigned NOT NULL,
  `count` smallint unsigned DEFAULT '1',
  `durability` int unsigned DEFAULT NULL,
  `custom_name` varchar(32) DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`owner`,`index`,`parts`,`deposited`),
  KEY `item_owner_idx` (`owner`),
  CONSTRAINT `fk.item.owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `legend`
--

DROP TABLE IF EXISTS `legend`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `legend` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `owner` int unsigned NOT NULL,
  `look` tinyint NOT NULL DEFAULT '0',
  `color` tinyint NOT NULL DEFAULT '0',
  `content` varchar(45) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk.legend.owner_idx` (`owner`),
  CONSTRAINT `fk.legend.owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
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
) ENGINE=InnoDB AUTO_INCREMENT=53 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
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
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`owner`,`slot`),
  KEY `spell_owner_idx` (`owner`),
  CONSTRAINT `fk.spell.owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
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
  `admin` tinyint NOT NULL DEFAULT '0',
  `look` smallint unsigned NOT NULL DEFAULT '0',
  `color` tinyint unsigned NOT NULL DEFAULT '0',
  `sex` tinyint(1) NOT NULL DEFAULT '0',
  `nation` smallint NOT NULL DEFAULT '0',
  `creature` smallint DEFAULT NULL,
  `map` smallint unsigned NOT NULL DEFAULT '0',
  `position_x` smallint unsigned NOT NULL DEFAULT '0',
  `position_y` smallint unsigned NOT NULL DEFAULT '0',
  `direction` tinyint unsigned NOT NULL DEFAULT '0',
  `state` tinyint unsigned NOT NULL DEFAULT '0',
  `class` tinyint unsigned NOT NULL DEFAULT '0',
  `promotion` tinyint unsigned NOT NULL DEFAULT '0',
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
  `aux_top_color` int DEFAULT NULL,
  `aux_bot_color` int unsigned DEFAULT NULL,
  `clan` int unsigned DEFAULT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  KEY `user_clan_idx` (`clan`),
  CONSTRAINT `user_clan` FOREIGN KEY (`clan`) REFERENCES `clan` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping routines for database 'fb'
--
/*!50003 DROP PROCEDURE IF EXISTS `USP_BOARD_ADD` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BOARD_ADD`(section INT, uid INT, title NVARCHAR(64), contents NVARCHAR(256))
BEGIN
	INSERT INTO board (`section`, `user`, `title`, `contents`)
    VALUES (section, uid, title, contents);
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_BOARD_DELETE` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BOARD_DELETE`(id INT, user INT)
BEGIN
	DECLARE _id INT;
    DECLARE _user INT;
    DECLARE _deleted TINYINT;
    
    SELECT id, user, deleted INTO _id, _user, _deleted FROM board WHERE board.id = id LIMIT 1;
    IF _id IS NULL THEN
		SELECT -1 AS result;
	ELSEIF _deleted = 1 THEN
		SELECT -2 AS result;
	ELSEIF _user != user THEN
		SELECT -3 AS result;
	ELSE
		UPDATE board SET deleted = 1 WHERE board.id = id;
		SELECT 1 AS result;
    END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_BOARD_GET` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BOARD_GET`(section INT, article INT)
BEGIN
	SELECT A.id, N.id AS uid, N.name AS uname, A.title, A.contents, A.created_date FROM board AS A
	LEFT JOIN name AS N ON A.user = N.id
	WHERE A.id = article AND A.section = section AND deleted = 0;
    
    SELECT EXISTS(SELECT * FROM board WHERE board.section = section AND board.id > article AND board.deleted = 0 LIMIT 1) as `next`;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `USP_BOARD_GET_LIST` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_BOARD_GET_LIST`(section INT, position INT)
BEGIN
	SELECT title FROM board WHERE id = section;

	SELECT A.id, N.id AS uid, N.name AS uname, A.title, A.created_date FROM board AS A
	LEFT JOIN name AS N
	ON A.user = N.id
    WHERE A.section = section AND A.deleted = 0 AND position >= A.id
    ORDER BY A.id DESC
	LIMIT 0, 20;
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
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`fb`@`%` PROCEDURE `USP_NAME_SET`(name NVARCHAR(256))
BEGIN
	DECLARE exist TINYINT;
    SELECT EXISTS(SELECT * FROM name WHERE name.name = name) INTO exist;
    
    IF exist = 0 THEN
		INSERT INTO name (name) VALUES (name);
        SELECT 1 AS result, last_insert_id() as uid;
	ELSE
		SELECT 0 AS result, 0 as uid;
    END IF;
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

-- Dump completed on 2024-11-19 23:58:49
