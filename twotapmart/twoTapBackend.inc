<?php
function showOrders(){
	global $user,$base_url;
	if($_POST['deleteOrderId']):
		db_query("DELETE FROM twotap_order_history WHERE order_id='{$_POST['deleteOrderId']}'");
	    drupal_set_message("Order History Deleted successfully !");
	endif;
	$results=db_query("SELECT * FROM twotap_order_history ORDER BY order_id DESC");
	//debug($_SESSION['twoTapCart']);
	$header=array("data_1"=>"Order ID","username"=>"Customer Name","data_2"=>"Transaction ID","items"=>"Items","order_total"=>"Order Total","data_3"=>"Purchased On","status"=>"Status","edit"=>"Edit","delete"=>"Delete");
	$rows=array();
	while($key=db_fetch_object($results)):
	$purchased_on=date("m/d/y h:i A",strtotime($key->purchased_on));
    if($key->status==0)
    $status="Pending";
    if($key->status==1)
    $status="Completed";
    if($key->status==2)
    $status="Still Processing";
    if($key->status==3)
    $status="Cancel";
    $pd=unserialize(base64_decode($key->purchase_details));
    //debug($pd);
    $pd=getOrderDetails($pd);
    $grand_total=number_format($pd['grandTotal'],2);
    $delete="<a href='javascript:void(0)' onclick='deleteOrderHistory($key->order_id)'>Delete</a>";
    $edit="<a href='$base_url/admin/view-order/$key->order_id'>Edit</a>";
    $customer=user_load($key->user_id);
    $rows[]=array("data_1"=>$key->order_id." .","username"=>$customer->name,"data_2"=>$key->purchase_id,"items"=>$pd['productsTaken'],"order_total"=>"$".$grand_total,"data_3"=>$purchased_on,"status"=>$status,"edit"=>$edit,"delete"=>$delete);
	endwhile;
	$css="<style>
	table{
		margin-top:15px;
		font-family:trebuchet ms
	}
		th,td{
			padding:10px;
		}
		th{
			background:steelblue;
			color:white !important;
		}
	h2{
		margin:15px;
	}
    </style>
   <script>
   function deleteOrderHistory(order_id){
	   	conf=window.confirm('Are you sure you want to delete this order history');
	   	if(conf){
	   	$('#deleteOrderId').val(order_id);
	   	document.deleteOrder.submit();
	    }
   }
   </script>

    <form name='deleteOrder' action='' method='post'>
    <input name='deleteOrderId' type='hidden' id='deleteOrderId'>
    </form>
	";
    return theme('table',$header,$rows).$css;
}

function debug($arrayObject){
	echo"<pre>";
	print_r($arrayObject);
	echo"</pre>";
}

function getOrderDetails($cart){
 
 $grand_total=0;
 $pd=array();
  foreach($cart as $key):
  $price=($key['details']->field_discount_price_value && ($key['details']->field_discount_price_value < $key['details']->field_original_price_value))?$key['details']->field_discount_price_value:$key['details']->field_original_price_value;
  $priceMultiple=$price*($key['cart_value']['quantity']);
  $grand_total+=$priceMultiple;
  $price=number_format($price,2);
  $priceMultiple=number_format($priceMultiple,2);
  $pd['productsTaken'].=$key['details']->field_name_value.", ";
  endforeach;

  $pd['grandTotal']=$grand_total;
  $pd['productsTaken']=rtrim($pd['productsTaken'],", ");
  return $pd;
}

function getSingleOrderInfo($order_id){
global $base_url;
$css = <<<EOD
<style>
label{
  padding:10px;
}
.billing_address{
  display:inline-block;
  width:46%;
  margin-left:30px;
  margin-top:30px;
}
#mainContent h1{
    border-bottom: 1px solid lightgrey;
    color: white;
    font-family: trebuchet ms;
    padding: 5px 10px;
    text-shadow:0 0 10px forestgreen;
    background:black;
}
.shipping_address{
  display:inline-block;
  width:46%;
  margin-left:30px;
  margin-top:30px;
}

