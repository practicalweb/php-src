--TEST--
POST Method test and arrays - 2
--SKIPIF--
<?php if (php_sapi_name()=='cli') echo 'skip'; ?>
--POST--
a[]=1&a[]=1
--FILE--
<?php
var_dump($_POST['a']); 
?>
--EXPECT--
array(2) {
  [0]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
}
