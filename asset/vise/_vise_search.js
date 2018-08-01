/*
  @file        _vise_search_result.js
  @description display search results from VISE
  @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
  @date        20 July 2018
*/

/*
precondition: variable '_vise_search_result_str' contains the JSON string
representation of VISE search result in a format given below:
  {
    "search_engine_id": "ox5k/1",
    "query": {
      "file_id": 2,
      "x": 156,
      "y": 228,
      "width": 316,
      "height": 502,
      "from": 0,
      "result_count": 4
    },
    "image_uri_prefix": "/vise/asset/ox5k/1/image/",
    "home_uri":"/vise/home.html",
    "image_uri_prefix:"/vise/asset/ox5k/1/",
    "image_uri_namespace":"image/",
    "query_uri_prefix":""/vise/query/ox5k/1/",
    "query_result_count":50,
    "query_result_subset": [
      {
        "file_id": 30,
        "filename": "all_souls_000040.jpg",
        "metadata": "",
        "score": 354,
        "H": [1,0,0,0,1,0,0,0,1]
      }, ...
    ]
  }
*/

function _vise_search() {
  // _vise_search_result_str is a global variable set by the vise_server
  if ( typeof(_vise_search_result_str) === 'undefined' ) {
    console.log('Error: _vise_search_result_data is not defined');
    return;
  }

  // _vise_search_result is a global variable
  _vise_search_result = JSON.parse(_vise_search_result_str);
  console.log(_vise_search_result); // debug

  var page = document.createElement('div');
  var nav = document.createElement('div');
  var query_container = document.createElement('div');
  var query = document.createElement('div');
  var result = document.createElement('div');
  page.setAttribute('id', 'search_result_page');
  page.classList.add('page');
  nav.classList.add('nav');
  query_container.classList.add('query_container');
  query.classList.add('query')
  result.classList.add('result')
  page.appendChild(nav);
  query_container.appendChild(query);
  page.appendChild(query_container);
  page.appendChild(result);

  _vise_search_init_nav(_vise_search_result, nav);
  _vise_search_show_query(_vise_search_result, query);

  _vise_search_show_all_result(_vise_search_result, result);

  document.body.appendChild(page);
  //_vise_search_show_match_detail(0);
}

function _vise_search_init_nav(d, nav) {
  nav.innerHTML  = '<a href="' + d.home_uri + '">Home</a>';
  nav.innerHTML += '<span class="left_space">' + d.search_engine_id + ' : </span>';
  nav.innerHTML += '<a href="" title="Search using image the search engine dataset">Search</a> | ';
  nav.innerHTML += '<a href="" title="Upload a new image and search using this image">Upload & Search</a> | ';
}

function _vise_search_show_all_result(d, content_panel) {
  var navbar = document.createElement('div');
  navbar.classList.add('navbar');
  navbar.innerHTML = "Total " + d.query_result_count + " images match the query region. Showing " + d.query.from + " to " + (d.query.from + d.query_result_subset.length);

  var navtool = document.createElement('span');
  navtool.classList.add('navtool');
  var links = [];
  if ( d.query.from !== 0 ) {
    links.push( '<a href="' + _vise_search_now_get_prev_uri(d) + '" title="Prev">Prev</a>' );
  }
  if ( d.query_result_subset.length === d.query.result_count ) {
    links.push( '<a href="' + _vise_search_now_get_next_uri(d) + '" title="Next">Next</a>' );
  }
  navtool.innerHTML = links.join('&nbsp;|&nbsp;');

  navbar.appendChild(navtool);
  content_panel.appendChild(navbar);

  var img_grid = document.createElement('div');
  img_grid.classList.add('img_grid');

  var i, n, ri;
  n = d.query_result_subset.length;
  for ( i = 0; i < n; ++i ) {
    _vise_search_show_result_i(d, i, img_grid);
  }
  content_panel.appendChild(img_grid)
}

function _vise_search_show_result_i(d, i, content_panel) {
  var img_with_region = document.createElement('div');
  img_with_region.classList.add('img_with_region');
  img_with_region.classList.add('cursor_pointer');
  img_with_region.setAttribute('onclick', '_vise_search_show_match_detail(' + i + ')');

  var img = document.createElement('img');
  img.setAttribute('src', d.image_uri_prefix + d.image_uri_namespace + d.query_result_subset[i].filename);
  img.addEventListener('load', function(e) {
    // automatically set viewBox attribute of SVG element so that
    // region defined in original image space is correctly scaled
    // for resized image
    this.nextElementSibling.setAttributeNS(null, 'viewBox', '0 0 ' + this.naturalWidth + ' ' + this.naturalHeight);
    this.nextElementSibling.style.display = 'block'; // by default, the region is invisible. Now, make the region visible
  });
  img_with_region.appendChild(img);

  var query_region = [ d.query.x, d.query.y, d.query.width, d.query.height ];
  var dimg = _vise_search_tx_rect_using_homography(query_region, d.query_result_subset[i].H);
  img_with_region.appendChild( _vise_search_svg_region('polygon', dimg) );

/*
  var score = document.createElement('span');
  score.innerHTML = d.query_result_subset[i].score;
  img_with_region.appendChild(score);
*/
  content_panel.appendChild(img_with_region);
}

