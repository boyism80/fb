-- MySQL dump 10.13  Distrib 8.0.16, for Win64 (x86_64)
--
-- Host: localhost    Database: fb
-- ------------------------------------------------------
-- Server version	8.0.16

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
 SET NAMES utf8 ;
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
-- Table structure for table `clan`
--

DROP TABLE IF EXISTS `clan`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `clan` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `owner` int(10) unsigned NOT NULL,
  `name` varchar(45) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `title` varchar(45) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `clan_owner_idx` (`owner`),
  CONSTRAINT `clan_owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item`
--

DROP TABLE IF EXISTS `item`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `item` (
  `owner` int(10) unsigned NOT NULL,
  `index` smallint(6) NOT NULL,
  `slot` smallint(6) NOT NULL,
  `master` int(10) unsigned DEFAULT NULL,
  `count` smallint(5) unsigned DEFAULT '1',
  `durability` smallint(5) unsigned DEFAULT NULL,
  PRIMARY KEY (`owner`,`index`,`slot`),
  KEY `item_owner_idx` (`owner`),
  CONSTRAINT `item_owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `legend`
--

DROP TABLE IF EXISTS `legend`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `legend` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `owner` int(10) unsigned NOT NULL,
  `look` tinyint(4) NOT NULL DEFAULT '0',
  `color` tinyint(4) NOT NULL DEFAULT '0',
  `content` varchar(45) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `legend_owner_idx` (`owner`),
  CONSTRAINT `legend_owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `spell`
--

DROP TABLE IF EXISTS `spell`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `spell` (
  `owner` int(10) unsigned NOT NULL,
  `slot` tinyint(4) NOT NULL,
  `id` int(11) DEFAULT NULL,
  PRIMARY KEY (`owner`,`slot`),
  KEY `spell_owner_idx` (`owner`),
  CONSTRAINT `spell_owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `user`
--

DROP TABLE IF EXISTS `user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `user` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(256) NOT NULL,
  `pw` varchar(256) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `birth` int(10) unsigned DEFAULT NULL,
  `datetime` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_login` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `admin` tinyint(4) NOT NULL DEFAULT '0',
  `look` smallint(5) unsigned NOT NULL DEFAULT '0',
  `color` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `sex` tinyint(1) NOT NULL DEFAULT '0',
  `nation` smallint(6) NOT NULL DEFAULT '0',
  `creature` smallint(6) DEFAULT NULL,
  `map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `position_x` smallint(5) unsigned NOT NULL DEFAULT '0',
  `position_y` smallint(5) unsigned NOT NULL DEFAULT '0',
  `direction` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `state` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `promotion` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `exp` int(10) unsigned NOT NULL DEFAULT '0',
  `money` int(10) unsigned NOT NULL DEFAULT '0',
  `disguise` smallint(5) unsigned DEFAULT NULL,
  `hp` int(10) unsigned NOT NULL,
  `base_hp` int(10) unsigned NOT NULL,
  `additional_hp` int(10) unsigned NOT NULL DEFAULT '0',
  `mp` int(10) unsigned NOT NULL,
  `base_mp` int(10) unsigned NOT NULL,
  `additional_mp` int(10) unsigned NOT NULL DEFAULT '0',
  `weapon_color` tinyint(3) unsigned DEFAULT NULL,
  `helmet_color` tinyint(3) unsigned DEFAULT NULL,
  `armor_color` tinyint(3) unsigned DEFAULT NULL,
  `shield_color` tinyint(3) unsigned DEFAULT NULL,
  `ring_left_color` tinyint(3) unsigned DEFAULT NULL,
  `ring_right_color` tinyint(3) unsigned DEFAULT NULL,
  `aux_top_color` int(11) DEFAULT NULL,
  `aux_bot_color` int(10) unsigned DEFAULT NULL,
  `clan` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  KEY `user_clan_idx` (`clan`),
  CONSTRAINT `user_clan` FOREIGN KEY (`clan`) REFERENCES `clan` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=103 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2023-08-04 13:37:34
