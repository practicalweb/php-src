--TEST--
sqlite: fetch column
--INI--
sqlite.assoc_case=0
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded("sqlite")) print "skip"; ?>
--FILE--
<?php 
include "blankdb.inc";

$data = array(
	array (0 => 'one', 1 => 'two'),
	array (0 => 'three', 1 => 'four')
	);

sqlite_query("CREATE TABLE strings(a VARCHAR, b VARCHAR)", $db);

foreach ($data as $str) {
	sqlite_query("INSERT INTO strings VALUES('${str[0]}','${str[1]}')", $db);
}

echo "====BUFFERED====\n";
$r = sqlite_query("SELECT a, b from strings", $db);
while (sqlite_valid($r)) {
	var_dump(sqlite_current($r, SQLITE_NUM));
	var_dump(sqlite_column($r, 0));
	var_dump(sqlite_column($r, 1));
	var_dump(sqlite_column($r, 'a'));
	var_dump(sqlite_column($r, 'b'));
	sqlite_next($r);
}
echo "====UNBUFFERED====\n";
$r = sqlite_unbuffered_query("SELECT a, b from strings", $db);
while (sqlite_valid($r)) {
	var_dump(sqlite_column($r, 0));
	var_dump(sqlite_column($r, 'b'));
	var_dump(sqlite_column($r, 1));
	var_dump(sqlite_column($r, 'a'));
	sqlite_next($r);
}

sqlite_close($db);

echo "DONE!\n";
?>
--EXPECT--
====BUFFERED====
array(2) {
  [0]=>
  unicode(3) "one"
  [1]=>
  unicode(3) "two"
}
unicode(3) "one"
unicode(3) "two"
unicode(3) "one"
unicode(3) "two"
array(2) {
  [0]=>
  unicode(5) "three"
  [1]=>
  unicode(4) "four"
}
unicode(5) "three"
unicode(4) "four"
unicode(5) "three"
unicode(4) "four"
====UNBUFFERED====
unicode(3) "one"
unicode(3) "two"
NULL
NULL
unicode(5) "three"
unicode(4) "four"
NULL
NULL
DONE!
