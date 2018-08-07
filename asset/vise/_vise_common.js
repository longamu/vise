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
  html.push( '<a href="" title="Search using image the search engine dataset">Image List</a>' );

  nav.innerHTML = search_engine_id + html.join('&nbsp;|&nbsp;');
}

