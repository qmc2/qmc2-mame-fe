SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

--
-- Table structure for table `rom_data`
--

CREATE TABLE IF NOT EXISTS `rom_data` (
  `hunk` bigint(20) unsigned NOT NULL,
  `data` longblob NOT NULL,
  `next_hunk` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`hunk`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii COLLATE=ascii_bin;

--
-- Table structure for table `rom_repo`
--

CREATE TABLE IF NOT EXISTS `rom_repo` (
  `sha1` varchar(64) collate ascii_bin NOT NULL,
  `mod_time` datetime default NULL,
  `mod_user` varchar(32) collate ascii_bin default NULL,
  `lock_time` datetime default NULL,
  `lock_user` varchar(32) collate ascii_bin default NULL,
  `hunk_size` int(10) unsigned NOT NULL default '0',
  `hunk_count` int(10) unsigned NOT NULL default '0',
  `first_hunk` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`sha1`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii COLLATE=ascii_bin;

--
-- Table structure for table `xml_repo`
--

CREATE TABLE IF NOT EXISTS `xml_repo` (
  `version` varchar(16) collate ascii_bin NOT NULL,
  `data` longblob NOT NULL,
  PRIMARY KEY  (`version`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii COLLATE=ascii_bin;

