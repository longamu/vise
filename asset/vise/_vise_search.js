/*
  @file        _vise_search_result.js
  @description display search results from VISE
  @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
  @date        20 July 2018
*/

/*
precondition: variable '_vise_search_data' contains the JSON string
representation of VISE search result in a format given below:
  {
    "search_engine_id": "ox5k/1",
    "query": {
      "file_id": 2,
      "x": 156,
      "y": 228,
      "w": 316,
      "h": 502,
      "from": 0,
      "to": 4,
      "score_threshold": 0
    },
    "image_uri_prefix": "/vise/asset/ox5k/1/image/",
    "query_result": [
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
  if ( typeof(_vise_search_data) === 'undefined' ) {
    console.log('Error: _vise_search_result_data is not defined');
    return;
  }

  var d = JSON.parse(_vise_search_data);
  console.log(d);

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

  _vise_search_init_nav(d, nav);
  _vise_search_show_query(d, query);
  _vise_search_show_all_result(d, result);

  document.body.appendChild(page);
}

function _vise_search_init_nav(d, nav) {
  nav.innerHTML  = '<a href="' + d.home_uri + '">Home</a>';
  nav.innerHTML += '<span class="left_space">' + d.search_engine_id + ' : </span>';
  nav.innerHTML += '<a href="" title="Search using image the search engine dataset">Search</a> | ';
  nav.innerHTML += '<a href="" title="Upload a new image and search using this image">Upload & Search</a> | ';
}

function _vise_search_show_all_result(d, content_panel) {
  var p = document.createElement('p');
  p.innerHTML = "Search result contains all images matching the query : " + d.query.from + " to " + (d.query.from + d.query.result_count);
  content_panel.appendChild(p);

  var i, n, ri;
  n = d.query_result.length;
  for ( i = 0; i < n; ++i ) {
    _vise_search_show_result_i(d, i, content_panel);
  }
}

function _vise_search_show_result_i(d, i, content_panel) {
  var img_with_region = document.createElement('div');
  img_with_region.classList.add('img_with_region');

  var img = document.createElement('img');
  img.setAttribute('src', d.image_uri_prefix + d.image_uri_namespace + d.query_result[i].filename);
  img.addEventListener('load', function(e) {
    // automatically set viewBox attribute of SVG element so that
    // region defined in original image space is correctly scaled
    // for resized image
    this.nextElementSibling.setAttributeNS(null, 'viewBox', '0 0 ' + this.naturalWidth + ' ' + this.naturalHeight);
  });
  img_with_region.appendChild(img);

  var query_region = [ d.query.x, d.query.y, d.query.w, d.query.h ];
  var dimg = _vise_search_tx_rect_using_homography(query_region, d.query_result[i].H);
  img_with_region.appendChild( _vise_search_svg_region('polygon', dimg) );

/*
  var score = document.createElement('span');
  score.innerHTML = d.query_result[i].score;
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
  });

  var dimg = [ d.query.x, d.query.y, d.query.w, d.query.h ];

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
  var dimg = [ d.query.x, d.query.y, d.query.w, d.query.h ];
  metadata.innerHTML += '<div class="row"><span class="col">Region</span><span class="col">[' + dimg.join(', ') + ']</span></div>';
  return metadata;
}

function _vise_search_svg_region(shape, dimg) {
  var SVG_NS = "http://www.w3.org/2000/svg";
  var svg = document.createElementNS(SVG_NS, shape);

  switch(shape) {
    case 'rect':
      svg.setAttributeNS(null, 'x', dimg[0]);
      svg.setAttributeNS(null, 'y', dimg[1]);
      svg.setAttributeNS(null, 'width', dimg[2]);
      svg.setAttributeNS(null, 'height', dimg[3]);
      break;
    case 'polygon':
      var pts = [];
      var i, n;
      n = dimg.length;
      for ( i = 0 ; i < n; i = i+2) {
        pts.push( dimg[i] + ',' + dimg[i+1] );
      }
      svg.setAttributeNS(null, 'points', pts.join(' ') );
      break;
  }
  var svg_container = document.createElementNS(SVG_NS, 'svg');
  svg_container.appendChild(svg);
  return svg_container;
}
