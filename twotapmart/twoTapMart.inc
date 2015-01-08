<?php
function getSingleProductInfo($nodeId){
    global $base_url;
    $result=db_query("SELECT * FROM content_type_product as c LEFT JOIN files as f ON c.field_product_image_fid=f.fid WHERE c.nid='$nodeId'");
    while($rec=db_fetch_object($result)):
        $rec->brand=  getTermData($rec->field_brand_value);
        $rec->store= getTermData($rec->field_store_value);
        $product=$rec;
        break;
      endwhile;
//    echo"<pre>";
//    print_r($product);
//    echo"</pre>";
     //Create a HTML page for the 
    $productImage=$base_url."/".$rec->filepath;
    $html="";
    $html.="<div class='productInfoDiv'>";
    $html.="<div class='productPic'><img src='$productImage'></div>";
    $html.="<div class='productDetails'><big>$rec->field_name_value</big><br>";
    $price=($product->field_discount_price_value && ($product->field_discount_price_value < $product->field_original_price_value))?$rec->field_discount_price_value:$rec->field_original_price_value;
    $price=number_format($price,2);
    $strikePrice=($product->field_discount_price_value && ($product->field_discount_price_value < $product->field_original_price_value))?"<br><strike>$".number_format($product->field_original_price_value,2)."</strike><br>":"";
    $shipping="Ships from <font color='steelblue'>".$product->store."</font>";
    $html.="<big style='color:red'>$$price</big><span class='shippign_text'>$shipping</span>";  
    $html.=$strikePrice;
    $html.="<span class='productDescription'>".$product->field_details_value."</span><br><br>";
    $html.="<font color='red'>You will be able to choose color and size at checkout.</font>";
    // $productAvailable=checkProductAvailability($product->field_product_url_url);
    // $inStock="<span style='color:green' class='stockText'> <b style='border-radius: 10px; padding: 2px 4px; background: none repeat scroll 0% 0% lightgreen;'>&#x2714;</b> In Stock</span>";
    // $outOfStock="<span style='color:tomato' class='stockText'> <b style='border-radius: 10px; padding: 2px 4px; background: none repeat scroll 0% 0% pink;'>&#x2718;</b> Out of Stock</span>";
    // if($productAvailable)
    // $html.="<a href='javascript:void(0)' class='addToBag' onclick='addToCart($nodeId)'>Add To Bag</a> $inStock ";
    // else
    // $html.="<a href='javascript:void(0)' class='addToBagNotAvailable'>Not Available</a> $outOfStock";
    $html.="<a href='javascript:void(0)' class='addToBag' onclick='addToCart($nodeId)'>Add To Bag</a>";
    $html.="</div></div>";
    addTwoTapCss();
    addTwoTapJavascript();
    $html.=showCartOverLay();
    return $html;
}

function getTermData($tid){
    $result=db_query("SELECT * FROM term_data WHERE tid='$tid'");
    while($rec=db_fetch_object($result)):
        return $rec->name;
    endwhile;
    return false;
}

function cartBuilder(){
    global $base_url;
   if($_REQUEST['task']=="add_to_cart"):
    //First check Whether the product is available or not, then keep the product inside the twotap collector.
       $productAvailable=checkProductAvailability($_REQUEST['productId']);
       if(!$productAvailable){
         echo"<div style='text-align:center;line-height:22px;'>";
         echo"<font color='red' size='2'><span style='color:tomato'> <b style='border-radius: 10px; padding: 2px 4px; background: none repeat scroll 0% 0% pink;'>&#x2718;</b></span> Sorry, The product is out of stock ! </font><br>";
         echo"<a href='$base_url/products'>Continue Shopping</a> | <a href='$base_url/cart' onclick='loadingTheCart()'>Go to Cart</a>";
         echo"</div>";
         exit;
       }
       $_SESSION['twotap_collector']=$_SESSION['twotap_collector']."|".$_REQUEST['productId'];
   endif;

   if($_REQUEST['task']=="remove"):
    $_SESSION['twotap_collector']=str_replace($_REQUEST['productId'],"",$_SESSION['twotap_collector']);
    $_SESSION['twotap_collector']=str_replace("||","|",$_SESSION['twotap_collector']);
    
   endif;

    //Need to Do Some more Work, this will show the old cart values.
    foreach($_SESSION['twoTapCart'] as $key):
    if(sizeof($key['cart_value'])):
    $_SESSION['twoTapbackUpAttribs'][$key['details']->field_product_url_url]=$key['cart_value'];
    endif;
    //Have to remove the backup Attrib as well
    if($_REQUEST['deleteProductUrl'] and $_REQUEST['task']=="remove"):
    $outerProductUrl=base64_decode($_REQUEST['deleteProductUrl']); // this is the product from where the client is showing the product.
    unset($_SESSION['twoTapbackUpAttribs'][$outerProductUrl]);
    endif;
    endforeach;


//The funda here is to save the product 0,1 and so on, there no number discontinuity, everytime it starts with 0,1 and so on.
   $_SESSION['twotap_collector']=str_replace("||",'|',$_SESSION['twotap_collector']);
   $_SESSION['twotap_collector']=ltrim($_SESSION['twotap_collector'],"|");
   $cart=rtrim($_SESSION['twotap_collector'],"|");
   $cart=explode("|",$cart);
   $cart=array_unique($cart);
   $_SESSION['twoTapCart']=array();
   $i=0;
   foreach($cart as $key=>$value):
       $_SESSION['twoTapCart'][$i]['productId']=$value;
       $_SESSION['twoTapCart'][$i]['details']=getProductDetails($value);
       $_SESSION['twoTapCart'][$i]['cart_value']=$_SESSION['twoTapbackUpAttribs'][$_SESSION['twoTapCart'][$i]['details']->field_product_url_url];
       $i++;
   endforeach;
//   echo"<pre>";
//   print_r($_SESSION['twoTapCart']);
//   echo"</pre>";
   
   
   //Now sort the cart based on store;
   $stores=array();
   $i=0;
   foreach($_SESSION['twoTapCart'] as $key):
   $stores[$i]=$key['details']->store;
   $i++;
   endforeach;
   $stores=  array_unique($stores);
   
   $i=0;
   foreach($_SESSION['twoTapCart'] as $key):
   $stores[$key['details']->store][]=$key;
   unset($stores[$i]);
   $i++;
   endforeach;   
   // $cart_id=twoTafrfpAddToCart();
   $_SESSION['twoTapCartSorted']=$stores;
   echo"<div style='text-align:center;line-height:30px;'>";
   echo"<font color='green' size='2'><span style='color:green'> <b style='border-radius: 10px; padding: 2px 4px; background: none repeat scroll 0% 0% lightgreen;'>&#x2714;</b></span>  Product Added Successfully !</font><br>";
   echo"<a href='$base_url/products'>Continue Shopping</a> | <a href='javascript:void(0)' onclick='loadingTheCart()'>Go to Cart</a>";
   echo"</div>";
}

//This will sort the based on the store.
function cartStoreSorting(){

   $stores=array();
   $i=0;
   foreach($_SESSION['twoTapCart'] as $key):
   $stores[$i]=$key['details']->store;
   $i++;
   endforeach;
   $i=0;
   foreach($_SESSION['twoTapCart'] as $key):
   //$_SESSION['twoTapCart']=array_filter($_SESSION['twoTapCart']);
   $stores[$key['details']->store][]=$key;
   unset($stores[$i]);
   $i++;
   endforeach;   
   // $cart_id=twoTafrfpAddToCart();
   unset($stores[""]);
   $_SESSION['twoTapCartSorted']=$stores;
   //debug($_SESSION['twoTapCartSorted']);
   //Remove empty entries.

}
function getProductDetails($nodeId){
    global $base_url;
    $result=db_query("SELECT * FROM content_type_product as c LEFT JOIN files as f ON c.field_product_image_fid=f.fid WHERE c.nid='$nodeId'");
    while($rec=db_fetch_object($result)):
        $rec->brand=  getTermData($rec->field_brand_value);
        $rec->store= getTermData($rec->field_store_value);
        $product=$rec;
        break;
      endwhile;
   return $product;
}

function showCartOverLay(){
    global $base_url;
    $ajaxLoader=$base_url."/".drupal_get_path('module','twotapmart')."/img/ajax-loader.gif";
    $html="<div id='darkOverlay'></div><a href='javascript:void(0)' onclick='disperseCart()' class='cancelButton'>X</a><div id='cartDiv'><img src='$ajaxLoader' id='ajax_loader' style='display:none;left:48%;top:48%;position:relative'></div>";
    return $html;
}

