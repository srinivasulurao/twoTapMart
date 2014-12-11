<?php
require_once($base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/Pest.php");
require_once($base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/PestJSON.php");
$address = 'https://api.twotap.com';
$GLOBALS['TT_PEST'] = new PestJSON($address);
$GLOBALS['TT_PUBLIC_TOKEN'] = 'be84fcbb686c42df83f8c321927e19';
$GLOBALS['TT_PRIVATE_TOKEN'] = '8a1703f8c60c480ce0acaefd3667a5da06d9d1f2316aa13042';
$GLOBALS['TT_TEST_MODE'] = 'none'; // none, dummy_data, fake_confirm.
/*
Two Tap requires different fields depending on what the merchants asks for.
To keep things easy we send all the possible fields that could be required.
*/
$data =array();
$data['email'] = 'yueqi88@gmail.com';
$data['shipping_title'] = $data['billing_title'] = 'Mr.';
$data['shipping_first_name'] = $data['billing_first_name'] = 'Joana';
$data['shipping_last_name'] = $data['billing_last_name'] = 'Peabody';
$data['shipping_address'] = $data['billing_address'] = 'Address';
$data['shipping_city'] = $data['billing_city'] = 'City';
$data['shipping_state'] = $data['billing_state'] = 'State';
$data['shipping_country'] = $data['billing_country'] = 'United States of America';
$data['shipping_zip'] = $data['billing_zip'] = 'ZIP';
$data['shipping_telephone'] = $data['billing_telephone'] = 'PHONE';
$data['shipping_name'] = $data['billing_name'] = 'Joana Peabody';
$data['day_of_birth'] = '10';
$data['month_of_birth'] = '12';
$data['year_of_birth'] = '1981';
$data['gender'] = 'Male';
$data['card_type'] = 'Visa';
$data['card_number'] = '4111111111111111';
$data['card_name'] = 'Card name';
$data['expiry_date_year'] = '2014';
$data['expiry_date_month'] = '12';
$data['cvv'] = '903';
$data['name'] = 'Joana Peabody';
$GLOBALS['product_url'] = 'http://www.gap.com/browse/product.do?cid=1019878&vid=1&pid=140309012';
$GLOBALS['data'] = $data;
//
// ADD TO CART
//
function add_to_cart($productArray) {
$pest = $GLOBALS['TT_PEST'];
$public_token = $GLOBALS['TT_PUBLIC_TOKEN'];
$test_mode = variable_get("twotap_add_to_cart_mode");
$response = $pest->post("/v1.0/add_to_cart?public_token=$public_token&test_mode=$test_mode",$productArray);
$cart_id = $response['cart_id'];
return $cart_id;
}

function add_to_cart_status($cart_id) {
$pest = $GLOBALS['TT_PEST'];
$public_token = $GLOBALS['TT_PUBLIC_TOKEN'];
$test_mode = variable_get("twotap_add_to_cart_status_mode");
$response = $pest->get("/v1.0/add_to_cart/status?cart_id=$cart_id&public_token=$public_token&test_mode=$test_mode");
if ($response['message'] == 'still_processing') {
sleep(5);
return add_to_cart_status($cart_id);
} else {
return $response;
}
}

//
// PURCHASE INFO
//
function purchase_info($cart_id, $fields_input) {
$pest = $GLOBALS['TT_PEST'];
$public_token = $GLOBALS['TT_PUBLIC_TOKEN'];
$test_mode = variable_get("twotap_purchase_info_mode");
$response = $pest->post("/v1.0/purchase_info?public_token=$public_token&test_mode=$test_mode", array("cart_id" => $cart_id, "fields_input" => $fields_input ));
return $response;
}


function purchase_info_status($purchase_id) {
$pest = $GLOBALS['TT_PEST'];
$public_token = $GLOBALS['TT_PUBLIC_TOKEN'];
$test_mode = variable_get("twotap_purchase_info_status_mode");
$response = $pest->get("/v1.0/purchase_info/status?purchase_id=$purchase_id&public_token=$public_token&test_mode=$test_mode");
ini_set("max_execution_time",3000);
if ($response['message'] == 'still_processing') {
sleep(5);
return purchase_info_status($purchase_id);
} else {
return $response;
}
}
//
// PURCHASE CONFIRM
//
function purchase_confirm($purchase_id) {
$pest = $GLOBALS['TT_PEST'];
$private_token = $GLOBALS['TT_PRIVATE_TOKEN'];
$test_mode = variable_get("twotap_purchase_confirm_mode");
// print_r("Starting purchase confirm for: $purchase_id");
$response = $pest->post("/v1.0/purchase_confirm?private_token=$private_token&test_mode=$test_mode",array("purchase_id" => $purchase_id));
// print_r("\nResponse:\n");
// print_r($response);
// $purchase_id = $response['purchase_id'];
// $purchase_confirm_data = purchase_confirm_status($purchase_id);
// if ($purchase_confirm_data['message'] != 'done') {
// print_r("\nPurchase failed:\n");
// print_r($purchase_confirm_data);
// return null;
// } else {
// print_r("\nPurchase succeeded:\n");
// print_r($purchase_confirm_data);
// return $purchase_confirm_data;
// }
return $response;
}

function purchase_confirm_status($purchase_id) {
$pest = $GLOBALS['TT_PEST'];
$public_token = $GLOBALS['TT_PUBLIC_TOKEN'];
$test_mode = variable_get("twotap_purchase_confirm_status_mode");
$response = $pest->get("/v1.0/purchase_confirm/status?purchase_id=$purchase_id&public_token=$public_token&test_mode=$test_mode");
if ($response['message'] == 'still_processing') {
sleep(5);
return purchase_confirm_status($purchase_id);
} else {
return $response;
}
}

function get_fields_input($cart) {
$data = $GLOBALS['data'];
$fields_input =array();
foreach ($cart['sites'] as $siteId => $site) {
$fields_input[$siteId] =array();
$fields_input[$siteId]['noauthCheckout'] = $data;
foreach ($cart['sites'][$siteId]['add_to_cart'] as $productMD5 => $product) {
$fields_input[$siteId]['addToCart'][$productMD5] = array("quantity" => "1");
}
}
return $fields_input;
}

function purchase_estimates($cart_id, $fields_input) {
$pest = $GLOBALS['TT_PEST'];
$public_token = $GLOBALS['TT_PUBLIC_TOKEN'];
$test_mode = variable_get("twotap_purchase_info_mode");
$response = $pest->post("/v1.0/purchase_estimates?public_token=$public_token&test_mode=$test_mode", array("cart_id" => $cart_id, "fields_input" => $fields_input ));
return $response;	
}

// $cart = add_to_cart();
// if (!$cart) {
// die();
// }
// $fields_input = get_fields_input($cart);
// $purchase = purchase_info($cart['cart_id'], $fields_input);
// if (!$purchase) {
// die();
// }
// $purchase = purchase_confirm($purchase['purchase_id']);
// print($purchase);
?>
