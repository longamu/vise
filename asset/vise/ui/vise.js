/*
  @file        _via_ctrl.js
  @description Handles the user interactions in web based user interface of VISE
  @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
  @date        22 June 2017
*/

var _vise_server_hostname = 'http://localhost';
var _vise_server_port     = '9973';
var _vise_server_ns       = 'vise/repo';
var _vise_server_uri      = _vise_server_hostname + ':' + _vise_server_port + '/' + _vise_server_ns;
var _vise_new_search_engine = {};

// state variables
var is_new_search_engine_image_add_ongoing = false;

// misc variables
var _vise_message_clear_timer = '';

// UI html elements
var invisible_file_input = document.getElementById("invisible_file_input");

// constants
var VISE_THEME_MESSAGE_TIMEOUT_MS = 5000;

// invoked by onload() event handler defined in <body> of vise.html
function vise_init() {
  show_message('VISE 2.0.0 ready');
}

function search_engine(name, version) {
  this.name    = name;
  this.version = version || '1';
  this.files   = [];
  this.filename_list = [];
  this.status  = {};
}

function sel_local_images() {
  // source: https://developer.mozilla.org/en-US/docs/Using_files_from_web_applications
  if (invisible_file_input) {
    invisible_file_input.setAttribute('multiple', 'multiple')
    invisible_file_input.accept   = '.jpg,.jpeg,.png,.bmp';
    invisible_file_input.onchange = search_engine_file_add_local;
    invisible_file_input.click();
  }
}

function search_engine_file_add_local(event) {
  if ( ! is_new_search_engine_image_add_ongoing ) {
    // first add event
    is_new_search_engine_image_add_ongoing = true;
    _vise_new_search_engine_name = generate_search_engine_rand_name();
    _vise_new_search_engine_version = '1';
    _vise_new_search_engine = new search_engine( generate_search_engine_rand_name(), '1' );

    vise_send_server_command(_vise_new_search_engine.name, _vise_new_search_engine.version, 'init').then( function(ok) {
      var added_img_count = 0;
      var promise_list = [];
      var user_selected_images = event.target.files;
      for ( var i = 0; i < user_selected_images.length; ++i ) {
        var filetype = user_selected_images[i].type.substr(0, 5);
        if ( filetype === 'image' ) {
          _vise_new_search_engine.files.push( user_selected_images[i] );
          _vise_new_search_engine.filename_list.push(  user_selected_images[i].name );
          added_img_count += 1;

          var name = _vise_new_search_engine.name;
          var version = _vise_new_search_engine.version;
          var args = ['filename=' + user_selected_images[i].name];
          var payload = user_selected_images[i];

          promise_list.push( vise_send_server_command(name, version, 'add_file', args, payload) );
        }
      }
      Promise.all(promise_list).then( function(values) {
        console.log(values);
        show_message('added [' + values.length + '] files');
        on_new_search_engine_file_add();
      }.bind(this), function(err) {
        console.log(err);
        show_message('failed to add [' + err.length + '] files');
      }.bind(this));
    }.bind(this), function(err) {
      console.log(err);
      show_message('Error: ' + err);
    }.bind(this));
  }
}

function vise_start_indexing() {
  var name = _vise_new_search_engine.name;
  var version = _vise_new_search_engine.version;
  vise_send_server_command(name, version, 'index_start');
  show_message('Indexing started');
}

function vise_send_server_command(name, version, cmd, uri_param, payload) {
  return new Promise( function(ok_callback, err_callback) {
    var xhr = new XMLHttpRequest();
    xhr.addEventListener('load', function(e) {
      ok_callback(xhr.responseText);
    });
    xhr.addEventListener('error', function(e) {
      err_callback(xhr.responseText);
    });
    xhr.addEventListener('abort', function(e) {
      err_callback(xhr.responseText);
    });
    xhr.addEventListener('timeout', function(e) {
      err_callback(xhr.responseText);
    });

    if ( typeof(uri_param) === 'undefined' ) {
      xhr.open('POST', _vise_server_uri + '/' + name + '/' + version + '/' + cmd);
    } else {
      xhr.open('POST', _vise_server_uri + '/' + name + '/' + version + '/' + cmd + '?' + uri_param.join('&'));
    }

    if ( typeof(payload) === 'undefined' ) {
      xhr.send();
    } else {
      xhr.send(payload);
    }
  });
}

//
// ui state maintainers
//
function on_new_search_engine_file_add() {
  console.log('updating dropdown');
  var e = document.getElementById('new_search_engine_select_preview_image');
  e.innerHTMl = '';
  var i, n;
  n = _vise_new_search_engine.filename_list.length;
  for ( i = 0; i < n; ++i ) {
    var o = document.createElement('option');
    o.setAttribute('value', i);
    o.innerHTML = '[' + (i+1) + '] ' + _vise_new_search_engine.filename_list[i];
    e.appendChild(o);
  }
  e.selectedIndex = 0;
  update_new_search_engine_preview_image(0);
}

function on_new_search_engine_filename_dropdown_update(e) {
  var index = e.options[e.selectedIndex].value;
  update_new_search_engine_preview_image(index);
}

function update_new_search_engine_preview_image(index) {
  var filereader = new FileReader();
  filereader.addEventListener( 'load', function() {
    document.getElementById('new_search_engine_preview_image').src = filereader.result;
  });
  filereader.readAsDataURL( _vise_new_search_engine.files[index] );
}

//
// misc
//

function show_message(msg, t) {
  if ( _vise_message_clear_timer ) {
    clearTimeout(_vise_message_clear_timer); // stop any previous timeouts
  }
  var timeout = t;
  if ( typeof t === 'undefined' ) {
    timeout = VISE_THEME_MESSAGE_TIMEOUT_MS;
  }
  document.getElementById('message_panel_content').innerHTML = msg;
  document.getElementById('message_panel').style.display = 'block';

  _vise_message_clear_timer = setTimeout( function() {
    document.getElementById('message_panel').style.display = 'none';
  }, timeout);
}


function generate_search_engine_rand_name() {
  //source: http://stackoverflow.com/questions/105034/how-to-create-a-guid-uuid-in-javascript
  return Math.random().toString(36).substring(2, 15) + Math.random().toString(36).substring(2, 15);
  //return new Date();
}
