--TEST--
Phar front controller mime type override, Phar::PHP [cache_list]
--INI--
default_charset=
phar.cache_list={PWD}/frontcontroller16.php
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--ENV--
SCRIPT_NAME=/frontcontroller16.php
REQUEST_URI=/frontcontroller16.php/a.phps
PATH_INFO=/a.phps
--FILE_EXTERNAL--
files/frontcontroller8.phar
--EXPECTHEADERS--
Content-type: text/html
--EXPECT--
hio1