function addTwoTapJavascript(){
    global $base_url;
    $ajaxLoader=$base_url."/".drupal_get_path('module','twotapmart')."/img/ajax-loader.gif";
    $javascript="
            
            <script>
            
            var ajaxLoader='$ajaxLoader';
                
       
       
            function addToCart(nid){
            
               $('#darkOverlay').fadeIn();
                $('#cartDiv').fadeIn();
                $('#ajax_loader').show();
                $('.cancelButton').show();
                
            $.ajax({
                        url: '{$base_url}/add-to-cart',
                        data:{'productId':nid,'task':'add_to_cart'},
                        type: 'post',
                        cache: false,
                        dataType: 'text',
                        success:function(response){
                        $('#cartDiv').html(response);
                        }
                        
            });   
            
            }
            
       function disperseCart(){
                $('#darkOverlay').fadeOut();
                $('#cartDiv').fadeOut();
                $('#cartDiv').css('text-align','center').html('<img src=\'$ajaxLoader\' id=\'ajax_loader\' style=\'display:none;top:48%;position:relative\'>');
                $('.cancelButton').hide();     
       } 

       function loadingTheCart(){
                $('#darkOverlay').fadeIn();
                $('#cartDiv').fadeIn();
                $('#cartDiv').css('text-align','center').html('<br>Please wait...<br><br><img src=\'$ajaxLoader\' id=\'ajax_loader\' style=\'display:none;top:45%;position:relative\'>');
                $('#ajax_loader').show();
                $('.cancelButton').show();
                window.location='$base_url/cart';
       }

            </script>";
    
    echo $javascript;
}
            
function addTwoTapCss(){
    $css="
   <style>
   .productPic img{
   height:288px;
   width:192px;
   box-shadow:0 0 10px grey;
   }
   
.productPic{
text-align:center;
width:25%;
display:inline-block;
vertical-align:top;
}
   
   .productInfoDiv{
   margin:20px;
   }
   
.productDetails{
width:70%;
display:inline-block;
vertical-align:top;
}

.productDetails big{
font-size:24px;
font-weight:bold;
}

.shippign_text{
color:grey;
margin-left:160px;
display:inline-block;
font-weight:bold;
}

strike{
font-size:16px;
font-weight:bold;
color:grey;
}

.productDescription{
padding-top:10px;
display:inline-block;
line-height:18px;
}

.productInfoDiv{
}

.addToBag{
background:black;
padding:8px 15px;
color:white !important;
text-decoration:none !important;
float:right;
margin-top:50px;
cursor:pointer;
}

.stockText{
padding:8px 15px;
text-decoration:none !important;
float:right;
margin-top:50px;
cursor:pointer;
}

.addToBagNotAvailable{
background:lightgrey;
padding:8px 15px;
color:white !important;
text-decoration:none !important;
float:right;
margin-top:50px;
cursor:pointer;
}

#darkOverlay{
    background: none repeat scroll 0 0 black;
    //background:white;
    display: none;
    height: 100%;
    opacity: 0.7;
    position: absolute;
    top: 0;
    left:0;
    width: 100%;
    z-index:501;
    position:fixed;
}
#cartDiv{
    background: none repeat scroll 0 0 white;
    border: 15px solid black;
    border-radius: 20px;
    display: none;
    height: 50px;
    left: 39%;
    line-height: 10px;
    padding: 10px 30px 30px;
    position: fixed;
    top: 35%;
    width: 225px;
    z-index: 501;
    font-family:lucida sans unicode;
}  

.cancelButton{
background: none repeat scroll 0 0 white;
    border-radius: 33px;
    color: black;
    font-size: 20px;
    left: 61%;
    padding: 3px 9px;
    position: fixed;
    top: 33%;
    z-index: 100000;
    display:none;
    text-decoration:none !important;

}

@media screen and (-webkit-min-device-pixel-ratio:0) { 
 .cancelButton{left:63% !important;}
}


   </style>";
    echo $css;
}

function getProductStatus($array,$md5){

  foreach($array['sites'] as $key):
  if(sizeof($key['add_to_cart'][$md5]))
  return $key['add_to_cart'][$md5];
  endforeach;

}
//public_token=$public_token&test_mode=$test_mode", [ "products" => [ $product_url ] ]

function destroyCart(){
 unset($_SESSION['twoTapCart']);
 unset($_SESSION['twoTapCartSorted']);
}


function checkProductAvailablityOnCartPage($statusArray){
    
    $errorMessageCollector=array();
    foreach($statusArray['sites'] as $key):
    if(sizeof($key['failed_to_add_to_cart'])):
       foreach($key['failed_to_add_to_cart'] as $failKey):
        $errorMessageCollector[]=$failKey['title'];
       endforeach;
    endif;
    endforeach;
       
    if(sizeof($errorMessageCollector)):
    $_SESSION['twoTapCart']['allProductsAvailable']=false;
       drupal_set_message('The product(s) '.implode(", ",$errorMessageCollector)." are out of stock, please remove them from your cart to proceed further!",'error');
    endif;
    
    if(!sizeof($errorMessageCollector)):
    $_SESSION['twoTapCart']['allProductsAvailable']=true;
    endif;
    
    return $_SESSION['twoTapCart']['allProductsAvailable'];
    
}

function cartPage(){
    //destroyCart();
    
  //This will show the errors will the twotap having any errors.
  

   //create the products url.
    $cart_id=twoTapAddToCart();
    $_SESSION['twotap_latest_cart_id']=$cart_id;
    $addToCartProductStatus=twoTapAddToCartStatus($cart_id);
    ##CLient requirement, again check the availablity
    checkProductAvailablityOnCartPage($addToCartProductStatus);
    ###Client Requirement, check availability ends here.
    $i=0;
    foreach($_SESSION['twoTapCart'] as $key):
        $products[$i]=$key['details']->field_product_url_url;
        $md5=md5($key['details']->field_product_url_url);
        $_SESSION['twoTapCart'][$i]['product_status']=getProductStatus($addToCartProductStatus,$md5);
        if(!$_SESSION['twoTapCart'][$i]['productId'])
        unset($_SESSION['twoTapCart'][$i]);
    $i++;
    endforeach;
    //debug($_SESSION['twoTapCart']);
    $checkout=checkoutPageBuilder();
    return $checkout;
}

