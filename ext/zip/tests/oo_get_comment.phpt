--TEST--
Get Comment
--SKIPIF--
<?php
/* $Id: oo_get_comment.phpt,v 1.1.2.1 2006/11/03 16:46:19 pajoye Exp $ */
if(!extension_loaded('zip')) die('skip');
?>
--FILE--
<?php
$dirname = dirname(__FILE__) . '/';
$file = $dirname . 'test_with_comment.zip';
include $dirname . 'utils.inc';
$zip = new ZipArchive;
if (!$zip->open($file)) {
	exit('failed');
}
$idx = $zip->locateName('foo');
echo $zip->getCommentName('foo') . "\n";
echo $zip->getCommentIndex($idx);

$zip->close();

?>
--EXPECTF--
foo comment
foo comment
