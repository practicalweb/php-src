--TEST--
Test array_slice() function : usage variations - different data types as keys in an array
--FILE--
<?php
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as keys in an array to array_slice()
 * to test how $preserve_keys treats them
 */

echo "*** Testing array_slice() : usage variations ***\n";

// Initialise function arguments not being substituted
$offset = 0;
$length = 10; // to ensure all elements are displayed

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays of different data types to be passed as $input
$inputs = array(

       // int data
/*1*/  'int' => array(
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ),

       // float data
/*2*/  'float' => array(
       10.5 => 'positive',
       -10.5 => 'negative',
       .5 => 'half',
       ),
       
/*3*/  'extreme floats' => array(
       12.3456789000e6 => 'large',
       12.3456789000E-10 => 'small',
       ),

       // null data
/*4*/ 'null uppercase' => array(
       NULL => 'null 1',
       ), 
       
/*5*/  'null lowercase' => array(
       null => 'null 2',
       ),

       // boolean data
/*6*/ 'bool lowercase' => array(
       true => 'lowert',
       false => 'lowerf',
       ),
       
/*7*/  'bool uppercase' => array(
       TRUE => 'uppert',
       FALSE => 'upperf',
       ),
       
       // empty data
/*8*/ 'empty double quotes' => array(
       "" => 'emptyd',
       ),
       
/*9*/  'empty single quotes' => array(
       '' => 'emptys',
       ),

       // string data
/*10*/ 'string' => array(
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ),

       // undefined data
/*11*/ 'undefined' => array(
       @$undefined_var => 'undefined',
       ),

       // unset data
/*12*/ 'unset' => array(
       @$unset_var => 'unset',
       ),
);

// loop through each element of $inputs to check the behavior of array_slice()
$iterator = 1;
foreach($inputs as $type => $input) {
  echo "\n-- Iteration $iterator : key type is $type --\n";
  echo "\$preserve_keys = TRUE\n";
  var_dump( array_slice($input, $offset, $length, true) );
  echo "\$preserve_keys = FALSE\n";
  var_dump( array_slice($input, $offset, $length, false) );
  $iterator++;
};

echo "Done";
?>

--EXPECT--
*** Testing array_slice() : usage variations ***

-- Iteration 1 : key type is int --
$preserve_keys = TRUE
array(4) {
  [0]=>
  unicode(4) "zero"
  [1]=>
  unicode(3) "one"
  [12345]=>
  unicode(8) "positive"
  [-2345]=>
  unicode(8) "negative"
}
$preserve_keys = FALSE
array(4) {
  [0]=>
  unicode(4) "zero"
  [1]=>
  unicode(3) "one"
  [2]=>
  unicode(8) "positive"
  [3]=>
  unicode(8) "negative"
}

-- Iteration 2 : key type is float --
$preserve_keys = TRUE
array(3) {
  [10]=>
  unicode(8) "positive"
  [-10]=>
  unicode(8) "negative"
  [0]=>
  unicode(4) "half"
}
$preserve_keys = FALSE
array(3) {
  [0]=>
  unicode(8) "positive"
  [1]=>
  unicode(8) "negative"
  [2]=>
  unicode(4) "half"
}

-- Iteration 3 : key type is extreme floats --
$preserve_keys = TRUE
array(2) {
  [12345678]=>
  unicode(5) "large"
  [0]=>
  unicode(5) "small"
}
$preserve_keys = FALSE
array(2) {
  [0]=>
  unicode(5) "large"
  [1]=>
  unicode(5) "small"
}

-- Iteration 4 : key type is null uppercase --
$preserve_keys = TRUE
array(1) {
  [u""]=>
  unicode(6) "null 1"
}
$preserve_keys = FALSE
array(1) {
  [u""]=>
  unicode(6) "null 1"
}

-- Iteration 5 : key type is null lowercase --
$preserve_keys = TRUE
array(1) {
  [u""]=>
  unicode(6) "null 2"
}
$preserve_keys = FALSE
array(1) {
  [u""]=>
  unicode(6) "null 2"
}

-- Iteration 6 : key type is bool lowercase --
$preserve_keys = TRUE
array(2) {
  [1]=>
  unicode(6) "lowert"
  [0]=>
  unicode(6) "lowerf"
}
$preserve_keys = FALSE
array(2) {
  [0]=>
  unicode(6) "lowert"
  [1]=>
  unicode(6) "lowerf"
}

-- Iteration 7 : key type is bool uppercase --
$preserve_keys = TRUE
array(2) {
  [1]=>
  unicode(6) "uppert"
  [0]=>
  unicode(6) "upperf"
}
$preserve_keys = FALSE
array(2) {
  [0]=>
  unicode(6) "uppert"
  [1]=>
  unicode(6) "upperf"
}

-- Iteration 8 : key type is empty double quotes --
$preserve_keys = TRUE
array(1) {
  [u""]=>
  unicode(6) "emptyd"
}
$preserve_keys = FALSE
array(1) {
  [u""]=>
  unicode(6) "emptyd"
}

-- Iteration 9 : key type is empty single quotes --
$preserve_keys = TRUE
array(1) {
  [u""]=>
  unicode(6) "emptys"
}
$preserve_keys = FALSE
array(1) {
  [u""]=>
  unicode(6) "emptys"
}

-- Iteration 10 : key type is string --
$preserve_keys = TRUE
array(3) {
  [u"stringd"]=>
  unicode(7) "stringd"
  [u"strings"]=>
  unicode(7) "strings"
  [u"hello world"]=>
  unicode(7) "stringh"
}
$preserve_keys = FALSE
array(3) {
  [u"stringd"]=>
  unicode(7) "stringd"
  [u"strings"]=>
  unicode(7) "strings"
  [u"hello world"]=>
  unicode(7) "stringh"
}

-- Iteration 11 : key type is undefined --
$preserve_keys = TRUE
array(1) {
  [u""]=>
  unicode(9) "undefined"
}
$preserve_keys = FALSE
array(1) {
  [u""]=>
  unicode(9) "undefined"
}

-- Iteration 12 : key type is unset --
$preserve_keys = TRUE
array(1) {
  [u""]=>
  unicode(5) "unset"
}
$preserve_keys = FALSE
array(1) {
  [u""]=>
  unicode(5) "unset"
}
Done