function checkoutPageBuilder($apiData=""){ 
    global $base_url;
    $html="";

    if(isset($_POST['submit_cart']) or isset($_POST['submit_cart_x']) or isset($_POST['submit_cart_y'])):
    $count=0;
    foreach($_SESSION['twoTapCart'] as $key):
    $required_field=$key['product_status']['required_field_names'];
    foreach($required_field as $k=>$v):
    $indexKey=$key['productId']."_".$v;
    $_SESSION['twoTapCart'][$count]['cart_value'][$v]=$_POST[$indexKey];
    endforeach;
    $qty=$key['productId']."_quantity";
    $_SESSION['twoTapCart'][$count]['cart_value']['quantity']=$_POST[$qty];
    $count++;
    endforeach;
    $_SESSION['grandTotal']=$_POST['grandTotal'];

    if($_POST['submit_cart']=="Checkout"){
    header("Location:$base_url"."/checkout");
    }
    endif;

    if($_SESSION['twoTapCart']['twotap_bad_fields']):
      drupal_set_message($_SESSION['twoTapCart']['twotap_bad_fields'],"error");
     unset($_SESSION['twoTapCart']['twotap_bad_fields']);
  endif; //
    //debug($_SESSION);
    cartStoreSorting();
    $stores=array_keys($_SESSION['twoTapCartSorted']);
    $cart=$_SESSION['twoTapCartSorted']; //this is the array sorted based on the store.

    
    
    $html="<form method='post' action='' onsubmit='return pleaseWaitOverlay()'>";
    $html.="<span id='cart_upper_links' style='float: right; position: relative; right: 10px; bottom: 25px;'><a href=''>Refresh</a></span>";
    $html.="<table id='twoTapCart' cellspacing='0' cellpadding='0' style='width:100%;font-family:lucida sans unicode;font-size:small'>";
    $html.="<tr style='color:white;background:black;height:40px;'><th style='padding-left:20px'>Item</th><th>Brand</th><th>Product Name</th><th>Price</th><th>Qty</th><th>Action</th><th>Subtotal</th></tr>";
    $grandTotal=0;
    foreach($stores as $key=>$value):
        $html.="<tr><td colspan='7' style='padding:10px 15px;background:black;color:white'>Shipped from $value</td></tr>";
        for($i=0;$i<sizeof($cart[$value]);$i++):
        $productImagePath=$base_url."/".$cart[$value][$i]['details']->filepath;
        $price=(($cart[$value][$i]['details']->field_original_price_value > $cart[$value][$i]['details']->field_discount_price_value ) && $cart[$value][$i]['details']->field_discount_price_value)?$cart[$value][$i]['details']->field_discount_price_value:$cart[$value][$i]['details']->field_original_price_value;
        $price=number_format($price,2);
        $productId=$cart[$value][$i]['productId'];
        $productLink=$base_url."/product/".$productId;
        $productUrlBase64=str_replace("=","",base64_encode($cart[$value][$i]['details']->field_product_url_url)); //this is for the product removal from the cart.
        $productImg="<a href='$productLink'><img src='$productImagePath' style='width:100px;height:150px'></a>";
        $html.="<tr><td style='padding:10px'>$productImg</td>";
        $html.="<td>{$cart[$value][$i]['details']->brand}</td>";
            $available_options="";
            foreach($cart[$value][$i]['product_status']['required_field_names'] as $key=>$val):
              if(sizeof($cart[$value][$i]['product_status']['required_field_values'][$val])):
              $available_options.="<b>".ucfirst($val)."</b>";
              $attributeName=$productId."_".$val;
              $dep=base64_encode(serialize($cart[$value][$i]['product_status']['required_field_values'][$val]));
              $available_options.=" <select required='required' name='$attributeName' onchange=\"getDepthProducts(this.value,'$productId','$dep','$attributeName')\">";
              $available_options.="<option value=''>-SELECT-</option>";
              $Attribcounter=0;  
                  foreach($cart[$value][$i]['product_status']['required_field_values'][$val] as $innerKey):
                  $optionVal=$innerKey['value'];
                  $optionText=$innerKey['text'];
                  $optionValSelected=($cart[$value][$i]['cart_value'][$val]==$optionVal)?"selected='selected'":"";
                  $available_options.="<option value='$optionVal' $optionValSelected>$optionText</option>";

                  if($optionValSelected):
                  $depth=sizeof($innerKey['dep']);
                  $depthArray=$innerKey['dep'];  
                  endif;
                  //Select the depth Option for the first Item.
                  $depthArray=$innerKey['dep']; //Just wrote this, coz i was getting a bug in depth dropdown.
                  if(!$optionValSelected and !$Attribcounter==0):
                  $depth=sizeof($innerKey['dep']);
                  $depthArray=$innerKey['dep'];
                  endif;

              $Attribcounter++;
                  endforeach;


              if($depth)
              {
              
              $depthSelect="";
              foreach($depthArray as $depthKey=>$depthValue):
              break;
              endforeach;
              foreach($depthValue as $dk):
                $text=$dk['text']."-".$dk['price'];
                $optVal=$dk['value'];
                $optionValSelected=($cart[$value][$i]['cart_value'][$depthKey]==$optVal)?"selected='selected'":"";
                $depthSelect.="<option value='$optVal' $optionValSelected>$text</option>";
                $depthCounter++;
              endforeach;
<<<<<<< HEAD
              $depthSelect=" <b>".ucfirst($depthKey)."</b> <select name='$productId"."_$depthKey' required='required'><option value=''>-SELECT-</option>$depthSelect</select>";
=======
              $depthSelect=" <b>".ucfirst($depthKey)."</b> <select name='$productId"."_$depthKey'><option value=''>-SELECT-</option>$depthSelect</select>";
>>>>>>> origin/master
              }
              $subOption=($depth)?"<span id='$attributeName'>$depthSelect</span>":"<span id='$attributeName'></span>";
              $available_options.="</select><br>$subOption";
              endif;
            endforeach;

        $html.="<td>{$cart[$value][$i]['details']->field_name_value} <br>$available_options</td>";
        $html.="<td>$$price</td>";
        $qtyName=$productId."_quantity";
        $updateImage=$base_url."/".drupal_get_path('module','twotapmart')."/img/update.png";
        $update="<input type='image' src='$updateImage' name='submit_cart' value='update_cart' style='top:5px;position:relative'> Update";
        $qtySelected=($cart[$value][$i]['cart_value']['quantity'])?$cart[$value][$i]['cart_value']['quantity']:1;
        $priceMultiple=number_format(($qtySelected*$price),2);
        $grandTotal+=($qtySelected*$price);
        $html.="<td><input type='number' min='1'  style='width:50px;' value='$qtySelected' name='$qtyName' onclick='changeQtyPrice($productId,this.value,$price)'></td>";
        $html.="<td><input id='$productUrlBase64' type='checkbox' onchange=\"removeItem(this)\" value='$productId' >Remove <br> $update</td>";
        $html.="<td id='subtotal_$productId' class='indi_subtotal'>$$priceMultiple</td></tr>";
        endfor;
    endforeach;
    if(!sizeof($stores)){
       $html.="<tr><td style='padding:15px;'><font color='red'>Sorry, You have not selected any products so far, your cart is empty</font></td></tr>";
    }
    else{
    $html.="<input type='hidden' name='grand_total' value='$grandTotal'>";
    $grandTotal=number_format($grandTotal,2);
    $html.="<tr style='background:wheat'><td colspan='6' style='padding:15px;text-align:right'>Grand Total</td><td id='grand_total'>$$grandTotal</td></tr>";
    $actionButton=($_SESSION['twoTapCart']['allProductsAvailable'])?"type='submit'":"type='button' onclick=\"alert('Please remove the products which are out of stock from the cart!');\"";
    $html.="<tr><td colspan='9' style='padding:15px;text-align:right'><a style='background: none repeat scroll 0% 0% steelblue;text-decoration:none;color: white; padding: 10px 25px; cursor: pointer; border: 1px none; box-shadow: 0px 0px 10px grey; border-radius: 76px 5px;display:inline-block;float:left;position:relative;' href='$base_url/products'>Continue Shopping</a><input $actionButton name='submit_cart' value='Checkout' style='background: none repeat scroll 0% 0% steelblue; color: white; padding: 10px 25px; cursor: pointer; border: 1px none; box-shadow: 0px 0px 10px grey; border-radius: 76px 5px;'></td></tr>";
    }
    $html.="</form>";
    $html.="</table>";

    //Need to add a blackoverlay Client's requirement.
    $html.=showCartOverLay();

    
    //We have to Create to new Cart were we can sort the whole cart.
    $html.=checkOutCss();
    $html.=checkOutJS();

    return $html;
}


function getDepthProducts(){
  $depthisThere=0;
  $productId=$_POST['productId'];
  $currentVal=$_POST['curr_val'];
  $html="";
  $depArray=unserialize(base64_decode($_POST['dep']));
  //debug($depArray);
  
  foreach($depArray as $key):
    if($currentVal==$key['value']):
    foreach($key['dep'] as $innerKey=>$innerValue):
    $depthisThere=1;
    break;
    endforeach;

    foreach($key['dep'][$innerKey] as $k=>$v):
    $text=$v['text']."-".$v['price'];
    $optVal=$v['value'];
    $html.="<option value='$optVal'>$text</option>";
    endforeach;

<<<<<<< HEAD
    $html=($depthisThere)?" <b>".ucfirst($innerKey)."</b> <select name='$productId"."_$innerKey' required='required'><option value=''>-SELECT-</option>$html</select>":"";
=======
    $html=($depthisThere)?" <b>".ucfirst($innerKey)."</b> <select name='$productId"."_$innerKey'><option value=''>-SELECT-</option>$html</select>":"";
>>>>>>> origin/master
    echo $html;
    exit;
    endif;
  endforeach;
}

function checkOutJS(){
  global $base_url;
    $str = <<<EOD
<script>
            function changeQtyPrice(pid,val,price){
            calc=parseFloat(val)*parseFloat(price);
            calc=parseFloat(calc).toFixed(2);
            document.getElementById('subtotal_'+pid).innerHTML="$"+calc;

            //Need to change the grandTotal As well.
            var grand_total=parseFloat(0);
            $(".indi_subtotal").each(function(){
            indi_subtotal=$(this).html();
            indi_subtotal=indi_subtotal.replace("$","");
            indi_subtotal=indi_subtotal.replace(",","");
            grand_total+=parseFloat(indi_subtotal)
            });
            grand_total=parseFloat(grand_total).toFixed(2);
            $("#grand_total").html("$"+grand_total);
            $("[name='grand_total']").val(grand_total);
            }
           
            function getDepthProducts(curr_val,productId,dep,innerHTMLId){
             
             $.ajax({
                        url: 'ajax-dep-products',
                        data:{'productId':productId,'dep':dep,'curr_val':curr_val},
                        type: 'post',
                        async:false,
                        cache:false,
                        dataType: 'text',
                        success:function(response){
                        $('#'+innerHTMLId).html(response);
                        }
                      });

            }

            function removeItem(e){
              if(e.checked){
              conf=window.confirm("Are you sure you want to remove this item from cart ?");
              if(conf){
                pleaseWaitOverlay();
                $.ajax({
                        url: 'add-to-cart',
                        data:{'productId':e.value,'deleteProductUrl':e.id,'task':'remove'},
                        type: 'post',
                        cache:false,
                        async:false,
                        dataType: 'text',
                        success:function(response){
                         location='';
                        }
                      });
                      }
              else{
                e.checked=false;
              }
            }
          }



          function pleaseWaitOverlay(){

            $('#darkOverlay').fadeIn();
            $('#cartDiv').fadeIn();
            $("#ajax_loader").css('top','8%').show();
            $('#cartDiv').append('<br><br><br>Processing your request, Please wait...');
            $('.cancelButton').hide(); 
            return true;
          }

           function disperseCart(){
                $('#darkOverlay').fadeOut();
                $('#cartDiv').fadeOut();
                $('#cartDiv').css('text-align','center').html('<img src=\'$ajaxLoader\' id=\'ajax_loader\' style=\'display:none;top:48%;position:relative\'>');
                $('.cancelButton').hide();     
       } 
</script>
EOD;
    
    return $str;
}