function _vise_search_tx_rect_using_homography(drect, H) {
  var points = [];
  var p 
  p = _vise_search_tx_point_using_homography(drect[0], drect[1], H);
  points.push(p[0]);
  points.push(p[1]);
  p = _vise_search_tx_point_using_homography(drect[0], drect[1] + drect[3], H);
  points.push(p[0]);
  points.push(p[1]);
  p = _vise_search_tx_point_using_homography(drect[0] + drect[2], drect[1] + drect[3], H);
  points.push(p[0]);
  points.push(p[1]);
  p = _vise_search_tx_point_using_homography(drect[0] + drect[2], drect[1], H);
  points.push(p[0]);
  points.push(p[1]);
  return points;
}

function _vise_search_tx_point_using_homography(x, y, H) {
  var p = [];
  p[0] = H[0]*x + H[1]*y + H[2];
  p[1] = H[3]*x + H[4]*y + H[5];
  return p;
}

function _vise_search_show_query(d, content_panel) {
  var img_with_region = document.createElement('div');
  img_with_region.classList.add('img_with_region');

  var img = document.createElement('img');
  img.setAttribute('src', d.image_uri_prefix + d.image_uri_namespace + d.query.filename);
  img.addEventListener('load', function(e) {
    // automatically set viewBox attribute of SVG element so that
    // the correct region is visible
    this.nextElementSibling.setAttributeNS(null, 'viewBox', '0 0 ' + this.naturalWidth + ' ' + this.naturalHeight);
    this.nextElementSibling.style.display = 'block'; // by default, the region is invisible. Now, make the region visible
  });

  var dimg = [ d.query.x, d.query.y, d.query.width, d.query.height ];

  img_with_region.appendChild(img);
  img_with_region.appendChild( _vise_search_svg_region('rect', dimg) );

  var query_image = document.createElement('div');
  query_image.classList.add('query_image');
  query_image.appendChild(img_with_region);

  content_panel.appendChild( _vise_search_show_query_metadata(d) );
  content_panel.appendChild( query_image );
}

function _vise_search_show_query_metadata(d) {
  var metadata = document.createElement('div');
  metadata.classList.add('metadata');
  metadata.innerHTML  = '<p>Search Query</p>';
  metadata.innerHTML += '<div class="row"><span class="col">Filename</span><span class="col"><a href="">' + d.query.filename + '</a></span></div>';
  var dimg = [ d.query.x, d.query.y, d.query.width, d.query.height ];
  metadata.innerHTML += '<div class="row"><span class="col">Region</span><span class="col" title="x, y, width, height">[' + dimg.join(', ') + ']</span></div>';
  return metadata;
}

function _vise_search_svg_region(shape, dimg) {
  var SVG_NS = 'http://www.w3.org/2000/svg';
  var svg = document.createElementNS(SVG_NS, shape);

  switch(shape) {
    case 'rect':
      svg.setAttributeNS(null, 'x', Math.floor(dimg[0]));
      svg.setAttributeNS(null, 'y', Math.floor(dimg[1]));
      svg.setAttributeNS(null, 'width', Math.floor(dimg[2]));
      svg.setAttributeNS(null, 'height', Math.floor(dimg[3]));
      break;
    case 'polygon':
      var pts = [];
      var i, n;
      n = dimg.length;
      for ( i = 0 ; i < n; i = i+2) {
        pts.push( Math.floor(dimg[i]) + ',' + Math.floor(dimg[i+1]) );
      }
      svg.setAttributeNS(null, 'points', pts.join(' ') );
      break;
  }
  var svg_container = document.createElementNS(SVG_NS, 'svg');
  svg_container.appendChild(svg);
  return svg_container;
}

function _vise_search_hide_detail() {
  var page_overlay = document.getElementById('page_overlay');
  if ( page_overlay ) {
    page_overlay.style.display = 'none';
    page_overlay.parentNode.removeChild(page_overlay);
    document.body.classList.remove('noscroll');
  }
}

