--TEST--
ocistatementtype()
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__)."/connect.inc";

if (!empty($dbase)) {
	var_dump($c = ocilogon($user, $password, $dbase));
}
else {
	var_dump($c = ocilogon($user, $password));
}

$sqls = Array(
    "SELECT * FROM table",
    "DELETE FROM table WHERE id = 1",
    "INSERT INTO table VALUES(1)",
    "UPDATE table SET id = 1",
	"DROP TABLE table",
	"CREATE OR REPLACE PROCEDURE myproc(v1 NUMBER) as BEGIN DBMS_OUTPUT.PUT_LINE(v1); END;",
    "CREATE TABLE table (id NUMBER)",
    "ALTER TABLE table ADD (col1 NUMBER)",
    "BEGIN NULL; END;",
    "DECLARE myn NUMBER BEGIN myn := 1; END;",
    "CALL myproc(1)",
    "WRONG SYNTAX",
    ""
);

foreach ($sqls as $sql) {
	$s = ociparse($c, $sql);
	var_dump(ocistatementtype($s));
}

echo "Done\n";

?>
--EXPECTF--
resource(%d) of type (oci8 connection)
unicode(6) "SELECT"
unicode(6) "DELETE"
unicode(6) "INSERT"
unicode(6) "UPDATE"
unicode(4) "DROP"
unicode(6) "CREATE"
unicode(6) "CREATE"
unicode(5) "ALTER"
unicode(5) "BEGIN"
unicode(7) "DECLARE"
unicode(4) "CALL"
unicode(7) "UNKNOWN"
unicode(7) "UNKNOWN"
Done