function confirmCheckoutPage(){

  //Now Need to send a new request.
   debug($REQUEST);
  $getShippingForm=userInfoTable();
  return $getShippingForm;
}

function getShippingOptions(){
  $cart_id=$_SESSION['twotap_latest_cart_id'];
  $base_path=getcwd();
  $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
  include_once($sdk);
  $response=add_to_cart_status($cart_id);
  $shipping_options=array();
  foreach($response['sites'] as $siteId):
    $shipping_options[]=$siteId['shipping_options'];
  endforeach;
  $shipping_options=array_unique($shipping_options);
  // $sw=file_get_contents("http://api.twotap.com/v1.0/supported_sites");
  // $sw=json_decode($sw);
  // debug($sw);
  return $shipping_options;
}

//We have to fetch the shipping address from the database and store it inside the session, so that user don't have to type it again n again.
function getShippingAddressDetails(){
  global $user;
  $shippingEqualsBilling=true;
  $query=db_query("SELECT * FROM twotap_user_shipping_details WHERE user_id='{$user->uid}'");

  if(!db_result($query)):
  $insert="INSERT INTO twotap_user_shipping_details SET user_id='{$user->uid}'";
  db_query($insert);
  endif;
 
  $query=db_query("SELECT * FROM twotap_user_shipping_details WHERE user_id='{$user->uid}'");
  while($rec=db_fetch_object($query)):
  if($rec->shipping_address and $rec->billing_address):
  $_SESSION['twoTapCart']['billing_address']=unserialize(base64_decode($rec->billing_address));
  $_SESSION['twoTapCart']['shipping_address']=unserialize(base64_decode($rec->shipping_address));
  break;
  endif;
  endwhile;


  return $shippingEqualsBilling;
}

function userInfoTable(){
  global $base_url,$user;
  unset($_SESSION['twoTapCart']['price_estimate']);
<<<<<<< HEAD
    
  if(!$_SESSION['twoTapCart']['allProductsAvailable']):
    header("Location:$base_url/cart");
    exit;
  endif;
    
=======
>>>>>>> origin/master
//debug($_POST);
//debug($_SESSION['twoTapCart']);


$inputs=array("email","title","first_name","last_name","address","zip","telephone","city");
if($_POST['submit_address']):
  foreach($inputs as $key=>$value):
  $shipping_post="shipping_".$value;
  $billing_post="billing_".$value;
  $_SESSION['twoTapCart']['shipping_address'][$shipping_post]=$_POST[$shipping_post];
  $_SESSION['twoTapCart']['billing_address'][$billing_post]=$_POST[$billing_post];
  endforeach;
  $_SESSION['twoTapCart']['billing_address']['billing_country']=$_POST['billing_country'];
  $_SESSION['twoTapCart']['shipping_address']['shipping_country']=$_POST['shipping_country'];
  $_SESSION['twoTapCart']['billing_address']['billing_state']=$_POST['billing_state'];
  $_SESSION['twoTapCart']['shipping_address']['shipping_state']=$_POST['shipping_state'];

  $_SESSION['twoTapCart']['card_details']['card_name']=$_POST['card_name'];
  $_SESSION['twoTapCart']['card_details']['card_number']=$_POST['card_number'];
  $_SESSION['twoTapCart']['card_details']['expiry_date_year']=$_POST['expiry_date_year'];
  $_SESSION['twoTapCart']['card_details']['expiry_date_month']=$_POST['expiry_date_month'];
  $_SESSION['twoTapCart']['card_details']['cvv']=$_POST['cvv'];
  $_SESSION['twoTapCart']['card_details']['card_type']=$_POST['card_type'];
  $_SESSION['twoTapCart']['billing_address']['chosen_shipping']=$_POST['chosen_shipping'];
  $_SESSION['twoTapCart']['shipping_address']['chosen_shipping']=$_POST['chosen_shipping'];
  

  //Store the values of shipping address and billing address into the database.
  $sa=base64_encode(serialize($_SESSION['twoTapCart']['shipping_address']));
  $ba=base64_encode(serialize($_SESSION['twoTapCart']['billing_address']));
  $update=db_query("UPDATE twotap_user_shipping_details SET billing_address='$ba',shipping_address='$sa' WHERE user_id='{$user->uid}'");

  if($_POST['submit_address']=="Go Checkout"):
  BUY_PRODUCTS();
  endif;

  if(!$_SESSION['twotap_bad_fields'] and $_POST['submit_address']=="Go Checkout"){
  header("Location:"."$base_url"."/confirm-purchase");
  }
  else{
  drupal_set_message($_SESSION['twotap_bad_fields'],"error");
  }
  
  endif;

  getShippingAddressDetails();
  
  $purchase_estimates=getPurchaseEstimates();
$purchaseEstimateInformation="";
if($purchase_estimates['message']=="done"):
$purchaseEstimateInformation="<br><h1 style='width:92%;margin:auto'>Purchase Estimation</h1><br>";
foreach($purchase_estimates['estimates'] as $peKey):
$shippingPrice+=str_replace("$","",$peKey['prices']['shipping_price']);
$salesTax+=str_replace("$","",$peKey['prices']['sales_tax']);
$finalPrice+=str_replace("$","",$peKey['prices']['final_price']);
endforeach;
$purchaseEstimateInformation.="<label style='margin-left:30px;width:120px;display:inline-block'>Shipping Price</label>: $".$shippingPrice."<br>";
$purchaseEstimateInformation.="<label style='margin-left:30px;width:120px;display:inline-block'>Sales Tax</label>: $".$salesTax."<br>";
$purchaseEstimateInformation.="<label style='margin-left:30px;width:120px;display:inline-block;font-weight:bold'>Final Price</label>: <font color='seagreen' style='font-weight: bold; border-radius: 10px; padding: 3px 10px; box-shadow: 0px 0px 10px lightgrey inset; border: 1px solid lightblue;'>$".$finalPrice."</font><br>";
endif;
  
  $form="<hr><br><br>";
  $form.="<form method='post' action='' onsubmit='return pleaseWaitOverlay()' style='font-family:lucida sans unicode'>";
  $form.=miniCart($_SESSION['twoTapCart'],'',$finalPrice);
  $form.="<div style='margin-left:30px;margin-right:30px'><h1>Choose Shipping Options </h1><br>";
  $shipping_options=getShippingOptions();
  foreach($shipping_options as $key): 
     $shippingCounter=0;
    foreach($key as $sk=>$sv):
    $checked="";
    if($_POST['chosen_shipping']==$sk)
    $checked="checked='checked'";
    if(!$_POST['chosen_shipping'])
    $checked="checked='checked'";
    $form.="<input type='radio' value='$sk' $checked name='chosen_shipping'> $sv<br>";
    $shippingCounter++;
    endforeach;
  endforeach;
  $form.="</div>";
  $form.=$purchaseEstimateInformation;
  $form.="<div class='billing_address'>";
  $form.="<h1>Billing Address</h1><br>";
  $countries=getCountriesList();
  $form.="<label style='width:120px;display:inline-block'>Country</label><select name='billing_country' id='country' style='width:150px' required='required'>";
   $form.="<option value=''>--SELECT--</option>";
  foreach($countries as $country_value):
  $billing_country_sel=($country_value==$_SESSION['twoTapCart']['billing_address']['billing_country'])?"selected='selected'":"";
  $form.="<option value='$country_value' $billing_country_sel>$country_value</option>";
  endforeach;
  $form.="</select><br>";
  foreach($inputs as $key=>$value):
  $label=ucfirst(str_replace("_"," ",$value));
  $id=$value;
  $name="billing_".$value;
  $field_value=$_SESSION['twoTapCart']['billing_address'][$name];
  $form.="<label style='width:120px;display:inline-block'>$label</label><input autocomplete='off' type='text' required='required' name='$name' id='$id' value='$field_value'><br>";
  endforeach;

  $states=getStatesList();
  $form.="<label style='width:120px;display:inline-block'>State</label><select name='billing_state' id='state' style='width:150px' required='required'>";
   $form.="<option value=''>--SELECT--</option>";
  foreach($states as $state_value):
  $billing_state_sel=($state_value==$_SESSION['twoTapCart']['billing_address']['billing_state'])?"selected='selected'":"";
  $form.="<option value='$state_value' $billing_state_sel>$state_value</option>";
  endforeach;
  $form.="</select><br>";


  $form.="</div>";

  $form.="<div class='shipping_address'>";
  $form.="<h1>Shipping Address</h1><br><br>";
  $form.="<span style='bottom:10px;position:relative'><input type='checkbox' checked='checked' id='same_billing_address' onclick='ChangeShippingAddress()'> Shipping Address same as that of billing Address.</span><br>";
  $countries=getCountriesList();
  $form.="<label style='width:120px;display:inline-block'>Country</label><select name='shipping_country' id='country' style='width:150px' required='required'>";
   $form.="<option value=''>--SELECT--</option>";
  foreach($countries as $country_value):
  $shipping_country_sel=($country_value==$_SESSION['twoTapCart']['shipping_address']['shipping_country'])?"selected='selected'":"";
  $form.="<option value='$country_value' $shipping_country_sel>$country_value</option>";
  endforeach;
  $form.="</select><br>";
 

  unset($inputs[0]);
  foreach($inputs as $key=>$value):
  $label=ucfirst(str_replace("_"," ",$value));
  $id=$value;
  $name="shipping_".$value;
  $field_value=$_SESSION['twoTapCart']['shipping_address'][$name];
  $form.="<label style='width:120px;display:inline-block'>$label</label><input autocomplete='off' required='required' type='text' name='$name' id='$id' value='$field_value'><br>";
  endforeach;

  $states=getStatesList();
  $form.="<label style='width:120px;display:inline-block'>State</label><select name='shipping_state' id='state' style='width:150px' required='required'>";
   $form.="<option value=''>--SELECT--</option>";
  foreach($states as $state_value):
  $shipping_state_sel=($state_value==$_SESSION['twoTapCart']['shipping_address']['shipping_state'])?"selected='selected'":"";
  $form.="<option value='$state_value' $shipping_state_sel>$state_value</option>";
  endforeach;
  $form.="</select><br>";
  $form.="</div>";

  

$form.="<div style='margin-top:30px;margin-left:30px;margin-right:30px;'>";
$form.="<h1>Payment-Credit Card Information</h1><br>";
$form.="<label style='width:120px;display:inline-block'>Name on card</label><input type='text' required='required' name='card_name' id='$id' ><br>";
$form.="<label style='width:120px;display:inline-block'>Card Number</label><input type='text' required='required' name='card_number' id='$id' >
<br>";
$form.="<label style='width:120px;display:inline-block'>Card Type</label><select name='card_type' required='required'>";
$form.="<option value='Visa'>VISA</option><option value='Mastercard'>MASTERCARD</option><option value='American Express'>AMERICAN EXPRESS</option><option value='Discover'>DISCOVER</option>";
$form.="</select><br>";
$month="<select name='expiry_date_month'>";
for($i=1;$i<=12;$i++){
  $month.="<option value='$i'>$i</option>";
}
$month.="</select>";

$year="<select name='expiry_date_year'>";
for($i=2014;$i<=2024;$i++){
  $year.="<option value='$i'>$i</option>";
}
$year.="</select>";

$form.="<label style='width:120px;display:inline-block'>Expiration Date</label> $month / $year <br>";


$form.="<label style='width:120px;display:inline-block'>CVV</label><input type='text' required='required' name='cvv' id='$id' ><br>";

$form.="</div>";

$cartUrl=$base_url."/cart";
$form.="<a style='background: none repeat scroll 0% 0% steelblue;text-decoration:none;color: white; padding: 10px 25px; cursor: pointer; border: 1px none; box-shadow: 0px 0px 10px grey; border-radius: 76px 5px;display:inline-block;float:left;left:4%;position:relative;margin-top:30px;margin-bottom:30px' href='$cartUrl' onclick='pleaseWaitOverlay()'>Go Cart</a>
<input name='submit_address' value='View Purchase Estimates' type='submit' style='background: none repeat scroll 0% 0% steelblue; color: white; padding: 10px 25px; cursor: pointer; border: 1px none; box-shadow: 0px 0px 10px grey; border-radius: 76px 5px;display:none;float:left;left:30%;position:relative;margin-top:30px;margin-bottom:30px'>
<input name='submit_address' value='Go Checkout' type='submit' style='background: none repeat scroll 0% 0% steelblue; color: white; padding: 10px 25px; cursor: pointer; border: 1px none; box-shadow: 0px 0px 10px grey; border-radius: 76px 5px;display:inline-block;float:right;right:4%;position:relative;margin-top:30px;margin-bottom:30px'>";
  $form.="</form>";
  $form.=showCartOverLay();

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
form h1{
  border-bottom: 1px solid lightgrey;
    box-shadow: 0 0 10px grey;
    color: white;
    font-family:lucida sans unicode;
    padding: 10px 15px;
    text-shadow:0 0 10px forestgreen;
    font-weight:normal;
    background:black;
}
.shipping_address{
  display:inline-block;
  width:46%;
  margin-left:30px;
  margin-top:30px;
}

input[type='submit']{
font-family:lucida sans unicode;
}

    #darkOverlay{
    background: none repeat scroll 0 0 black;
    //background:white;
    display: none;
    height: 100%;
    opacity: 0.7;
    position: absolute;
    top: 0;
    left:0;
    width: 100%;
    z-index:501;
    position:fixed;
}
#cartDiv{
    background: none repeat scroll 0 0 white;
    border: 15px solid black;
    border-radius: 20px;
    display: none;
    height: 50px;
    left: 39%;
    line-height: 10px;
    padding: 10px 30px 30px;
    position: fixed;
    top: 35%;
    width: 225px;
    z-index: 501;
    font-family:lucida sans unicode;
}  

