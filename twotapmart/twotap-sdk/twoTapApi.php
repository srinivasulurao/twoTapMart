<?php
/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
error_reporting(1);
/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
class twoTap{
    public $api_url="https://api.twotap.com/v1.0/add_to_cart?public_token=";
    public $public_token="be84fcbb686c42df83f8c321927e19";
    public $private_token="8a1703f8c60c480ce0acaefd3667a5da06d9d1f2316aa13042";
    public $curl_type="POST";
public function response($post=array())
{
//$certificate=getcwd()."/".drupal_get_path('module','twotapmart')."/twotap-sdk/mozilla.pem";
$curl_url=$this->api_url.$this->public_token;
//print_r($post);

// Get cURL resource
$curl = curl_init();
// Set some options - we are passing in a useragent too here
curl_setopt_array($curl, array(
CURLOPT_RETURNTRANSFER => 1,
CURLOPT_URL =>$curl_url,
CURLOPT_POST=>1,
CURLOPT_CUSTOMREQUEST=>$this->curl_type,
CURLOPT_POSTFIELDS=>$post,
CURLOPT_SSL_VERIFYPEER =>false,
CURLOPT_USERAGENT => 'Curl Request for getting TwoTap Response',
CURLOPT_FOLLOWLOCATION=>1
));
// Send the request & save response to $response variable.
$response = curl_exec($curl);
echo curl_error ($curl);
return $response;
}

public function get($post=array())
{
//$certificate=getcwd()."/".drupal_get_path('module','twotapmart')."/twotap-sdk/mozilla.pem";
echo $curl_url=$this->api_url.$this->public_token;
//print_r($post);

// Get cURL resource
$curl = curl_init();
// Set some options - we are passing in a useragent too here
curl_setopt_array($curl, array(
CURLOPT_RETURNTRANSFER => 1,
CURLOPT_URL =>$curl_url,
CURLOPT_POST=>0,
CURLOPT_CUSTOMREQUEST=>$this->curl_type,
CURLOPT_POSTFIELDS=>$post,
CURLOPT_SSL_VERIFYPEER =>false,
CURLOPT_USERAGENT => 'Curl Request for getting TwoTap Response',
CURLOPT_FOLLOWLOCATION=>1
));
// Send the request & save response to $response variable.
$response = curl_exec($curl);
echo curl_error ($curl);
return $response;
}



}