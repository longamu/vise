/*
  @file        _vise_filelist.js
  @description display the internal files of search engine in VISE
  @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
  @date        1 August 2018
*/

/*
precondition: variable '_vise_filelist_str' contains the JSON string
representation of VISE search engine filelist in a format given below:

GET http://localhost:9973/vise/query/ox5k/1/_filelist?from=0&result_count=3&format=json

{
  "search_engine_id":"ox5k/1",
  "filelist_count": 5063,
  "image_uri_prefix": "/vise/asset/ox5k/1/",
  "home_uri": "/vise/home.html",
  "image_uri_namespace": "image/",
  "query_uri_prefix": "/vise/query/ox5k/1/",
  "from": 0,
  "filename_regex": "",
  "result_count": 3,
  "filelist_subset": [
    {
      "file_id": 0,
      "filename": "all_souls_000000.jpg"
    },
    {
      "file_id": 1,
      "filename": "all_souls_000001.jpg"
    },
    {
      "file_id": 2,
      "filename": "all_souls_000002.jpg"
    }
  ]
}
*/

function _vise_filelist() {
  // _vise_filelist_result_str is a global variable set by the vise_server
  if ( typeof(_vise_filelist_str) === 'undefined' ) {
    console.log('Error: _vise_filelist_str is not defined');
    return;
  }

  // _vise_filelist is a global variable
  _vise_filelist = JSON.parse(_vise_filelist_str);
  console.log(_vise_filelist); // debug

  var page = document.createElement('div');
  var nav = document.createElement('div');
  var filelist = document.createElement('div');
  page.setAttribute('id', 'filelist_page');
  page.classList.add('page');
  nav.classList.add('nav');
  filelist.classList.add('result')
  page.appendChild(nav);
  page.appendChild(filelist);

  _vise_filelist_init_nav(_vise_filelist, nav);
  _vise_filelist_show_all_files(_vise_filelist, filelist);

  document.body.appendChild(page);
}

function _vise_filelist_init_nav(d, nav) {
  nav.innerHTML  = '<a href="' + d.home_uri + '" title="VISE home page which shows a list of available search engines">Home</a>';
  nav.innerHTML += '<span class="left_space">' + d.search_engine_id + ' : </span>';
  nav.innerHTML += '<a href="" title="Search using image the search engine dataset">Search</a> | ';
  nav.innerHTML += '<a href="" title="Upload a new image and search using this image">Upload & Search</a> | ';
}

function _vise_filelist_show_all_files(d, content_panel) {
  var navbar = document.createElement('div');
  navbar.classList.add('navbar');
  content_panel.appendChild(navbar);

  // info. about filelist
  var navinfo = document.createElement('div');
  navinfo.classList.add('navinfo');
  navbar.appendChild(navinfo);
  navinfo.innerHTML = "Total " + d.filelist_count + " images. Showing " + d.from + " to " + (d.from + d.filelist_subset.length);

  // filelist filter input
  var navtool = document.createElement('div');
  navtool.classList.add('navtool');
  navbar.appendChild(navtool);
  navtool.innerHTML = '<input type="text" title="type partial filename in order to show all files matching that keyword" id="filename_regex" placeholder="type partial filename to filter files">';

  // filelist prev,next buttons
  var navbuttons = document.createElement('div');
  navbuttons.classList.add('navbuttons');
  navbar.appendChild(navbuttons);
  var links = [];
  if ( d.from !== 0 ) {
    links.push( '<a href="' + _vise_filelist_now_get_prev_uri(d) + '" title="Prev">Prev</a>' );
  } else {
    links.push( '<span>Prev</span>' );
  }
  if ( d.filelist_subset.length === d.result_count ) {
    links.push( '<a href="' + _vise_filelist_now_get_next_uri(d) + '" title="Next">Next</a>' );
  } else {
    links.push( '<span>Next</span>' );
  }
  navbuttons.innerHTML = links.join('&nbsp;|&nbsp;');

  // clear floats
  var float_clear = document.createElement('div');
  float_clear.setAttribute('style', 'clear:both;');
  navbar.appendChild(float_clear);

  var img_grid = document.createElement('div');
  img_grid.classList.add('img_grid');

  var i, n, ri;
  n = d.filelist_subset.length;
  for ( i = 0; i < n; ++i ) {
    _vise_filelist_show_file_i(d, i, img_grid);
  }
  content_panel.appendChild(img_grid)
}

function _vise_filelist_show_file_i(d, i, content_panel) {
  var img_with_region = document.createElement('div');
  img_with_region.classList.add('img_with_region');
  img_with_region.classList.add('cursor_pointer');
  img_with_region.setAttribute('onclick', '@todo');

  var img = document.createElement('img');
  img.setAttribute('src', d.image_uri_prefix + d.image_uri_namespace + d.filelist_subset[i].filename);
  img_with_region.appendChild(img);
  content_panel.appendChild(img_with_region);
}

//
// page navigation utils
//
function _vise_filelist_now_get_filelist_uri(d) {
  var uri = d.query_uri_prefix + '_filelist?';
  if ( typeof(d.filename_regex) !== 'undefined' && d.filename_regex !== '' ) {
    uri += 'filename_regex=' + d.filename_regex;
  }
  return uri;
}

function _vise_filelist_now_get_next_uri(d) {
  var uri = [];
  uri.push('from=' + (parseInt(d.from) + parseInt(d.result_count)) );
  uri.push('result_count=' + d.result_count );
  return _vise_filelist_now_get_filelist_uri(d) + uri.join('&');
}

function _vise_filelist_now_get_prev_uri(d) {
  var uri = [];
  uri.push('from=' + (parseInt(d.from) - parseInt(d.result_count)) );
  uri.push('result_count=' + d.result_count );
  return _vise_filelist_now_get_filelist_uri(d) + uri.join('&');
}

