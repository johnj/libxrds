<?php

$discovery = new Xrds();
if($discovery->discover("http://jawed.name/xrds.php")) 
{
    echo "URI: " . $discovery->ServiceURIForType("http://oauth.net/core/1.0/endpoint/request") . "\n";
    echo "Other information about this service: \n";
    print_r($discovery->childElements("http://oauth.net/core/1.0/endpoint/request"));
}
else
{
    echo "Discovery failed\n";
}
?>
