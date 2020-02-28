#ifndef VISE_EXPORT_HTML_TEXT
#define VISE_EXPORT_HTML_TEXT

const char *VISE_HTML_EXPORT_HEAD_STR = R"TEXT(<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <title>Ashmolean Image Archive</title>
    <meta name="author" content="vise">
    <meta name="description" content="vise result exported as HTML using --export-html flag">
    <style>
body { font-family:sans; background-color:white; margin:1ch; font-size:12pt; }
input[type="text"] { width:2em; }
img { }
.item { border:1px solid #cccccc; margin:2em 1em; padding:0.4em; }
.item div { display:block; }
figure { display:inline-block; margin:1em; }
    </style>
  </head>
)TEXT";

const char *VISE_HTML_EXPORT_TAIL_STR = R"TEXT(</html>)TEXT";

const char *VISE_HTML_EXPORT_MATCH_JS_STR = R"TEXT(<script>
function vise_init_html_ui() {
  init_toolbar();
}

function init_toolbar() {
  var toolbar = document.getElementById('toolbar');
  toolbar.innerHTML = '';

  // selector for number of images in a set
  var match_size_list = Object.keys(match_stat);
  this.match_size_dropdown = document.createElement('select');
  for(var i=0; i<match_size_list.length; ++i) {
    var oi = document.createElement('option');
    oi.innerHTML = 'Match set containing ' + match_size_list[i] + ' images';
    oi.setAttribute('value', match_size_list[i]);
    this.match_size_dropdown.appendChild(oi);
  }
  this.match_size_dropdown.addEventListener('change', match_size_dropdown_onchage.bind(this));

  var label_match_size = document.createElement('span');
  label_match_size.innerHTML = 'Select a subset &nbsp;';
  toolbar.appendChild(label_match_size);
  toolbar.appendChild(match_size_dropdown);

  this.match_set_stat = document.createElement('span');
  this.match_set_stat.innerHTML = '&nbsp;';
  toolbar.appendChild(this.match_set_stat);

  this.pageno_dropdown = document.createElement('select');
  pageno_dropdown.setAttribute('title', 'Jump to page number');
  var label_pageno = document.createElement('span');
  label_pageno.innerHTML = '&nbsp;Jump to&nbsp;';
  toolbar.appendChild(label_pageno);
  toolbar.appendChild(this.pageno_dropdown);

  this.set_perpage_input = document.createElement('input');
  this.set_perpage_input.setAttribute('type', 'text');
  this.set_perpage_input.setAttribute('value', '10');
  this.set_perpage_input.setAttribute('title', 'Number of entries per page');
  this.set_perpage_input.addEventListener('change', function(e) {
    this.reload_current_page();
  }.bind(this));
  var label_perpage = document.createElement('span');
  label_perpage.innerHTML = 'Entries per page&nbsp;';
  label_perpage.setAttribute('style', 'margin-left:1em;');
  toolbar.appendChild(label_perpage);
  toolbar.appendChild(this.set_perpage_input);

  this.imsize_input = document.createElement('input');
  this.imsize_input.setAttribute('type', 'text');
  this.imsize_input.setAttribute('value', '50');
  this.imsize_input.setAttribute('title', 'Image Width (in &percnt;)');

  this.imsize_input.addEventListener('change', function(e) {
    this.reload_current_page();
  }.bind(this))
  var label_imsize = document.createElement('span');
  label_imsize.setAttribute('style', 'margin-left:1em;');
  label_imsize.innerHTML = 'Image width (in &percnt;)&nbsp;';
  var label_percent = document.createElement('span');

  toolbar.appendChild(label_imsize);
  toolbar.appendChild(this.imsize_input);

  if(match_size_list.length) {
    this.match_size_dropdown.selectedIndex = 0;
    set_match_size(match_size_list[0]);
  } else {
    match_size_dropdown.selectedIndex = -1;
  }
}

function match_size_dropdown_onchage(e) {
  var match_size = e.target.options[e.target.selectedIndex].value;
  set_match_size(match_size);
}

function set_match_size(match_size) {
  this.match_set_stat.innerHTML = '&nbsp; Subset contains ' + match_stat[match_size].length + ' entries.&nbsp;';

  var entries_per_page = parseInt(this.set_perpage_input.value);
  var page_count = Math.ceil(match_stat[match_size].length / entries_per_page);
  this.pageno_dropdown.innerHTML = '';
  for(var pageno=0; pageno < page_count; ++pageno) {
    var oi = document.createElement('option');
    oi.innerHTML = 'Page ' + (pageno + 1);
    oi.setAttribute('value', pageno);
    oi.setAttribute('data-matchsize', match_size);
    this.pageno_dropdown.appendChild(oi);
  }
  this.pageno_dropdown.addEventListener('change', function(e) {
    var oi = e.target.options[e.target.selectedIndex];
    var match_size = oi.dataset.matchsize;
    var pageno = oi.value;
    show_page(match_size, pageno);
  }.bind(this));
  if(page_count > 0) {
    this.pageno_dropdown.selectedIndex = 0;
    show_page(match_size, 0); // show first page by default
  } else {
    this.pageno_dropdown.selectedIndex = -1;
  }
}

function reload_current_page() {
  this.show_page(this.current_match_size, this.current_pageno);
}

function show_page(match_size, pageno) {
  this.current_match_size = match_size;
  this.current_pageno = pageno;
  this.current_imsize = parseFloat(this.imsize_input.value);

  var entries_per_page = parseInt(this.set_perpage_input.value);
  var start = entries_per_page * pageno;
  var end = Math.min(start + entries_per_page, match_stat[match_size].length);
  var content = document.getElementById('content');
  content.innerHTML = '';
  for( var i=start; i < end; ++i ) {
    var match_index = match_stat[match_size][i];
    var match_query = [match_index];
    var match = match_query.concat(match_list[match_index].slice(0));
    var matchdiv = document.createElement('div');
    matchdiv.setAttribute('class', 'item');
    var matchdivtitle = document.createElement('div');
    matchdivtitle.innerHTML = '[' + (i+1) + ']';
    matchdiv.appendChild(matchdivtitle);
    for ( var j=0; j < match.length; ++j) {
      var img = document.createElement('img');
      img.setAttribute('src', target_dir + flist[ match[j] ]);
      img.setAttribute('width', this.current_imsize + '%');
      img.setAttribute('title', flist[ match[j] ]);
      var fig = document.createElement('figure');
      fig.appendChild(img);

      var figcap = document.createElement('figcaption');
      figcap.innerHTML = flist_original[ match[j] ];
      fig.appendChild(figcap);
      matchdiv.appendChild(fig);
    }
    content.appendChild(matchdiv);
  }
}

</script>)TEXT";

const char *VISE_HTML_EXPORT_COLLAGE_HEAD_STR = R"TEXT(<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <style>
img {  }
    </style>
  </head>
  <body>
)TEXT";

#endif