function _vise_search_show_match_detail(match_index) {
  var page_overlay = document.createElement('div');
  page_overlay.setAttribute('id', 'page_overlay');
  page_overlay.style.display = 'block';
  page_overlay.addEventListener('click', function(e) {
    // click on the outside overlay region hides the overlay
    if ( e.target.id === 'page_overlay' ) {
      _vise_search_hide_detail();
    }
  });

  var content = document.createElement('div');
  page_overlay.appendChild(content);
  content.classList.add('content');

  var toolbar = document.createElement('div');
  toolbar.classList.add('toolbar');
  content.appendChild(toolbar);
  toolbar.innerHTML = '<span class="text_button" onclick="_vise_search_hide_detail()">&times;</span>';
  
  _vise_search_show_match_pair_comparison(match_index, content);

  document.body.classList.add('noscroll');
  document.body.appendChild(page_overlay);
}

function _vise_search_show_match_pair_comparison(match_index, content) {
  if ( typeof(_vise_search_result) === 'undefined' ) {
    console.log('_vise_search_result is not defined');
    return;
  }

  var compare_panel = document.createElement('div');
  content.appendChild(compare_panel);
  compare_panel.classList.add('compare_panel');

  // caption row
  var r1 = document.createElement('div');
  compare_panel.appendChild(r1);
  r1.classList.add('row');
  var r1c1 = document.createElement('span');
  r1.appendChild(r1c1);
  r1c1.classList.add('col');
  r1c1.classList.add('col_query');
  r1c1.innerHTML = 'Query';
  var r1c2 = document.createElement('span');
  r1.appendChild(r1c2);
  r1c2.classList.add('col');
  r1c2.classList.add('col_compare');
  r1c2.innerHTML = 'Comparison';
  var r1c3 = document.createElement('span');
  r1.appendChild(r1c3);
  r1c3.classList.add('col');
  r1c3.classList.add('col_match');
  r1c3.innerHTML = 'Match';

  // image row
  var r2 = document.createElement('div');
  compare_panel.appendChild(r2);
  r2.classList.add('row');

  // left: query
  var r2c1 = document.createElement('span');
  r2.appendChild(r2c1);
  r2c1.classList.add('query');
  var qsrc = _vise_search_result.image_uri_prefix + _vise_search_result.image_uri_namespace + _vise_search_result.query.filename;
  _vise_search_set_cropped_img(r2c1, qsrc, _vise_search_result.query.x, _vise_search_result.query.y, _vise_search_result.query.width, _vise_search_result.query.height)

  // center: compare element
  var r2c2 = document.createElement('span');
  r2.appendChild(r2c2);
  r2c2.classList.add('compare');
  _vise_search_set_compare_element(r2c2);

  // right: match
  var r2c3 = document.createElement('span');
  r2.appendChild(r2c3);
  r2c3.classList.add('match');
  var msrc = _vise_search_result.image_uri_prefix + _vise_search_result.image_uri_namespace + _vise_search_result.query_result_subset[match_index].filename;
  _vise_search_set_transform_and_cropped_img(r2c3, msrc, _vise_search_result.query.x, _vise_search_result.query.y, _vise_search_result.query.width, _vise_search_result.query.height, _vise_search_result.query_result_subset[match_index].H);

  // filename row
  var r3 = document.createElement('div');
  compare_panel.appendChild(r3);
  r3.classList.add('row');
  var r3c1 = document.createElement('span');
  r3.appendChild(r3c1);
  r3c1.classList.add('col');
  r3c1.innerHTML = '[' + _vise_search_result.query.file_id + '] ' + _vise_search_result.query.filename;
  var r3c2 = document.createElement('span');
  r3.appendChild(r3c2);
  r3c2.classList.add('col');
  r3c2.innerHTML = 'Click on the image to flip and compare';
  var r3c3 = document.createElement('span');
  r3.appendChild(r3c3);
  r3c3.classList.add('col');
  r3c3.innerHTML  = '[' + _vise_search_result.query_result_subset[match_index].file_id + '] ' + _vise_search_result.query_result_subset[match_index].filename;
  r3c3.innerHTML += '<br/>( score = ' + _vise_search_result.query_result_subset[match_index].score + ' )';
}

function _vise_search_get_img_element(src, id, classname, alt, title) {
  var p = document.createElement('img');
  p.setAttribute('src', src);

  if ( id ) {
    p.setAttribute('id', id);
  }

  if ( classname ) {
    p.classList.add(classname);
  }

  if ( alt ) {
    p.setAttribute('alt', alt);
  }

  if ( title ) {
    p.setAttribute('title', title);
  }
  return p;
}

