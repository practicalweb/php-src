--TEST--
Bug #40451 (addAttribute() may crash when used with non-existent child node)
--SKIPIF--
<?php if (!extension_loaded("simplexml")) die("skip"); ?>
--FILE--
<?php

$string = <<<XML
<?xml version="1.0"?>
	<Host enable="true">
	 <Name>host.server.com</Name>
	 </Host>
XML;

$xml = simplexml_load_string((binary)$string);

$add = $xml->addChild('Host');
$add->Host->addAttribute('enable', 'true');

echo "Done\n";
?>
--EXPECTF--	
Warning: SimpleXMLElement::addAttribute(): Unable to locate parent Element in %s on line %d
Done
