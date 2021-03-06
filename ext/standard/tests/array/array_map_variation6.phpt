--TEST--
Test array_map() function : usage variations - array having subarrays
--FILE--
<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing array having different subarrays
 */

echo "*** Testing array_map() : array having subarrays ***\n";

function callback($a)
{
  return $a;
}

// different subarrays
$arr1 = array(
  array(),
  array(1, 2),
  array('a', 'b'),
  array(1, 2, 'a', 'b'),
  array(1 => 'a', 'b' => 2)
);  

var_dump( array_map('callback', $arr1));
echo "Done";
?>
--EXPECT--
*** Testing array_map() : array having subarrays ***
array(5) {
  [0]=>
  array(0) {
  }
  [1]=>
  array(2) {
    [0]=>
    int(1)
    [1]=>
    int(2)
  }
  [2]=>
  array(2) {
    [0]=>
    unicode(1) "a"
    [1]=>
    unicode(1) "b"
  }
  [3]=>
  array(4) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    unicode(1) "a"
    [3]=>
    unicode(1) "b"
  }
  [4]=>
  array(2) {
    [1]=>
    unicode(1) "a"
    [u"b"]=>
    int(2)
  }
}
Done
