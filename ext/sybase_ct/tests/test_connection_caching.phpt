--TEST--
Sybase-CT connection caching
--SKIPIF--
<?php require('skipif.inc'); ?>
--FILE--
<?php
/* This file is part of PHP test framework for ext/sybase_ct
 *
 * $Id: test_connection_caching.phpt,v 1.1.2.1 2008/11/09 10:57:09 thekid Exp $ 
 */

  require('test.inc');

  $db1= sybase_connect_ex();
  $db2= sybase_connect_ex();
  var_dump($db1, $db2, (string)$db1 == (string)$db2);
  sybase_close($db1);
  
?>
--EXPECTF--
resource(%d) of type (sybase-ct link)
resource(%d) of type (sybase-ct link)
bool(true)
