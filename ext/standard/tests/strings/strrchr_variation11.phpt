--TEST--
Test strrchr() function : usage variations - unexpected inputs for haystack and needle
--FILE--
<?php
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function with unexpected inputs for haystack and needle */

echo "*** Testing strrchr() function: with unexpected inputs for haystack and needle ***\n";

// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

// declaring a class
class sample  {
  public function __toString() {
    return "object";
  } 
}

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values
$values =  array (

  // integer values
  0,
  1,
  12345,
  -2345,

  // float values
  10.5,
  -10.5,
  10.5e10,
  10.6E-10,
  .5,

  // array values
  array(),
  array(0),
  array(1),
  array(1, 2),
  array('color' => 'red', 'item' => 'pen'),

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // objects
  new sample(),

  // empty string
  "",
  '',

  // null vlaues
  NULL,
  null,

  // resource
  $file_handle,

  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var
);


// loop through each element of the array and check the working of strrchr()
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  var_dump( strrchr($values[$index], $values[$index]) );
  $counter ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***";
?>
--EXPECTF--
*** Testing strrchr() function: with unexpected inputs for haystack and needle ***
-- Iteration 1 --
bool(false)
-- Iteration 2 --
bool(false)
-- Iteration 3 --
bool(false)
-- Iteration 4 --

Warning: Needle argument codepoint value out of range (0 - 0x10FFFF) in %s on line %d
bool(false)
-- Iteration 5 --
bool(false)
-- Iteration 6 --

Warning: Needle argument codepoint value out of range (0 - 0x10FFFF) in %s on line %d
bool(false)
-- Iteration 7 --

Warning: Needle argument codepoint value out of range (0 - 0x10FFFF) in %s on line %d
bool(false)
-- Iteration 8 --
bool(false)
-- Iteration 9 --
bool(false)
-- Iteration 10 --

Notice: Array to string conversion in %s on line %d
bool(false)
-- Iteration 11 --

Notice: Array to string conversion in %s on line %d
bool(false)
-- Iteration 12 --

Notice: Array to string conversion in %s on line %d
bool(false)
-- Iteration 13 --

Notice: Array to string conversion in %s on line %d
bool(false)
-- Iteration 14 --

Notice: Array to string conversion in %s on line %d
bool(false)
-- Iteration 15 --
bool(false)
-- Iteration 16 --
bool(false)
-- Iteration 17 --
bool(false)
-- Iteration 18 --
bool(false)
-- Iteration 19 --

Notice: Object of class sample could not be converted to int in %s on line %d
bool(false)
-- Iteration 20 --
bool(false)
-- Iteration 21 --
bool(false)
-- Iteration 22 --
bool(false)
-- Iteration 23 --
bool(false)
-- Iteration 24 --
%s
-- Iteration 25 --
bool(false)
-- Iteration 26 --
bool(false)
*** Done ***