#mainContent h2{
	margin:20px;
}
label{
	width:100px;
	display:inline-block;
	padding:10px;
	font-family:trebuchet ms;
	font-weight:bold;
}
</style>
EOD;
if($_POST['status']):
db_query("UPDATE twotap_order_history SET status='{$_POST['status']}' WHERE order_id='$order_id'");
drupal_set_message("Order History Status changed successfully !");
endif;
$order="";
$results=db_query("SELECT * FROM twotap_order_history WHERE order_id='$order_id'");
while($rec=db_fetch_object($results)):
$customer=user_load($rec->user_id);
$order.="<hr><a href='$base_url/admin/two-tap-orders' style='float:right;text-decoration:none;color:white;' class='form-submit'>Back</a>";
$order.="<br><form method='post'>";
$order.="<label>ORDER ID</label>: #$rec->order_id"."<br>";
$order.="<label>Customer</label>: $customer->name"."<br>";
$updateImage=$base_url."/".drupal_get_path('module','twotapmart')."/img/update.png";
$update="<input type='image' src='$updateImage' name='submit_cart' value='update_cart' style='top:5px;position:relative'> Update";
$completed=($rec->status==1)?"selected='selected'":"";
$processing=($rec->status==2)?"selected='selected'":"";
$cancelled=($rec->status==3)?"selected='selected'":"";
$order.="<label>Status </label>: <select name='status'><option value='0'>Pending</option><option value='1' $completed>Completed</option><option value='2' $processing>Still Processing</option><option value='3' $cancelled >Cancelled</option></select> $update <br>";
$order.="<label>Two Tap Cart Id </label>: $rec->cart_id"."<br>";
$order.="<label>Purchase Id </label>: $rec->purchase_id"."<br>";
$order.="<label>Purchased On </label>:".date("m-d-Y h:i A",strtotime($rec->purchased_on))."<br>";
$order.="<h1>Cart Information</h1>";
$order.=miniCart(unserialize(base64_decode($rec->purchase_details)));
$pd=unserialize(base64_decode($rec->purchase_details));
$order.="<h1>Address Details</h1>";
$order.="<table style='margin:30px;width:94%'>";
$order.="<tr><th>Billing Address</th><th>Shipping Address</th></tr>";
$order.="<tr><td>";
foreach($pd['billing_address'] as $key=>$value):
$key=str_replace("billing_","",$key);
$key=str_replace("_"," ",$key);
$key=ucfirst($key);
if($value and $key!="Chosen shipping")
$order.="<label>$key</label> : $value<br>";
endforeach;
$order.="</td><td style='vertical-align:top'>";
foreach($pd['shipping_address'] as $key=>$value):
$key=str_replace("shipping_","",$key);
$key=str_replace("_"," ",$key);
$key=ucfirst($key);
if($value and $key!="Chosen shipping")
$order.="<label>$key</label> : $value<br>";
endforeach;
$order.="</td></tr>";
$order.="</table>";
$order.="<h1>Shipping Option</h1>";
$order.="<div style='background:lightyellow;margin:30px;padding:10px;'>{$pd['billing_address']['chosen_shipping']}</div>";
$order.=$css;
$order.="</form>";
$order="<div style='width:94%;margin:auto;font-family:trebuchet ms'>$order</div>";
return $order;
endwhile;
}


function miniCart($cart){
  //debug($cart);
  $smallCart.="<table style='width:94%;font-family:trebuchet ms;margin:30px;font-size:small;box-shadow:0 0 10px;color:grey'>";
  $smallCart.="<tr><th>Description</th><th>Brand</th><th>Quantity</th><th>Price/Qty</th><th>Subtotal</th></tr>";
  foreach($cart as $key):
  $options="";
  $price=($key['details']->field_discount_price_value && ($key['details']->field_discount_price_value < $key['details']->field_original_price_value))?$key['details']->field_discount_price_value:$key['details']->field_original_price_value;
  $priceMultiple=$price*($key['cart_value']['quantity']);
  $grand_total+=$priceMultiple;
  $price=number_format($price,2);
  $priceMultiple=number_format($priceMultiple,2);
  foreach($key['cart_value'] as $k=>$value):
  $k=ucfirst($k);
  if($k!="Quantity")
  $options.="<b>$k</b>"." : ".$value."<br>";
  endforeach;
  if($price>0)
  $smallCart.="<tr><td>{$key['details']->field_name_value}<br>$options</td><td>{$key['details']->brand}</td><td style='padding-left:20px'>{$key['cart_value']['quantity']}</td><td>$$price</td><td>$$priceMultiple</td></tr>";
  endforeach;
  $grand_total=number_format($grand_total,2);
  $smallCart.="<tr style='background:steelblue;color:white;'><td colspan='4' align='right'>Grand Total :</td><td>$$grand_total</td></tr>";
  $smallCart.="</table>";
  $smallCart.="
<style>
th,td{
  padding:10px;
}
th{
  background:steelblue;
  color:white;
}
tr:nth-child(even) {background:ivory}
tr:nth-child(odd) {background:lightyellow}
</style>
  ";
return $smallCart;
}


