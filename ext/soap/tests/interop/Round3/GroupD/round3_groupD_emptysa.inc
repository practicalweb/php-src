<?php
class SOAP_Interop_GroupD {

    function echoString($inputString)
    {
      return $inputString;
    }

}

$server = new SoapServer(dirname(__FILE__)."/round3_groupD_emptysa.wsdl");
$server->setClass("SOAP_Interop_GroupD");
$server->handle();
?>