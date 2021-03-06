--TEST--
mysqli_change_user() - ROLLBACK
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifemb.inc');
require_once('skipifconnectfailure.inc');
require_once('connect.inc');
$link = mysqli_connect($host, $user, $passwd, $db, $port, $socket);
$result = mysqli_query($link, "SHOW VARIABLES LIKE 'have_innodb'");
$row = mysqli_fetch_row($result);
mysqli_free_result($result);
mysqli_close($link);

if ($row[1] == 'NO') {
	printf ("skip ROLLBACK requires transactional engine InnoDB");
}
?>
--FILE--
<?php
	require_once('connect.inc');
	require_once('table.inc');

	if (!mysqli_query($link, 'ALTER TABLE test ENGINE=InnoDB'))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_autocommit($link, false);

	if (!$res = mysqli_query($link, 'SELECT COUNT(*) AS _num FROM test'))
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$row = mysqli_fetch_assoc($res))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_free_result($res);

	$num = $row['_num'];
	assert($num > 0);

	if (!$res = mysqli_query($link, 'DELETE FROM test'))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$res = mysqli_query($link, 'SELECT COUNT(*) AS _num FROM test'))
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$row = mysqli_fetch_assoc($res))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_free_result($res);

	if (0 != $row['_num'])
		printf("[007] Rows should have been deleted in this transaction\n");

	// DELETE should be rolled back
	mysqli_change_user($link, $user, $passwd, $db);

	if (!$res = mysqli_query($link, 'SELECT COUNT(*) AS _num FROM test'))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$row = mysqli_fetch_assoc($res))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if ($row['_num'] != $num)
		printf("[010] Expecting %d rows in the table test, found %d rows\n",
			$num, $row['_num']);

	mysqli_free_result($res);
	mysqli_close($link);
	print "done!";
?>
--EXPECTF--
done!