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
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item`
--

DROP TABLE IF EXISTS `item`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `item` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `master` int(10) unsigned NOT NULL,
  `owner` int(10) unsigned DEFAULT NULL,
  `slot` smallint(5) DEFAULT NULL,
  `count` smallint(5) unsigned NOT NULL DEFAULT '1',
  `durability` smallint(5) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `item_owner_idx` (`owner`),
  CONSTRAINT `item_owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=33 DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `legend`
--

DROP TABLE IF EXISTS `legend`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `legend` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `owner` int(10) unsigned NOT NULL,
  `look` tinyint(4) NOT NULL DEFAULT '0',
  `color` tinyint(4) NOT NULL DEFAULT '0',
  `content` varchar(45) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `legend_owner_idx` (`owner`),
  CONSTRAINT `legend_owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `user`
--

DROP TABLE IF EXISTS `user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `user` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(45) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `pw` varchar(256) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `birth` int(11) unsigned DEFAULT NULL,
  `datetime` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
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
  `weapon` int(10) unsigned DEFAULT NULL,
  `weapon_color` tinyint(3) unsigned DEFAULT NULL,
  `helmet` int(10) unsigned DEFAULT NULL,
  `helmet_color` tinyint(3) unsigned DEFAULT NULL,
  `armor` int(10) unsigned DEFAULT NULL,
  `armor_color` tinyint(3) unsigned DEFAULT NULL,
  `shield` int(10) unsigned DEFAULT NULL,
  `shield_color` tinyint(3) unsigned DEFAULT NULL,
  `ring_left` int(10) unsigned DEFAULT NULL,
  `ring_left_color` tinyint(3) unsigned DEFAULT NULL,
  `ring_right` int(10) unsigned DEFAULT NULL,
  `ring_right_color` tinyint(3) unsigned DEFAULT NULL,
  `aux_top` int(10) unsigned DEFAULT NULL,
  `aux_top_color` int(11) DEFAULT NULL,
  `aux_bot` int(10) unsigned DEFAULT NULL,
  `aux_bot_color` int(10) unsigned DEFAULT NULL,
  `clan` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `index_idx` (`weapon`,`helmet`,`armor`,`shield`,`ring_left`,`ring_right`,`aux_top`,`aux_bot`),
  KEY `helmet_idx` (`helmet`),
  KEY `armor_idx` (`armor`),
  KEY `shield_idx` (`shield`),
  KEY `ring_left_idx` (`ring_left`),
  KEY `ring_right_idx` (`ring_right`),
  KEY `aux_top_idx` (`aux_top`),
  KEY `aux_bot_idx` (`aux_bot`),
  KEY `user_clan_idx` (`clan`),
  CONSTRAINT `user_armor` FOREIGN KEY (`armor`) REFERENCES `item` (`id`),
  CONSTRAINT `user_aux_bot` FOREIGN KEY (`aux_bot`) REFERENCES `item` (`id`),
  CONSTRAINT `user_aux_top` FOREIGN KEY (`aux_top`) REFERENCES `item` (`id`),
  CONSTRAINT `user_clan` FOREIGN KEY (`clan`) REFERENCES `clan` (`id`),
  CONSTRAINT `user_helmet` FOREIGN KEY (`helmet`) REFERENCES `item` (`id`),
  CONSTRAINT `user_ring_left` FOREIGN KEY (`ring_left`) REFERENCES `item` (`id`),
  CONSTRAINT `user_ring_right` FOREIGN KEY (`ring_right`) REFERENCES `item` (`id`),
  CONSTRAINT `user_shield` FOREIGN KEY (`shield`) REFERENCES `item` (`id`),
  CONSTRAINT `user_weapon` FOREIGN KEY (`weapon`) REFERENCES `item` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=101 DEFAULT CHARSET=euckr;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2020-10-28 20:57:08
