/*
  @file        _via_common.js
  @description methods shared by all VISE pages
  @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
  @date        22 June 2017
*/

function _vise_init_top_nav(d, nav) {
  var html = [];
//  nav.innerHTML  = '<a href="' + d.home_uri + '" title="VISE home page which shows a list of available search engines">VISE Home</a>';
  var search_engine_id = '<span>' + d.search_engine_id + ' : </span>';
  var search_engine_home_url = d.query_uri_prefix + '_filelist?from=0&count=1024&show_from=0&show_count=45';
  html.push( '<a href="' + search_engine_home_url + '" title="Search using image the search engine dataset">Image List</a>' );

  nav.innerHTML = search_engine_id + html.join('&nbsp;|&nbsp;');
}

function _vise_save_data_as_local_file(data, filename) {
  var a      = document.createElement('a');
  a.href     = URL.createObjectURL(data);
  a.target   = '_blank';
  a.download = filename;

  // simulate a mouse click event
  var event = new MouseEvent('click', {
    view: window,
    bubbles: true,
    cancelable: true
  });

  a.dispatchEvent(event);
}

