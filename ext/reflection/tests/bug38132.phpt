--TEST--
Reflection Bug #38132 (ReflectionClass::getStaticProperties() retains \0 in key names)
--FILE--
<?php
class foo {
	static protected $bar = 'baz';
	static public $a = 'a';
}

$class = new ReflectionClass('foo');
$properties = $class->getStaticProperties();
var_dump($properties, array_keys($properties));
var_dump(isset($properties['*bar']));
var_dump(isset($properties["\0*\0bar"]));
var_dump(isset($properties["bar"]));
?>
--EXPECT--
array(2) {
  [u"bar"]=>
  unicode(3) "baz"
  [u"a"]=>
  unicode(1) "a"
}
array(2) {
  [0]=>
  unicode(3) "bar"
  [1]=>
  unicode(1) "a"
}
bool(false)
bool(false)
bool(true)