.cancelButton{
    background: none repeat scroll 0 0 white;
    border-radius: 33px;
    color: black;
    font-size: 20px;
    left: 61%;
    padding: 3px 9px;
    position: fixed;
    top: 33%;
    z-index: 100000;
    display:none;
    text-decoration:none !important;

</style>

<script>
ChangeShippingAddress();

 //One more thing, the same billing address has to be checked, if all the parameters are correct. 
 if($(".shipping_address #country").val()==$(".billing_address  #country").val() && $(".shipping_address #title").val()==$(".billing_address #title").val() && $(".shipping_address #first_name").val()==$(".billing_address #first_name").val() && $(".shipping_address #last_name").val()==$(".billing_address #last_name").val() && $(".shipping_address #address").val()==$(".billing_address  #address").val() && $(".shipping_address #city").val()==$(".billing_address  #city").val() && $(".shipping_address #state").val()==$(".billing_address  #state").val() && $(".shipping_address #zip").val()==$(".billing_address  #zip").val() && $(".shipping_address #telephone").val()==$(".billing_address  #telephone").val()){
 document.getElementById('same_billing_address').checked=true;
 }
 else
 {
 document.getElementById('same_billing_address').checked=false;
 }

$(".billing_address input[type='text'], .billing_address select").bind("keyup change",function(){
ChangeShippingAddress();
});

function pleaseWaitOverlay(){

            $('#darkOverlay').fadeIn();
            $('#cartDiv').fadeIn();
            $("#ajax_loader").css('top','8%').show();
            $('#cartDiv').append('<br><br><br>Processing your request, Please wait...');
            $('.cancelButton').hide(); 
            return true;
          }

function ChangeShippingAddress(){
  is_checked=document.getElementById('same_billing_address').checked;
  if(is_checked){
    $(".shipping_address #country").val($(".billing_address  #country").val());
    $(".shipping_address #title").val($(".billing_address  #title").val());
    $(".shipping_address #first_name").val($(".billing_address  #first_name").val());
    $(".shipping_address #last_name").val($(".billing_address  #last_name").val());
    $(".shipping_address #address").val($(".billing_address  #address").val());
    $(".shipping_address #city").val($(".billing_address  #city").val());
    $(".shipping_address #state").val($(".billing_address  #state").val());
    $(".shipping_address #zip").val($(".billing_address  #zip").val());
    $(".shipping_address #telephone").val($(".billing_address  #telephone").val());
  }
  
  
 
 
 
}
</script>
EOD;




  $form.=$css;

  return $form;


}