function twoTapSettings(){
  $css= <<< EOD
<style>
h2{
  margin:20px;
}
label{
  width:150px;
  display:inline-block;
  padding:10px;
}
</style>
EOD;
if($_POST['twotap_setting_submit']):
  //debug($_POST);
  variable_set('twotap_add_to_cart_mode',$_POST['twotap_add_to_cart_mode']);
  variable_set('twotap_add_to_cart_status_mode',$_POST['twotap_add_to_cart_status_mode']);
  variable_set('twotap_purchase_info_mode',$_POST['twotap_purchase_info_mode']);
  variable_set('twotap_purchase_info_status_mode',$_POST['twotap_purchase_info_status_mode']);
  variable_set('twotap_purchase_confirm_mode',$_POST['twotap_purchase_confirm_mode']);
  variable_set('twotap_purchase_confirm_status_mode',$_POST['twotap_purchase_confirm_status_mode']);
  variable_set('twotap_email',$_POST['twotap_email']);
  variable_set('twotap_password',$_POST['twotap_password']);

  drupal_set_message("Two Tap Settings are saved successfully !");
endif;

$mode_1_1=(variable_get("twotap_add_to_cart_mode")=="dummy_data")?"selected='selected'":"";
$mode_1_2=(variable_get("twotap_add_to_cart_mode")=="fake_confirm")?"selected='selected'":"";
$mode_1_3=(variable_get("twotap_add_to_cart_mode")=="none")?"selected='selected'":"";

$mode_2_1=(variable_get("twotap_add_to_cart_status_mode")=="dummy_data")?"selected='selected'":"";
$mode_2_2=(variable_get("twotap_add_to_cart_status_mode")=="fake_confirm")?"selected='selected'":"";
$mode_2_3=(variable_get("twotap_add_to_cart_status_mode")=="none")?"selected='selected'":"";

$mode_3_1=(variable_get("twotap_purchase_info_mode")=="dummy_data")?"selected='selected'":"";
$mode_3_2=(variable_get("twotap_purchase_info_mode")=="fake_confirm")?"selected='selected'":"";
$mode_3_3=(variable_get("twotap_purchase_info_mode")=="none")?"selected='selected'":"";

$mode_4_1=(variable_get("twotap_purchase_info_status_mode")=="dummy_data")?"selected='selected'":"";
$mode_4_2=(variable_get("twotap_purchase_info_status_mode")=="fake_confirm")?"selected='selected'":"";
$mode_4_3=(variable_get("twotap_purchase_info_status_mode")=="none")?"selected='selected'":"";

$mode_5_1=(variable_get("twotap_purchase_confirm_mode")=="dummy_data")?"selected='selected'":"";
$mode_5_2=(variable_get("twotap_purchase_confirm_mode")=="fake_confirm")?"selected='selected'":"";
$mode_5_3=(variable_get("twotap_purchase_confirm_mode")=="none")?"selected='selected'":"";

$mode_6_1=(variable_get("twotap_purchase_confirm_status_mode")=="dummy_data")?"selected='selected'":"";
$mode_6_2=(variable_get("twotap_purchase_confirm_status_mode")=="fake_confirm")?"selected='selected'":"";
$mode_6_3=(variable_get("twotap_purchase_confirm_status_mode")=="none")?"selected='selected'":"";

$twotap_email=variable_get("twotap_email");
$twotap_password=variable_get("twotap_password");

$html="";
$html.="<form method='post' action=''>";
$html.="<label>Add To Cart </label>";
$html.="<select name='twotap_add_to_cart_mode'><option value='dummy_data' $mode_1_1 >Dummy Data</option><option value='fake_confirm' $mode_1_2 >Fake Confirm</option><option value='none' $mode_1_3>Live</option></select><br>";

$html.="<label>Add To Cart Status</label>";
$html.="<select name='twotap_add_to_cart_status_mode'><option value='dummy_data' $mode_2_1 >Dummy Data</option><option value='fake_confirm' $mode_2_2 >Fake Confirm</option><option value='none' $mode_2_3>Live</option></select><br>";

$html.="<label>Purchase Info</label>";
$html.="<select name='twotap_purchase_info_mode'><option value='dummy_data' $mode_3_1 >Dummy Data</option><option value='fake_confirm' $mode_3_2 >Fake Confirm</option><option value='none' $mode_3_3 >Live</option></select><br>";

$html.="<label>Purchase Info Status</label>";
$html.="<select name='twotap_purchase_info_status_mode'><option value='dummy_data' $mode_4_1 >Dummy Data</option><option value='fake_confirm' $mode_4_2 >Fake Confirm</option><option value='none' $mode_4_3>Live</option></select><br>";

$html.="<label>Purchase Confirm</label>";
$html.="<select name='twotap_purchase_confirm_mode'><option value='dummy_data' $mode_5_1 >Dummy Data</option><option value='fake_confirm' $mode_5_2 >Fake Confirm</option><option value='none' $mode_5_3>Live</option></select><br>";

$html.="<label>Purchase Confirm Status</label>";
$html.="<select name='twotap_purchase_confirm_status_mode'><option value='dummy_data' $mode_6_1 >Dummy Data</option><option value='fake_confirm' $mode_6_2 >Fake Confirm</option><option value='none' $mode_6_3>Live</option></select><br>";

$html.="<label>Two Tap Email Id</label>";
$html.="<input type='text' name='twotap_email' value='$twotap_email'><br>";

$html.="<label>Two Tap Password</label>";
$html.="<input type='text' name='twotap_password' value='$twotap_password'><br>";

$html.="<input type='submit' name='twotap_setting_submit' value='SUBMIT' class='form-submit'>";

$html.="</form>";
$html="<hr><div style='width:94%;margin:auto;padding-top:10px;font-family:trebuchet ms'>$html</div>";
$html.=ini_get("upload_tmp_dir");
return $html.$css;
}

?>