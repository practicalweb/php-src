--TEST--
Test sprintf() function : usage variations - char formats with string values
--FILE--
<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : char formats with string values ***\n";

// array of string values 
$string_values = array(
  "",
  '',
  "0",
  '0',
  "1",
  '1',
  "\x01",
  '\x01',
  "\01",
  '\01',
  'string',
  "string",
  "true",
  "FALSE",
  'false',
  'TRUE',
  "NULL",
  'null'
);

// array of char formats
$char_formats = array( 
  "%c", "%hc", "%lc", 
  "%Lc", " %c", "%c ",
  "\t%c", "\n%c", "%4c",
  "%30c", "%[a-bA-B@#$&]", "%*c"
);

$count = 1;
foreach($string_values as $string_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($char_formats as $format) {
    var_dump( sprintf($format, $string_value) );
  }
  $count++;
};

echo "Done";
?>
--EXPECT--
*** Testing sprintf() : char formats with string values ***

-- Iteration 1 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 2 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 3 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 4 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 5 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 6 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 7 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 8 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 9 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 10 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 11 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 12 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 13 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 14 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 15 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 16 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 17 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 18 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"
Done