function _vise_search_set_compare_element(container) {
  var left = document.getElementById('compare_detail_left_canvas');
  var right = document.getElementById('compare_detail_right_canvas');

  if ( !left || !right ) {
    // retry after some delay
    setTimeout( function() {
      _vise_search_set_compare_element(container);
    }.bind(this), 500);
    return;
  }

  var c = document.createElement('canvas');
  c.setAttribute('id', 'compare_detail_center_canvas');
  c.setAttribute('style', 'cursor:pointer;');
  var cx = c.getContext('2d');
  c.height = left.height;
  c.width  = left.width;
  cx.drawImage(left, 0, 0);
  c.addEventListener('mousedown', function(e) {
    cx.drawImage(right, 0, 0);
  });
  c.addEventListener('mouseup', function(e) {
    cx.drawImage(left, 0, 0);
  });
  container.appendChild(c);
}

function _vise_search_set_cropped_img(element, img_src, x, y, width, height) {
  var img = new Image();
  img.addEventListener('load', function(e) {
    var c = document.createElement('canvas');
    c.setAttribute('id', 'compare_detail_left_canvas');
    var cx = c.getContext('2d');
    c.height = height;
    c.width  = width;
    cx.drawImage(img, x, y, width, height, 0, 0, width, height);
    element.appendChild(c);
  });
  img.src = img_src;
}

function _vise_search_set_transform_and_cropped_img(element, img_src, x, y, width, height, H) {
  var img = new Image();
  img.addEventListener('load', function(e) {
    // draw transformed image in canvas 'c'
    var Hinv = _vise_homography_inverse(H);
    var c = document.createElement('canvas');
    var cx = c.getContext('2d', { alpha: false });
    c.height = img.naturalWidth;
    c.width  = img.naturalHeight;
    cx.transform(Hinv[0], Hinv[3], Hinv[1], Hinv[4], Hinv[2], Hinv[5]);
    cx.drawImage(img, 0, 0)

    // crop rectangular region from canvas 'c'
    var c2 = document.createElement('canvas', { alpha: false });
    c2.setAttribute('id', 'compare_detail_right_canvas');
    var cx2 = c2.getContext('2d');
    c2.width = width;
    c2.height = height;
    cx2.drawImage(c, x, y, width, height, 0, 0, width, height)
    element.appendChild(c2);
  });
  img.src = img_src;
}

//
// page navigation utils
//
function _vise_search_now_get_search_uri(d) {
  var uri = [];
  uri.push(d.query_uri_prefix + '_search?' + 'file_id=' + d.query.file_id);
  uri.push('region=' + d.query.x + ',' + d.query.y + ',' + d.query.width + ',' + d.query.height);
  return uri;
}

function _vise_search_now_get_next_uri(d) {
  var uri = _vise_search_now_get_search_uri(d);
  uri.push('from=' + (parseInt(d.query.from) + parseInt(d.query.result_count)) );
  uri.push('result_count=' + d.query.result_count );
  return uri.join('&');
}

function _vise_search_now_get_prev_uri(d) {
  var uri = _vise_search_now_get_search_uri(d);
  uri.push('from=' + (parseInt(d.query.from) - parseInt(d.query.result_count)) );
  uri.push('result_count=' + d.query.result_count );
  return uri.join('&');
}

//
// affine homography utils
//
function _vise_homography_inverse(H) {
  var invdet = H[0]*H[4]*H[8] - H[0]*H[5]*H[7] - H[3]*H[1]*H[8] + H[3]*H[2]*H[7] + H[6]*H[1]*H[5] - H[6]*H[2]*H[4];
  invdet = 1.0 / invdet;
  var Hinv = [];
  Hinv[0]=  (H[4]*H[8] - H[5]*H[7])*invdet;
  Hinv[1]= -(H[1]*H[8] - H[2]*H[7])*invdet;
  Hinv[2]=  (H[1]*H[5] - H[2]*H[4])*invdet;
  Hinv[3]= -(H[3]*H[8] - H[5]*H[6])*invdet;
  Hinv[4]=  (H[0]*H[8] - H[2]*H[6])*invdet;
  Hinv[5]= -(H[0]*H[5] - H[2]*H[3])*invdet;
  Hinv[6]=  (H[3]*H[7] - H[4]*H[6])*invdet;
  Hinv[7]= -(H[0]*H[7] - H[1]*H[6])*invdet;
  Hinv[8]=  (H[0]*H[4] - H[1]*H[3])*invdet;
  return _vise_homography_normalize(Hinv); // normalize such that H[8] = 1.0
}

function _vise_homography_normalize(H) {
  var h = H.slice(0);
  h[0]/= h[8]; h[1]/= h[8]; h[2]/= h[8];
  h[3]/= h[8]; h[4]/= h[8]; h[5]/= h[8];
  h[6]/= h[8]; h[7]/= h[8]; h[8]= 1.0;
  return h;
}