function getCurrentCheckoutStatus()
 {
 global $user,$base_url;
 if(!$_SESSION['twotap_collector']):
 header("Location:".$base_url."/order-history");
 endif;
 drupal_set_message("Purchase Successfully completed, please checkout the purchase details of your products!");
  //first complete the db
 unset($_SESSION['twoTapCart']['card_details']); //Don't save the credit card details, its illegal.
 $purchase_details=base64_encode(serialize($_SESSION['twoTapCart']));
 $purchased_on=date("Y-m-d H:i:s",time());
 $cart_id=$_SESSION['twotap_latest_cart_id'];
 $purchase_id=$_SESSION['twotap_latest_purchase_id'];
 $uid=$user->uid;
 if($_SESSION['twotap_collector']): //Don't know why this executes twice, and inserts a empty in the table.
 db_query("INSERT INTO twotap_order_history SET purchase_details='$purchase_details',purchased_on='$purchased_on', cart_id='$cart_id',user_id='$uid',purchase_id='$purchase_id'");
 endif;
 $results=db_query("SELECT * FROM twotap_order_history WHERE user_id='{$user->uid}' ORDER BY order_id DESC");
 $html.="<br>Thanks for shopping with 72looks.com, your order would be shipped very soon from our store, following are the details of your recent purchase.<br>";
 while($rec=db_fetch_object($results)):
 $html.="<br><table style='width:70%'>";
 $html.="<tr><td><label>Order Id</label> : #$rec->order_id</td></tr>";
 $html.="<tr><td><label>Transaction Id</label> : $rec->purchase_id</td></tr>";
 $html.="<tr><td><label>Purchased On</label> : ".date("m-d-Y, h:i A",strtotime($rec->purchased_on))."</td></tr>";
 $html.="</table>";
 break;
 endwhile;
 $html.="<br>For more details on the order please visit your account <a href='$base_url/order-history'>Order History</a> page";
$html="<hr><div style='width:94%;margin:auto;'>$html</div><br>";
$html.="<style>
label{
  width:150px;
  padding:10px;
  display:inline-block;
  font-weight:bold;
  font-family:lucida sans unicode;
}
td{
  padding:10px;
}
tr:nth-child(even) {background:lightgrey}
tr:nth-child(odd) {background:lightyellow}
</style>";

//Very important step,remove all old session dfrom here,
  unset($_SESSION['twoTapCart']);
  unset($_SESSION['twotap_collector']);
  unset($_SESSION['twoTapCartSorted']);
  unset($_SESSION['twoTapbackUpAttribs']);


 return $html;
 }

 function getStatesList(){
  $states=array("Alabama","Alaska","Alberta","American","Samoa","Arizona" 
,"Arkansas","Armed Forces Americas","Armed Forces Europe","Armed Forces Pacific","British Columbia"  
,"California","Colorado","Connecticut","Delaware","District of Columbia"
,"District of Columbia","Federated States of Micronesia","Florida Georgia Guam"  
,"Hawaii","Idaho","Illinois","Indiana","Iowa"  
,"Kansas","Kentucky","Louisiana","Maine","Manitoba"  
,"Marshall","Islands","Maryland","Massachusetts","Michigan","Minnesota" 
,"Mississippi","Missouri","Montana","Nebraska","Nevada"  
,"New Brunswick","New Hampshire","New Jersey","New Mexico","New York"  
,"Newfoundland and Labrador","North Carolina","North Dakota","Northern Mariana Islands","Northwest Territories" 
,"Nova","Scotia","Nunavut","Ohio","Oklahoma","Ontario" 
,"Oregon","Palau","Pennsylvania","Prince Edward Island","Puerto Rico" 
,"Quebec","Rhode Island","Saskatchewan","South Carolina","South Dakota"  
,"Tennessee","Texas","U.S. Virgin Islands","Utah","Vermont" 
,"Virginia","Washington","West Virginia","Wisconsin","Wyoming" 
,"Yukon");

  return $states;
 }

function getCountriesList(){

$countries = array("Afghanistan","Aland Islands","Albania","Algeria","American Samoa"
,"Andorr","Angola","Anguilla","Antarctica","Antigua and Barbuda","Argentin","Armeni","Arub"
,"Australi","Austria","Azerbaijan","Bahama","Bahrai","Bangladesh","Barbados ","Belaru","Belgiu"
,"Belize","Beni","Bermuda","Bhutan","Bolivi","Bosnia and Herzegowina","Botswana","Bouvet Island"
,"Brazil" ,"British Indian Ocean Territory" ,"Brunei Darussalam","Bulgaria" ,"Burkina Faso " ,"Burundi"
,"Cambodia" ,"Cameroon" ,"Canada" ,"Cape Verde " ,"Cayman Islands" ,"Central African Republic" ,"Chad" ,"Chile","China"
,"Christmas Island","Cocos (Keeling) Islands", "Colombia","Comoros Congo"
,"Congo, The Democratic Republic Of The","Cook Islands" ,"Costa Rica","Cote D'Ivoire","Croatia" 
,"Cuba","Cyprus", "Czech Republic",  "Denmark", "Djibouti"  
,"Dominica","Dominican Republic","East Timor","Ecuador","Egypt" 
,"El Salvador","Equatorial Guinea","Eritrea","Estonia","Ethiopia"  
,"Falkland Islands (Malvinas)","Faroe Islands","Fiji","Finland","France"
,"French Guiana","French Southern Territories","Gabon","Gambia","Georgia"
,"Germany","Ghana","Gibraltar","Greec","Greenland"
,"Grenada","Guadeloupe","Guam","Guatemala","Guinea"  
,"Guinea-Bissau","Guyana","Hait","Heard And Mc Donald Islands","Honduras"
,"Hong Kong","Hungary","Iceland","India","Indonesia" 
,"Iran","Iraq","Ireland","Isle of Man","Israel"  
,"Italy","Jamaica","Japan","Jordan","Kazakhstan"  
,"Kenya","Kiribati","Kuwait","Kyrgyzstan","Lao People's Democratic Republic"  
,"Latvia","Lebanon","Lesotho","Liberia","Libyan Arab Jamahiriya"  
,"Liechtenstein","Lithuani","Luxembour" ,"Macau","Macedonia, The Former Yugoslav Republic"
,"Madagascar","Malawi","Malaysia","Maldives","Mali"
,"Malta","Marshall Islands","Martinique","Mauritania","Mauritius"
,"Mayotte","Mexico","Micronesia (Federated States Of)","Moldova","Monaco"  
,"Mongolia","Montenegro","Montserrat","Morocco","Mozambique"  
,"Myanmar","Namibia","Nauru","Nepal","Netherlands" 
,"Netherlands Antilles","New Caledonia","New Zealand","Nicaragua","Niger" 
,"Nigeria","Niue","Norfolk Island","North Korea","Northern Mariana Islands"
,"Norway","Oman","Pakistan","Palau","Palestenian Territories"
,"Panama","Papua New Guinea","Paraguay","Peru","Philippines" 
,"Pitcairn","Poland","Portugal","Puerto Rico","Qatar" 
,"Reunion","Romania","Russian Federation","Rwanda","Saint Kitts And Nevis" 
,"Saint Lucia","Saint Vincent And The Grenadines","Samoa","San Marino","Sao Tome And Principe" 
,"Saudi Arabia","Scotland","Senegal","Serbia","Seychelles"  
,"Sierra Leone","Singapore","Slovakia","Slovenia","Solomon Islands" 
,"Somalia","South Africa","South Georgia And The South Sandwich Isl","South Korea Spain" 
,"Spain - Canary Islands","Sri Lanka","St Barthelemy","St. Helena","St. Pierre And Miquelon" 
,"Sudan","Suriname","Svalbard And Jan Mayen Islands","Swaziland","Sweden"  
,"Switzerland","Syrian Arab Republic","Tahiti","Taiwan, Province of China","Tajikistan"  
,"Tanzania, United Republic Of","Thailand","Togo","Tokelau","Tonga"
,"Trinidad And Tobago","Tunisia","Turkey","Turkmenistan","Turks And Caicos Islands"  
,"Tuvalu","Uganda","Ukraine","United Arab Emirates","United Kingdom"  
,"United Kingdom - Guernsey","United Kingdom - Jersey","United States Minor Outlying Islands","United States of America","Uruguay" 
,"Uzbekistan","Vanuatu","Vatican City State (Holy See)","Venezuela","Vietnam" 
,"Virgin Islands (British)","Virgin Islands (U.S)","Wallis And Futuna Islands","Western Sahara","Yemen"
,"Zambia","Zimbabwe");        

return $countries;
}
//This is a very Important function to show a cart.
function miniCart($cart,$time="present"){
  //debug($cart);
  $smallCart="<h1 style='margin-left:30px;margin-right:30px;'>Your Cart</h1>";
  $smallCart.="<table style='width:94%;font-family:lucida sans unicode;margin:30px;font-size:small;box-shadow:0 0 10px;color:grey'>";
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
  if($key['details']->field_name_value)
  $smallCart.="<tr><td>{$key['details']->field_name_value}<br>$options</td><td>{$key['details']->brand}</td><td style='padding-left:20px'>{$key['cart_value']['quantity']}</td><td>$$price</td><td>$$priceMultiple</td></tr>";
  endforeach;
  $grand_total=number_format($grand_total,2);
  $smallCart.="<tr style='background:steelblue;color:white;'><td colspan='4' align='right'>Grand Sub-Total :</td><td>$$grand_total</td></tr>";
  
  if($cart['price_estimate']['final_price']):
  $smallCart.="<tr style='background:steelblue;color:white;border-top:1px solid lightgrey;font-weight:bold'><td colspan='4' align='right'>Final Price (Including Taxes):</td><td>$".number_format($cart['price_estimate']['final_price'],2)."</td></tr>";
  endif;
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



function checkOutCss(){
    $css="
    <style>
    #twoTapCart td{
    border-top:1px dashed lightgrey;
    }
    input[type='submit'],input[type='button']{
    font-family:lucida sans unicode;
    }
    #cart_upper_links a{
    border-radius: 10px;
    box-shadow: 0 0 10px grey inset;
    color: black;
    font-family: candara;
    padding: 3px 15px;
    text-decoration: none;
    }
    #darkOverlay{
    background: none repeat scroll 0 0 black;
    //background:white;
    display: none;
    height: 100%;
    opacity: 0.7;
    position: absolute;
    top: 0;
    left:0;
    width: 100%;
    z-index:501;
    position:fixed;
}
#cartDiv{
    background: none repeat scroll 0 0 white;
    border: 15px solid black;
    border-radius: 20px;
    display: none;
    height: 50px;
    left: 39%;
    line-height: 10px;
    padding: 10px 30px 30px;
    position: fixed;
    top: 35%;
    width: 225px;
    z-index: 501;
    font-family:lucida sans unicode;
}  

