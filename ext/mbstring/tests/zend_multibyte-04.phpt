--TEST--
zend multibyte (4)
--SKIP--
<?php
ini_set("mbstring.script_encoding", "SJIS");
ini_set("mbstring.script_encoding", "***") != "SJIS" or
	die("skip zend-multibyte is not available");
?>
--INI--
mbstring.script_encoding=CP932
mbstring.internal_encoding=UTF-8
--FILE--
<?php
var_dump(bin2hex("�e�X�g"));
?>
--EXPECT--
string(18) "e38386e382b9e38388"
