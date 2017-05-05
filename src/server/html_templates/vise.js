var VISE_THEME_MESSAGE_TIMEOUT_MS = 4000;

var _vise_message_clear_timer;

var _vise_server = new XMLHttpRequest();
var _vise_messenger = new XMLHttpRequest();
var _vise_query = new XMLHttpRequest();

var VISE_SERVER_ADDRESS    = "http://localhost:8080/";
var VISE_MESSENGER_ADDRESS = VISE_SERVER_ADDRESS + "_message";
var VISE_QUERY_ADDRESS     = VISE_SERVER_ADDRESS + "_query";

_vise_server.addEventListener("load", _vise_server_response_listener);
_vise_messenger.addEventListener("load", _vise_message_listener);
_vise_query.addEventListener("load", _vise_query_listener);

var _vise_current_state_id = -1;
var _vise_current_state_name = "";
var _vise_current_search_engine_name = "";
var _vise_search_engine_state;

function _vise_init() {
  // initially hide footer and log
  document.getElementById("footer").style.display = "none";
  document.getElementById("log").style.display = "none";

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

      case 'http_post_response':
        // @todo handle post response, current assumption is that it will be OK
        // json['result'] === "OK"
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

  if ( receiver === "command" ) {
    _vise_handle_command( sender, msg );
  } else if ( receiver === "log" ) {
    _vise_handle_log_message(sender, msg);
  } else if ( receiver === "message" ) {
    _vise_show_message(msg, VISE_THEME_MESSAGE_TIMEOUT_MS);
  }
}

function _vise_show_message(msg, t) {
  if ( _vise_message_clear_timer ) {
    clearTimeout(_vise_message_clear_timer); // stop any previous timeouts
  }
  document.getElementById('message_panel').innerHTML = msg;

  var timeout = t;
  if ( timeout > 0 || typeof t === 'undefined') {
    if ( typeof t === 'undefined') {
      timeout = VISE_THEME_MESSAGE_TIMEOUT_MS;
    }
    _vise_message_clear_timer = setTimeout( function() {
        document.getElementById('message_panel').innerHTML = ' ';
    }, timeout);
  }
}

function _vise_handle_log_message(sender, msg) {
  var log_panel = document.getElementById( "log" );
  if ( msg.startsWith("\n") ) {
    msg = "\n" + sender + " : " + msg.substr(1);
  }
  if ( typeof log_panel !== 'undefined' ) {
    log_panel.innerHTML += msg;
    log_panel.scrollTop = log_panel.scrollHeight; // automatically scroll
  } 
}

function _vise_handle_command(sender, command_str) {
  // command_str = "_state update_now"
  // command_str = "_control_panel remove Info_proceed_button"
  var first_spc = command_str.indexOf(' ', 0);
  var cmd = command_str.substr(0, first_spc);
  var param = command_str.substr(first_spc+1);

  switch ( cmd ) {
    case "_state":
      switch( param ) {
        case 'update_now':
          _vise_server_send_get_request("_state");
          break;
        case "show":
          document.getElementById("footer").style.display = "block";
          break;
        case "hide":
          document.getElementById("footer").style.display = "none";
          break;
      }
      break;
    case "_log":
      _vise_handle_log_command(param);
      break;
    case "_control_panel":
      _vise_handle_control_panel_command(param);
      break;
    defaul:
      console.log("Unknown command : " + command_str);
  }
}

function _vise_handle_log_command(command_str) {
  var command = command_str.split(' ');
  for ( var i=0; i < command.length; i++ ) {
    switch( command[i] ) {
      case "show":
        document.getElementById("log").style.display = "block";
        break;
      case "hide":
        document.getElementById("log").style.display = "none";
        break;
      case "clear":
        document.getElementById("log").innerHTML = "&nbsp;";
        break;
      default:
        console.log("Received unknown log command : " + command[i]);
    }
  }
}

function _vise_handle_control_panel_command(command) {
  // command_str = "remove Info_proceed_button"
  var first_spc = command.indexOf(' ', 0);
  var cmd = command.substr(0, first_spc);
  var param = command.substr(first_spc+1);

  switch ( cmd ) {
    case "add":
      var cpanel = document.getElementById("control_panel");
      cpanel.innerHTML += param;
      break;
    case "remove":
      document.getElementById(param).remove();
      break;
    case "clear":
      if ( param === "all" ) {
        var cpanel = document.getElementById("control_panel");
        cpanel.innerHTML = "";
      }
      break;
  }
}

//
// Maintainer of UI to reflect VISE current state 
//
function _vise_update_state_info( json ) {
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

function _vise_server_send_state_post_request(state_name, post_data) {
  _vise_server.open("POST", VISE_SERVER_ADDRESS + state_name);
  _vise_server.send(post_data);
}

function _vise_server_send_get_request(resource_name) {
  _vise_server.open("GET", VISE_SERVER_ADDRESS + resource_name);
  _vise_server.send();
}

function _vise_send_msg_to_training_process(msg) {
  _vise_server_send_post_request("msg_to_training_process " + msg);
}

//
// State: Setting
//
function _vise_send_setting_data() {
  var postdata = [];
  var vise_settings = document.getElementById("vise_setting");
  if ( typeof vise_settings !== 'undefined' && vise_settings !== null ) {
    var setting_param = vise_settings.getElementsByClassName("vise_setting_param");

    for ( var i = 0; i < setting_param.length; i++) {
      var param_name  = setting_param[i].name;
      var param_value = setting_param[i].value;
      postdata.push( param_name + "=" + param_value );
    }
    var postdata_str = postdata.join('\n');

    _vise_server_send_state_post_request( _vise_current_state_name, postdata_str );
  }
}

//
// State: Query
//
function q(s) {
  // s = "cmd=show_img_list&arg1=value1&arg2=value2"
  _vise_query.open( "GET", VISE_QUERY_ADDRESS + "?" + s )
  _vise_query.send();
}

function _vise_query_listener() {
  var response_str = this.responseText;
  var content_type = this.getResponseHeader('Content-Type')

  if ( content_type.includes("text/html") ) {
    document.getElementById("content").innerHTML = response_str;
  } else if ( content_type.includes("application/json") ) {
    var json = JSON.parse(response_str);
    switch(json.id) {
      case '':
        break;

      default:
        console.log("Do not know where to forward the received query response!");
        console.log(response_str);
    }
  } else {
    console.log("Received response of unknown content-type : " + content_type);
  }
}
