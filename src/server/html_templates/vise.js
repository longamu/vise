var VISE_THEME_MESSAGE_TIMEOUT_MS = 4000;

var _vise_message_clear_timer;

var _vise_server = new XMLHttpRequest();
var _vise_messenger = new XMLHttpRequest();

var VISE_SERVER_ADDRESS    = "http://localhost:8080/";
var VISE_MESSENGER_ADDRESS = VISE_SERVER_ADDRESS + "_message";

_vise_server.addEventListener("load", _vise_server_response_listener);
_vise_messenger.addEventListener("load", _vise_message_listener);

var _vise_current_state_id = -1;
var _vise_current_state_name = "";
var _vise_current_search_engine_name = "";
var _vise_search_engine_state;

function _vise_init() {
  // initially hide footer
  document.getElementById("footer").style.display = "none";

  // request the contents of vise_index.html
  _vise_server.open("GET", VISE_SERVER_ADDRESS + "_vise_index.html");
  _vise_server.send();

  // create the seed connection to receive messages
  _vise_messenger.open("GET", VISE_MESSENGER_ADDRESS);
  _vise_messenger.send();

}

function _vise_server_response_listener() {
  var response_str = this.responseText;
  var content_type = this.getResponseHeader('Content-Type')

  if ( content_type.includes("text/html") ) {
    document.getElementById("content").innerHTML = response_str;
  } else if ( content_type.includes("application/json") ) {
    var json = JSON.parse(response_str);
    switch(json.id) {
      case 'search_engine_state':
        _vise_update_state_info(json);
        break;

      default:
        console.log("Do not know where to forward the received response!");
        console.log(response_str);
    }
  } else {
    console.log("Received response of unknown content-type : " + content_type);
  }
}

function _vise_message_listener() {
  var packet = this.responseText;

  // create a connection for next message
  _vise_messenger.open("GET", VISE_MESSENGER_ADDRESS);
  _vise_messenger.send();

  // process the received message asynchronously
  setTimeout( function() {
    _vise_handle_message( packet );
  }, 0);
}
// msg = sender receiver msg
function _vise_handle_message(packet) {
  var first_space  = packet.indexOf(' ', 0);
  var second_space = packet.indexOf(' ', first_space + 1);

  var sender   = packet.substr(0, first_space);
  var receiver = packet.substr(first_space + 1, second_space - first_space - 1);
  var msg = packet.substr(second_space + 1);

  if ( receiver === "control" ) {
    switch ( msg ) {
      case "VISE_STATE_HAS_CHANGED":
        _vise_server_send_get_request("_state");
      break;
      default:
        console.log("Received unknown control message: " + packet);
    }
  }
}

//
// Maintainer of UI to reflect VISE current state 
//
function _vise_update_state_info( json ) {
  console.log( json );
  var html = [];
  for ( var i=1; i<json['state_name_list'].length; i++ ) {
    html.push('<div class="state_block">');
    if ( i === json['current_state_id'] ) {
      html.push('<div class="title current_state">' + json['state_name_list'][i] + '</div>');
    } else {
      html.push('<div class="title">' + json['state_name_list'][i] + '</div>');
    }
    if ( json['state_info_list'][i] === '' ) {
      html.push('<div class="info">&nbsp;</div>');
    } else {
      html.push('<div class="info">' + json['state_info_list'][i] + '</div>');
    }
    html.push('</div>');
    html.push('<div class="state_block_sep">&rarr;</div>');
  }
  // remove the last arrow
  html.splice(html.length - 1, 1);
  document.getElementById("footer").style.display = "block";
  document.getElementById("footer").innerHTML = html.join('');

  if ( _vise_current_state_id !== json['current_state_id'] ) {
    _vise_current_state_id = json['current_state_id'];
    _vise_current_state_name = json['state_name_list'][_vise_current_state_id];
    _vise_current_search_engine_name = json['search_engine_name'];
    _vise_search_engine_state = json;

    // request content for this state
    _vise_fetch_current_state_content();
  }
}

function _vise_fetch_current_state_content() {
  //var resource_uri = _vise_current_search_engine_name + "/" + _vise_current_state_name;
  var resource_uri = _vise_current_state_name;
  _vise_server_send_get_request( resource_uri );
}
//
// Vise Server requests at
// http://localhost:8080
//
function _vise_create_search_engine() {
  var search_engine_name = document.getElementById('vise_search_engine_name').value;
  _vise_server_send_post_request("create_search_engine " + search_engine_name);
}

function _vise_load_search_engine(name) {
  _vise_server_send_post_request("load_search_engine " + name);
}

function _vise_server_send_post_request(post_data) {
  _vise_server.open("POST", VISE_SERVER_ADDRESS);
  _vise_server.send(post_data);
}

function _vise_server_send_get_request(resource_name) {
  _vise_server.open("GET", VISE_SERVER_ADDRESS + resource_name);
  _vise_server.send();
}