.cancelButton{
    background: none repeat scroll 0 0 white;
    border-radius: 33px;
    color: black;
    font-size: 20px;
    left: 61%;
    padding: 3px 9px;
    position: fixed;
    top: 33%;
    z-index: 100000;
    display:none;
    text-decoration:none !important;

}
    </style>";

    return $css;
}

function twoTapAddToCart(){
    $base_path=getcwd();
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $productArray=array();
    $i=0;
    foreach($_SESSION['twoTapCart'] as $key):
        $products[$i]=$key['details']->field_product_url_url;
    $i++;
    endforeach;
    $productArray['products']=$products;

    $cart_id=add_to_cart($productArray);
    return $cart_id;
}

function twoTapAddToCartStatus($cart_id)
{
    $base_path=getcwd();
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $response=add_to_cart_status($cart_id);
    return $response;
}

//This will give the purchase id
function twoTapPurchase(){
    $_SESSION['twotap_bad_fields']="";
    $cart_id=$_SESSION['twotap_latest_cart_id'];
    $base_path=getcwd();
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $field_inputs=get_myfield_inputs();
    $response=purchase_info($cart_id,$field_inputs);
    
    if($response['message']=="bad_required_fields"):
    $_SESSION['twotap_bad_fields']=$response['description']."<br>";
    endif;

    $_SESSION['twotap_latest_purchase_id']=$response['purchase_id'];
    return $response['purchase_id'];
}


function getPurchaseEstimates()
{
    $_SESSION['twotap_bad_fields']="";
    $cart_id=$_SESSION['twotap_latest_cart_id'];
    $base_path=getcwd();
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $field_inputs=get_myfield_inputs();
    $response=purchase_estimates($cart_id,$field_inputs); 
    // debug($response);
    // exit;
    return $response;
}

function getPurchaseEstimatesNew(){
    $cart_id=$_SESSION['twotap_latest_cart_id'];
    $base_path=getcwd();
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $field_inputs=get_myfield_inputs();
    $response=purchase_info($cart_id,$field_inputs);
    $pec=twoTapPurchaseInfoStatus($response['purchase_id']);
    $purchaseEstimates=array();
    $purchaseEstimates['message']=$pec['message'];
    foreach($pec['sites'] as $key=>$value):
    $purchaseEstimates['estimates'][$key]['prices']=$pec['sites'][$key]['prices'];
    endforeach;
    
    return $purchaseEstimates;
}



function twoTapPurchaseInfoStatus($purchase_id=""){
    global $base_url;
    ini_set("max_exectution_time",3000);
    $purchase_id=($purchase_id)?$purchase_id:$_SESSION['twotap_latest_purchase_id'];
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $response=purchase_info_status($purchase_id);
    
//     debug($response);
//     exit;
    if($response['message']=="has_failures"):
    foreach($response['sites'] as $key):
    $_SESSION['twotap_bad_fields'].=implode("<br>",$key['status_messages']);
    endforeach;
    $_SESSION['twoTapCart']['twotap_bad_fields']=$_SESSION['twotap_bad_fields'];
    header("Location:".$base_url."/cart");
    endif;

    if($response['message']=="not_found"):   
    $_SESSION['twotap_bad_fields'].=$response['description']; 
    endif;

    
//  $purchaseEstimateInformation="";
  if($response['message']=="done"):
    $shippingPrice=$salesTax=$finalPrice="";
    foreach($response['sites'] as $peKey=>$peVal):
      $shippingPrice+=str_replace("$","",$response['sites'][$peKey]['prices']['shipping_price']);
      $salesTax+=str_replace("$","",$response['sites'][$peKey]['prices']['sales_tax']);
      $finalPrice+=str_replace("$","",$response['sites'][$peKey]['prices']['final_price']);
    endforeach;
    
    //We have to store this value in our session as well.
    $_SESSION['twoTapCart']['price_estimate']['final_price']=$finalPrice;
    $_SESSION['twoTapCart']['price_estimate']['sales_tax']=$salesTax;
    $_SESSION['twoTapCart']['price_estimate']['shipping_price']=$shippingPrice;
    //stored price estimate in the twotapCart, which was client requirement.
    endif;
    
    
    return $response;
}

function twoTapConfirmPurchase(){
  global $base_url;
  displayErrors();
    $purchase_id=$_SESSION['twotap_latest_purchase_id'];
    $base_path=getcwd();
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $response=purchase_confirm($purchase_id);

    if($response['message']=="failed"):
    $_SESSION['twotap_bad_fields']=$response['description'];
    endif;

//     debug($response);
//     exit;

    return $response;
}

function twoTapConfirmPurchaseStatus(){
   global $base_url;
    $purchase_id=$_SESSION['twotap_latest_purchase_id'];
    $base_path=getcwd();
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $response=purchase_confirm_status($purchase_id);
    if($response['message']=="has_failures" or $response['message']=="invalid_status"):
    $_SESSION['twotap_bad_fields']=$response['description']."<br>";
    foreach($response['sites'] as $key):
    $errors=array();
    $errors=$key['status_messages'];
    $_SESSION['twotap_bad_fields'].=implode("<br>",$errors);
    endforeach;
    endif;

    // debug($response);
    // exit;

    return $response;
}

function BUY_PRODUCTS(){
  twoTapPurchase();
  if(!$_SESSION['twotap_bad_fields'])
  twoTapPurchaseInfoStatus();
}

function displayErrors(){
  ini_set('display_errors',1);
  ini_set('display_startup_errors',1);
  error_reporting(-1);
}


