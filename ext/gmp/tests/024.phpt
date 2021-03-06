--TEST--
gmp_jacobi() basic tests
--SKIPIF--
<?php if (!extension_loaded("gmp")) print "skip"; ?>
--FILE--
<?php

for ($i = -1; $i < 10; $i++) {
	var_dump(gmp_strval(gmp_jacobi(($i*$i)-1, 3)));
}

var_dump(gmp_strval(gmp_jacobi(7, 23)));
var_dump(gmp_strval(gmp_jacobi("733535124", "1234123423434535623")));
var_dump(gmp_strval(gmp_jacobi(3, "1234123423434535623")));

$n = "123123";
$n1 = "1231231";

var_dump(gmp_strval(gmp_jacobi($n, $n1)));
var_dump(gmp_strval(gmp_jacobi($n, 3)));
var_dump(gmp_strval(gmp_jacobi(3, $n1)));

var_dump(gmp_jacobi(3, array()));
var_dump(gmp_jacobi(array(), 3));
var_dump(gmp_jacobi(array(), array()));

var_dump(gmp_jacobi(array(), array(), 1));
var_dump(gmp_jacobi(array()));
var_dump(gmp_jacobi());

echo "Done\n";
?>
--EXPECTF--
unicode(1) "0"
unicode(2) "-1"
unicode(1) "0"
unicode(1) "0"
unicode(2) "-1"
unicode(1) "0"
unicode(1) "0"
unicode(2) "-1"
unicode(1) "0"
unicode(1) "0"
unicode(2) "-1"
unicode(2) "-1"
unicode(1) "0"
unicode(1) "0"
unicode(2) "-1"
unicode(1) "0"
unicode(2) "-1"

Warning: gmp_jacobi(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)

Warning: gmp_jacobi(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)

Warning: gmp_jacobi(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)

Warning: gmp_jacobi() expects exactly 2 parameters, 3 given in %s on line %d
NULL

Warning: gmp_jacobi() expects exactly 2 parameters, 1 given in %s on line %d
NULL

Warning: gmp_jacobi() expects exactly 2 parameters, 0 given in %s on line %d
NULL
Done
