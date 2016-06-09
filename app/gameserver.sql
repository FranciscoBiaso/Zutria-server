-- phpMyAdmin SQL Dump
-- version 4.4.14
-- http://www.phpmyadmin.net
--
-- Host: 127.0.0.1
-- Generation Time: 08-Maio-2016 às 01:15
-- Versão do servidor: 5.6.26
-- PHP Version: 5.6.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `avesta74`
--

-- --------------------------------------------------------

--
-- Estrutura da tabela `accounts`
--

CREATE TABLE IF NOT EXISTS `accounts` (
  `id` int(10) unsigned NOT NULL,
  `password` varchar(255) NOT NULL,
  `email` varchar(255) NOT NULL DEFAULT '',
  `premend` int(10) unsigned NOT NULL DEFAULT '0',
  `blocked` tinyint(1) NOT NULL DEFAULT '0',
  `deleted` tinyint(1) NOT NULL DEFAULT '0',
  `warned` tinyint(1) NOT NULL DEFAULT '0',
  `warnings` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

--
-- Extraindo dados da tabela `accounts`
--

INSERT INTO `accounts` (`id`, `password`, `email`, `premend`, `blocked`, `deleted`, `warned`, `warnings`) VALUES
(1, '1', '', 9999, 0, 0, 0, 0);

--
-- Acionadores `accounts`
--
DELIMITER $$
CREATE TRIGGER `ondelete_accounts` BEFORE DELETE ON `accounts`
 FOR EACH ROW BEGIN
    DELETE FROM `bans` WHERE `type` = 3 AND `value` = OLD.`id`;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Estrutura da tabela `bans`
--

CREATE TABLE IF NOT EXISTS `bans` (
  `id` int(10) unsigned NOT NULL,
  `type` int(11) NOT NULL COMMENT 'this field defines if its ip, account, player, or any else ban',
  `value` int(10) unsigned NOT NULL COMMENT 'ip, player guid, account number',
  `param` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'mask',
  `active` tinyint(1) NOT NULL DEFAULT '1',
  `expires` int(10) unsigned NOT NULL,
  `added` int(10) unsigned NOT NULL,
  `admin_id` int(10) unsigned NOT NULL DEFAULT '0',
  `reason` varchar(255) NOT NULL DEFAULT '',
  `comment` varchar(255) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `global_storage`
--

CREATE TABLE IF NOT EXISTS `global_storage` (
  `key` int(10) unsigned NOT NULL,
  `value` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `groups`
--

CREATE TABLE IF NOT EXISTS `groups` (
  `id` int(10) unsigned NOT NULL,
  `name` varchar(255) NOT NULL COMMENT 'group name',
  `flags` bigint(20) unsigned NOT NULL DEFAULT '0',
  `access` int(11) NOT NULL,
  `maxdepotitems` int(11) NOT NULL,
  `maxviplist` int(11) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

--
-- Extraindo dados da tabela `groups`
--

INSERT INTO `groups` (`id`, `name`, `flags`, `access`, `maxdepotitems`, `maxviplist`) VALUES
(1, 'player', 0, 0, 20, 20);

-- --------------------------------------------------------

--
-- Estrutura da tabela `guilds`
--

CREATE TABLE IF NOT EXISTS `guilds` (
  `id` int(10) unsigned NOT NULL,
  `name` varchar(255) NOT NULL COMMENT 'guild name - nothing else needed here',
  `ownerid` int(11) NOT NULL,
  `creationdata` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Acionadores `guilds`
--
DELIMITER $$
CREATE TRIGGER `oncreate_guilds` AFTER INSERT ON `guilds`
 FOR EACH ROW BEGIN
    INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Leader', 3, NEW.`id`);
    INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Vice-Leader', 2, NEW.`id`);
    INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Member', 1, NEW.`id`);
END
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `ondelete_guilds` BEFORE DELETE ON `guilds`
 FOR EACH ROW BEGIN
    UPDATE `players` SET `guildnick` = '', `rank_id` = 0 WHERE `rank_id` IN (SELECT `id` FROM `guild_ranks` WHERE `guild_id` = OLD.`id`);
    DELETE FROM `guild_ranks` WHERE `guild_id` = OLD.`id`;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Estrutura da tabela `guild_ranks`
--

CREATE TABLE IF NOT EXISTS `guild_ranks` (
  `id` int(10) unsigned NOT NULL,
  `guild_id` int(10) unsigned NOT NULL COMMENT 'guild',
  `name` varchar(255) NOT NULL COMMENT 'rank name',
  `level` int(11) NOT NULL COMMENT 'rank level - leader, vice, member, maybe something else'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `houses`
--

CREATE TABLE IF NOT EXISTS `houses` (
  `id` int(10) unsigned NOT NULL,
  `owner` int(11) NOT NULL,
  `paid` int(10) unsigned NOT NULL DEFAULT '0',
  `warnings` int(11) NOT NULL DEFAULT '0',
  `lastwarning` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `house_lists`
--

CREATE TABLE IF NOT EXISTS `house_lists` (
  `house_id` int(10) unsigned NOT NULL,
  `listid` int(11) NOT NULL,
  `list` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `map_store`
--

CREATE TABLE IF NOT EXISTS `map_store` (
  `house_id` int(10) unsigned NOT NULL,
  `data` blob NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `players`
--

CREATE TABLE IF NOT EXISTS `players` (
  `id` int(10) unsigned NOT NULL,
  `name` varchar(255) NOT NULL,
  `account_id` int(10) unsigned NOT NULL,
  `group_id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'users group',
  `sex` int(10) unsigned NOT NULL DEFAULT '0',
  `vocation` int(10) unsigned NOT NULL DEFAULT '0',
  `experience` bigint(20) unsigned NOT NULL DEFAULT '0',
  `level` int(10) unsigned NOT NULL DEFAULT '1',
  `health` int(10) unsigned NOT NULL DEFAULT '100',
  `mana` int(10) unsigned NOT NULL DEFAULT '100',
  `manaspent` int(10) unsigned NOT NULL DEFAULT '0',
  `direction` int(10) unsigned NOT NULL DEFAULT '0',
  `looktype` int(10) unsigned NOT NULL DEFAULT '136',
  `lookhead` int(10) unsigned NOT NULL DEFAULT '10',
  `lookbody` int(10) unsigned NOT NULL DEFAULT '10',
  `looklegs` int(10) unsigned NOT NULL DEFAULT '10',
  `lookfeet` int(10) unsigned NOT NULL DEFAULT '10',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `posz` int(11) NOT NULL DEFAULT '0',
  `lastlogin` int(10) unsigned NOT NULL DEFAULT '0',
  `lastlogout` int(10) unsigned NOT NULL DEFAULT '0',
  `lastip` int(10) unsigned NOT NULL DEFAULT '0',
  `save` tinyint(1) NOT NULL DEFAULT '1',
  `conditions` blob NOT NULL COMMENT 'drunk, poisoned etc (maybe also food and redskull)',
  `redskulltime` int(10) unsigned NOT NULL DEFAULT '0',
  `redskull` tinyint(1) NOT NULL DEFAULT '0',
  `guildnick` varchar(255) NOT NULL DEFAULT '' COMMENT 'additional nick in guild - mostly for web interfaces i think',
  `loss_experience` int(11) NOT NULL DEFAULT '10',
  `loss_mana` int(11) NOT NULL DEFAULT '10',
  `loss_skills` int(11) NOT NULL DEFAULT '10',
  `loss_items` int(11) NOT NULL DEFAULT '10',
  `rank_id` int(10) unsigned NOT NULL COMMENT 'by this field everything with guilds is done - player has a rank which belongs to certain guild',
  `town_id` int(11) NOT NULL COMMENT 'old masterpos, temple spawn point position',
  `balance` int(11) NOT NULL DEFAULT '0' COMMENT 'money balance of the player for houses paying',
  `status` tinyint(1) NOT NULL DEFAULT '0',
  `levelPoints` int(11) NOT NULL DEFAULT '0',
  `unusedMagicPoints` int(10) unsigned DEFAULT '0',
  `breath` int(10) unsigned DEFAULT '200'
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;

--
-- Extraindo dados da tabela `players`
--

INSERT INTO `players` (`id`, `name`, `account_id`, `group_id`, `sex`, `vocation`, `experience`, `level`, `health`, `mana`, `manaspent`, `direction`, `looktype`, `lookhead`, `lookbody`, `looklegs`, `lookfeet`, `posx`, `posy`, `posz`, `lastlogin`, `lastlogout`, `lastip`, `save`, `conditions`, `redskulltime`, `redskull`, `guildnick`, `loss_experience`, `loss_mana`, `loss_skills`, `loss_items`, `rank_id`, `town_id`, `balance`, `status`, `levelPoints`, `unusedMagicPoints`, `breath`) VALUES
(7, 'francicso new', 1, 1, 1, 1, 1, 1, 300, 300, 0, 0, 2, 4278190335, 4294967295, 23, 16711680, 786, 857, 7, 1461734377, 0, 16777343, 0, '', 0, 0, '0', 0, 0, 0, 0, 1, 1, 0, 0, 10, 0, NULL),
(8, 'francicso ', 1, 1, 1, 1, 1, 1, 300, 300, 0, 0, 2, 4278190335, 4294967295, 23, 16711680, 786, 857, 7, 1462654893, 0, 16777343, 0, '', 0, 0, '0', 0, 0, 0, 0, 1, 1, 0, 0, 100, 0, 100),
(9, 'druid', 1, 1, 1, 2, 1, 1, 300, 300, 0, 0, 2, 4278190335, 4294967295, 23, 16711680, 786, 857, 7, 1462501817, 0, 16777343, 0, '', 0, 0, '0', 0, 0, 0, 0, 1, 1, 0, 0, 100, 0, 100);

--
-- Acionadores `players`
--
DELIMITER $$
CREATE TRIGGER `oncreate_players` AFTER INSERT ON `players`
 FOR EACH ROW BEGIN
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 0, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 1, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 2, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 3, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 4, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 5, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 6, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 7, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 8, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 9, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 10, 300);
    INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES (NEW.`id`, 11, 300);
END
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `ondelete_players` BEFORE DELETE ON `players`
 FOR EACH ROW BEGIN
    DELETE FROM `bans` WHERE `type` = 2 AND `value` = OLD.`id`;
    UPDATE `houses` SET `owner` = 0 WHERE `owner` = OLD.`id`;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Estrutura da tabela `player_deaths`
--

CREATE TABLE IF NOT EXISTS `player_deaths` (
  `player_id` int(10) unsigned NOT NULL,
  `time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `level` int(10) unsigned NOT NULL DEFAULT '1',
  `lasthit_killer` varchar(255) NOT NULL,
  `mostdamage_killer` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `player_depotitems`
--

CREATE TABLE IF NOT EXISTS `player_depotitems` (
  `player_id` int(10) unsigned NOT NULL,
  `depot_id` int(11) NOT NULL DEFAULT '0',
  `sid` int(11) NOT NULL COMMENT 'any given range eg 0-100 will be reserved for depot lockers and all > 100 will be then normal items inside depots',
  `pid` int(11) NOT NULL DEFAULT '0',
  `itemtype` int(11) NOT NULL,
  `count` int(11) NOT NULL DEFAULT '0',
  `attributes` blob NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `player_items`
--

CREATE TABLE IF NOT EXISTS `player_items` (
  `player_id` int(10) unsigned NOT NULL,
  `sid` int(11) NOT NULL,
  `pid` int(11) NOT NULL DEFAULT '0',
  `itemtype` int(11) NOT NULL,
  `count` int(11) NOT NULL DEFAULT '0',
  `attributes` blob COMMENT 'replaces unique_id, action_id, text, special_desc'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `player_skills`
--

CREATE TABLE IF NOT EXISTS `player_skills` (
  `player_id` int(10) unsigned NOT NULL,
  `attribute` int(10) unsigned NOT NULL,
  `value` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Extraindo dados da tabela `player_skills`
--

INSERT INTO `player_skills` (`player_id`, `attribute`, `value`) VALUES
(7, 0, 300),
(7, 1, 300),
(7, 2, 300),
(7, 3, 300),
(7, 4, 300),
(7, 5, 300),
(7, 6, 300),
(7, 7, 300),
(7, 8, 300),
(7, 9, 300),
(7, 10, 300),
(7, 11, 300),
(8, 0, 300),
(8, 1, 300),
(8, 2, 300),
(8, 3, 300),
(8, 4, 300),
(8, 5, 300),
(8, 6, 300),
(8, 7, 300),
(8, 8, 300),
(8, 9, 300),
(8, 10, 300),
(8, 11, 300),
(9, 0, 300),
(9, 1, 300),
(9, 2, 300),
(9, 3, 300),
(9, 4, 300),
(9, 5, 300),
(9, 6, 300),
(9, 7, 300),
(9, 8, 300),
(9, 9, 300),
(9, 10, 300),
(9, 11, 300);

-- --------------------------------------------------------

--
-- Estrutura da tabela `player_spells`
--

CREATE TABLE IF NOT EXISTS `player_spells` (
  `player_id` int(10) unsigned NOT NULL,
  `spell_id` int(11) NOT NULL,
  `level` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Extraindo dados da tabela `player_spells`
--

INSERT INTO `player_spells` (`player_id`, `spell_id`, `level`) VALUES
(8, 1, 0),
(8, 2, 0),
(8, 3, 0),
(8, 4, 0),
(8, 5, 0),
(8, 6, 0),
(8, 7, 0),
(8, 8, 0),
(8, 9, 0),
(8, 11, 0),
(8, 12, 0),
(9, 21, 0);

-- --------------------------------------------------------

--
-- Estrutura da tabela `player_storage`
--

CREATE TABLE IF NOT EXISTS `player_storage` (
  `player_id` int(10) unsigned NOT NULL,
  `key` int(11) NOT NULL,
  `value` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `player_viplist`
--

CREATE TABLE IF NOT EXISTS `player_viplist` (
  `player_id` int(10) unsigned NOT NULL COMMENT 'id of player whose viplist entry it is',
  `vip_id` int(10) unsigned NOT NULL COMMENT 'id of target player of viplist entry'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `server_record`
--

CREATE TABLE IF NOT EXISTS `server_record` (
  `record` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Extraindo dados da tabela `server_record`
--

INSERT INTO `server_record` (`record`) VALUES
(2);

-- --------------------------------------------------------

--
-- Estrutura da tabela `tiles`
--

CREATE TABLE IF NOT EXISTS `tiles` (
  `id` int(10) unsigned NOT NULL,
  `x` int(11) NOT NULL,
  `y` int(11) NOT NULL,
  `z` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Estrutura da tabela `tile_items`
--

CREATE TABLE IF NOT EXISTS `tile_items` (
  `tile_id` int(10) unsigned NOT NULL,
  `sid` int(11) NOT NULL,
  `pid` int(11) NOT NULL DEFAULT '0',
  `itemtype` int(11) NOT NULL,
  `count` int(11) NOT NULL DEFAULT '0',
  `attributes` blob NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `accounts`
--
ALTER TABLE `accounts`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `bans`
--
ALTER TABLE `bans`
  ADD PRIMARY KEY (`id`),
  ADD KEY `type` (`type`,`value`),
  ADD KEY `expires` (`expires`);

--
-- Indexes for table `global_storage`
--
ALTER TABLE `global_storage`
  ADD PRIMARY KEY (`key`);

--
-- Indexes for table `groups`
--
ALTER TABLE `groups`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `guilds`
--
ALTER TABLE `guilds`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `guild_ranks`
--
ALTER TABLE `guild_ranks`
  ADD PRIMARY KEY (`id`),
  ADD KEY `guild_id` (`guild_id`);

--
-- Indexes for table `houses`
--
ALTER TABLE `houses`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `house_lists`
--
ALTER TABLE `house_lists`
  ADD KEY `house_id` (`house_id`);

--
-- Indexes for table `map_store`
--
ALTER TABLE `map_store`
  ADD UNIQUE KEY `house_id` (`house_id`);

--
-- Indexes for table `players`
--
ALTER TABLE `players`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`),
  ADD KEY `account_id` (`account_id`),
  ADD KEY `group_id` (`group_id`);

--
-- Indexes for table `player_deaths`
--
ALTER TABLE `player_deaths`
  ADD KEY `player_id` (`player_id`);

--
-- Indexes for table `player_depotitems`
--
ALTER TABLE `player_depotitems`
  ADD UNIQUE KEY `player_id_2` (`player_id`,`sid`),
  ADD KEY `player_id` (`player_id`,`depot_id`);

--
-- Indexes for table `player_items`
--
ALTER TABLE `player_items`
  ADD UNIQUE KEY `player_id` (`player_id`,`sid`);

--
-- Indexes for table `player_skills`
--
ALTER TABLE `player_skills`
  ADD KEY `player_id` (`player_id`);

--
-- Indexes for table `player_spells`
--
ALTER TABLE `player_spells`
  ADD PRIMARY KEY (`spell_id`),
  ADD KEY `player_id` (`player_id`);

--
-- Indexes for table `player_storage`
--
ALTER TABLE `player_storage`
  ADD KEY `player_id` (`player_id`);

--
-- Indexes for table `player_viplist`
--
ALTER TABLE `player_viplist`
  ADD KEY `player_id` (`player_id`),
  ADD KEY `vip_id` (`vip_id`);

--
-- Indexes for table `server_record`
--
ALTER TABLE `server_record`
  ADD PRIMARY KEY (`record`);

--
-- Indexes for table `tiles`
--
ALTER TABLE `tiles`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tile_items`
--
ALTER TABLE `tile_items`
  ADD KEY `tile_id` (`tile_id`),
  ADD KEY `sid` (`sid`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `accounts`
--
ALTER TABLE `accounts`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `bans`
--
ALTER TABLE `bans`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `groups`
--
ALTER TABLE `groups`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `guilds`
--
ALTER TABLE `guilds`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `guild_ranks`
--
ALTER TABLE `guild_ranks`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `players`
--
ALTER TABLE `players`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=10;
--
-- Constraints for dumped tables
--

--
-- Limitadores para a tabela `guild_ranks`
--
ALTER TABLE `guild_ranks`
  ADD CONSTRAINT `guild_ranks_ibfk_1` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `house_lists`
--
ALTER TABLE `house_lists`
  ADD CONSTRAINT `house_lists_ibfk_1` FOREIGN KEY (`house_id`) REFERENCES `houses` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `players`
--
ALTER TABLE `players`
  ADD CONSTRAINT `players_ibfk_1` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `players_ibfk_2` FOREIGN KEY (`group_id`) REFERENCES `groups` (`id`);

--
-- Limitadores para a tabela `player_deaths`
--
ALTER TABLE `player_deaths`
  ADD CONSTRAINT `player_deaths_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `player_depotitems`
--
ALTER TABLE `player_depotitems`
  ADD CONSTRAINT `player_depotitems_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `player_items`
--
ALTER TABLE `player_items`
  ADD CONSTRAINT `player_items_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `player_skills`
--
ALTER TABLE `player_skills`
  ADD CONSTRAINT `player_skills_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `player_spells`
--
ALTER TABLE `player_spells`
  ADD CONSTRAINT `player_spells_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `player_storage`
--
ALTER TABLE `player_storage`
  ADD CONSTRAINT `player_storage_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `player_viplist`
--
ALTER TABLE `player_viplist`
  ADD CONSTRAINT `player_viplist_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `player_viplist_ibfk_2` FOREIGN KEY (`vip_id`) REFERENCES `players` (`id`) ON DELETE CASCADE;

--
-- Limitadores para a tabela `tile_items`
--
ALTER TABLE `tile_items`
  ADD CONSTRAINT `tile_items_ibfk_1` FOREIGN KEY (`tile_id`) REFERENCES `tiles` (`id`) ON DELETE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