function confirmPurchaseTwoTap(){
  global $base_url;
  ini_set("max_exectution_time",3000);
  if($_REQUEST['confirm_purchase_status_last_step']){
  $purchase=twoTapConfirmPurchase();
  $response=twoTapConfirmPurchaseStatus();

  if($response['message']=="done" && ($purchase['message']=="done" or $purchase['message']=="still_processing")):
  header("location:".$base_url."/checkout-status");
  exit;
  endif;

  if($_SESSION['twotap_bad_fields']):
    drupal_set_message($_SESSION['twotap_bad_fields'],"error");
  endif;

  }
 
  $html="<span style='font-family:lucida sans unicode;'>";
  $purchaseEstimateInformation="<br><h1 style='width:92%;margin:auto;border-bottom: 1px solid lightgrey;box-shadow: 0 0 10px grey;color: white;font-family:lucida sans unicode;padding: 10px 15px;text-shadow:0 0 10px forestgreen;font-weight:normal;background:black;'>Purchase Estimation</h1><br>";
  $purchaseEstimateInformation.="<label style='padding:7px;margin-left:30px;width:120px;display:inline-block'>Shipping Price</label>: $".number_format($_SESSION['twoTapCart']['price_estimate']['shipping_price'],2)."<br>";
  $purchaseEstimateInformation.="<label style='padding:7px;margin-left:30px;width:120px;display:inline-block'>Sales Tax</label>: $".number_format($_SESSION['twoTapCart']['price_estimate']['sales_tax'],2)."<br>";
  $purchaseEstimateInformation.="<div style='font-size:15px;color:white;background:steelblue;width: 95%;margin: auto;padding-left: 0px;'><label style='padding:7px;margin-left:3px;width:120px;display:inline-block;font-weight:bold'>Final Price</label>: <font color='white' style='font-weight: bold; border-radius: 10px;'>$".number_format($_SESSION['twoTapCart']['price_estimate']['final_price'],2)."</font> (Inclusive all charges)</div>";
  
  $html.="<hr>$purchaseEstimateInformation<br><form method='post' action='/development/beta/confirm-purchase' accept-charset='UTF-8' onsubmit='pleaseWaitOverlay()'>";
  $html.="<input type='checkbox' required='required' value='1' style='margin-left:40px;float:left'><span style='font-family:lucida sans unicode;float: left; color: green; position: relative; font-size: 15px; top: 0px;'>This is the final charge you will be charged.You have two minutes to confirm.</span>";
  $html.="<div style='clear:both'></div><a onclick='pleaseWaitOverlay()' href='$base_url/checkout' style='background: none repeat scroll 0% 0% steelblue; color: white; padding: 10px 25px; cursor: pointer; border: 1px none; box-shadow: 0px 0px 10px grey; border-radius: 76px 5px;display:inline-block;float:left;left:4%;position:relative;margin-top:30px;margin-bottom:30px;text-decoration:none'><< Checkout</a>";

  $html.="<input value='Confirm Purchase' type='submit' name='confirm_purchase_status_last_step' style='background: none repeat scroll 0% 0% steelblue; color: white; padding: 10px 25px; cursor: pointer; border: 1px none; box-shadow: 0px 0px 10px grey; border-radius: 76px 5px;display:inline-block;float:right;right:4%;position:relative;margin-top:30px;margin-bottom:30px'>";
  $html.="<input type='hidden'>";
  $html.="</form>";
  $html.=showCartOverLay();
  $html.=checkOutCss();
  $html.=checkOutJS();
  $html.="</span>";

  return $html;
}


function get_myfield_inputs(){
$shipping_payment_data=array_merge($_SESSION['twoTapCart']['billing_address'],$_SESSION['twoTapCart']['shipping_address'],$_SESSION['twoTapCart']['card_details']);
$shipping_payment_data['name']=$_SESSION['twoTapCart']['billing_address']['billing_first_name']." ".$_SESSION['twoTapCart']['billing_address']['billing_last_name'];
$shipping_payment_data['email']=$_SESSION['twoTapCart']['billing_address']['billing_email'];
$shipping_payment_data['billing_name']=$shipping_payment_data['billing_name']=$shipping_payment_data['name'];
$cart_id=$_SESSION['twotap_latest_cart_id'];
$cart=twoTapAddToCartStatus($cart_id);
$field_input=array();
foreach($cart['sites'] as $siteId=>$value):
$field_input[$siteId]['noauthCheckout']=$shipping_payment_data;
$field_input[$siteId]['login']=array("email"=>variable_get("twotap_email"),"password"=>variable_get("twotap_password"));

foreach($cart['sites'][$siteId]['add_to_cart'] as $l=>$g):
foreach($_SESSION['twoTapCart'] as $k):
if(md5($k['details']->field_product_url_url)==$l){
$field_input[$siteId]['addToCart'][$l]=$k['cart_value'];
$field_input[$siteId]['shipping_option']=$_SESSION['twoTapCart']['billing_address']['chosen_shipping'];
break;
}
endforeach;
endforeach;

endforeach; ///End of the add_to_cartstatus.

//$field_input['shipping_option']=$_SESSION['twoTapCart']['billing_address']['chosen_shipping'];
return $field_input;
}

function debug($arrayObject){
echo "<pre style='font-family:consolas;font-size:small'>";
print_r($arrayObject);
echo"</pre>";
}


function checkProductAvailability($productId){
    $results=db_query("SELECT field_product_url_url as product_url FROM content_type_product WHERE nid='$productId'");
    while($rec=db_fetch_object($results)):
      $product_url=$rec->product_url;
     break;
    endwhile;
    $base_path=getcwd();
    $sdk=$base_path."/".drupal_get_path('module','twotapmart')."/twotap-sdk/twoTap.php";
    include_once($sdk);
    $productArray=array();
    $productArray['products'][0]=$product_url;
    $cart_id=add_to_cart($productArray);
    $response=add_to_cart_status($cart_id);
    //debug($response);
    $result=($response['message']=="done")?true:false;

    //please update the product rate in the database as well, this will create a superb synchronization option for me.
    if($result):
    foreach($response['sites'] as $key):
    $md5=md5($product_url);
    $price=str_replace("$","",$key['add_to_cart'][$md5]['price']);
    db_query("UPDATE content_type_product SET field_original_price_value='$price' WHERE nid='$productId'");
    endforeach;
    endif;

  return $result;
}

function showCustomerOrders(){
  global $user,$base_url;
  if($_POST['deleteOrderId']):
    db_query("DELETE FROM twotap_order_history WHERE order_id='{$_POST['deleteOrderId']}' ");
      drupal_set_message("Order History Deleted successfully !");
  endif;

  $results=db_query("SELECT * FROM twotap_order_history WHERE user_id='{$user->uid}' ORDER BY order_id DESC");
  //debug($_SESSION['twoTapCart']);
  $header=array("data_1"=>"Order ID","username"=>"Customer Name","data_2"=>"Transaction ID","items"=>"Items","order_total"=>"Order Total","data_3"=>"Purchased On","status"=>"Status","edit"=>"View Details");
  $rows=array();
  $i=0;
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
    $base_order_id=str_replace("=","",base64_encode($key->order_id));
    $edit="<a href='$base_url/view-order/$base_order_id'>View</a>";
    $customer=user_load($key->user_id);
    $rows[]=array("data_1"=>"#".$key->order_id." .","username"=>$customer->name,"data_2"=>$key->purchase_id,"items"=>$pd['productsTaken'],"order_total"=>"$".$grand_total,"data_3"=>$purchased_on,"status"=>$status,"edit"=>$edit);
  $i++;
  endwhile;
  if(!$i):
    $noOrder="<font color='red'>Sorry, You have not placed any yet</font>";
  endif;
  $css="<style>
  table{
    margin-top:15px;
    font-family:lucida sans unicode;
    font-size:small;
    color:grey;
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
    return theme('table',$header,$rows).$noOrder.$css;
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
// function getSingleOrderInfo($order_id){

//   return $order_id;
// }
function getSingleOrderInfo($order_id){
$order_id=base64_decode($order_id);
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
    font-family:lucida sans unicode;
    padding: 5px 10px;
    text-shadow:0 0 10px forestgreen;
    background:black;
    margin:0px !important;
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
  font-family:lucida sans unicode;
  font-weight:bold;
}
table{
  font-size:small;
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
$order.="<hr><a href='$base_url/order-history' style='float:right;text-decoration:none;color:white;' class='form-submit'>Back</a>";
$order.="<br><form method='post'>";
$order.="<label>ORDER ID</label>: #$rec->order_id"."<br>";
$order.="<label>Customer</label>: $customer->name"."<br>";
$updateImage=$base_url."/".drupal_get_path('module','twotapmart')."/img/update.png";
$update="<input type='image' src='$updateImage' name='submit_cart' value='update_cart' style='top:5px;position:relative'> Update";
if($rec->status==0)
$status="Pending";
if($rec->status==1)
$status="Completed";
if($rec->status==2)
$status="Still Processing";
if($rec->status==3)
$status="Cancelled";
$order.="<label>Status </label>: $status<br>";
//$order.="<label>Two Tap Cart Id </label>: $rec->cart_id"."<br>";
$order.="<label>Transaction Id </label>: $rec->purchase_id"."<br>";
$order.="<label>Purchased On </label>: ".date("m-d-Y, h:i A",strtotime($rec->purchased_on))."<br>";
$order.=miniCart(unserialize(base64_decode($rec->purchase_details)));
$pd=unserialize(base64_decode($rec->purchase_details));
$order.="<h1>Purchase Estimate</h1>";
$order.="<label>Shipping Price</label>:$".number_format($pd['price_estimate']['shipping_price'],2)."<br>";
$order.="<label>Sales Tax</label>:$".number_format($pd['price_estimate']['sales_tax'],2)."<br>";
$order.="<label>Final Price</label>:$".number_format($pd['price_estimate']['final_price'],2)."<br><br>";

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
$order.="<div style='background:lightyellow;margin:30px;padding:10px;'>".ucfirst($pd['billing_address']['chosen_shipping'])."</div>";
$order.=$css;
$order.="</form>";
$order="<div style='width:94%;margin:auto;font-family:lucida sans unicode'>$order</div>";
return $order;
endwhile;
}

?>
