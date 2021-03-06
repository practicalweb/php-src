--TEST--
mysqli_fetch_object
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	mysqli_select_db($link, $db);
	mysqli_query($link, "SET sql_mode=''");

	mysqli_query($link,"DROP TABLE IF EXISTS test_bind_fetch");
	mysqli_query($link,"CREATE TABLE test_bind_fetch(c1 smallint unsigned,
		c2 smallint unsigned,
		c3 smallint,
		c4 smallint,
		c5 smallint,
		c6 smallint unsigned,
		c7 smallint) ENGINE=" . $engine);

	$stmt = mysqli_prepare($link, "INSERT INTO test_bind_fetch VALUES (?,?,?,?,?,?,?)");
	mysqli_bind_param($stmt, "iiiiiii", $c1,$c2,$c3,$c4,$c5,$c6,$c7);

	$c1 = -23;
	$c2 = 35999;
	$c3 = NULL;
	$c4 = -500;
	$c5 = -9999999;
	$c6 = -0;
	$c7 = 0;

	mysqli_execute($stmt);
	mysqli_stmt_close($stmt);

	$result = mysqli_query($link, "SELECT * FROM test_bind_fetch");
	$test = mysqli_fetch_object($result);
	mysqli_free_result($result);

	var_dump($test);

	mysqli_query($link, "DROP TABLE IF EXISTS test_bind_fetch");
	mysqli_close($link);
	print "done!"
?>
--EXPECTF--
object(stdClass)#%d (7) {
  [%u|b%"c1"]=>
  %unicode|string%(1) "0"
  [%u|b%"c2"]=>
  %unicode|string%(5) "35999"
  [%u|b%"c3"]=>
  NULL
  [%u|b%"c4"]=>
  %unicode|string%(4) "-500"
  [%u|b%"c5"]=>
  %unicode|string%(6) "-32768"
  [%u|b%"c6"]=>
  %unicode|string%(1) "0"
  [%u|b%"c7"]=>
  %unicode|string%(1) "0"
}
done!