##
## VISE Customized for 15th Century Booktrade Project
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 29 Jan. 2018
##


import cherrypy;

import random;
import template;
import copy;
import collections;

import pandas as pd;
import numpy as np;

class text_search:

  def __init__(self, pageTemplate, file_attributes):
    self.pT = pageTemplate;
    self.file_attributes = file_attributes;

  def search_istc_metadata(self, keyword):
    # id,author,title,imprint,format
    ## iterate through each column
    N = self.file_attributes.istc_db.shape[0];
    matches = None;
    for col in self.file_attributes.istc_db.columns:
      matches += self.file_attributes.istc_db[col].str.contains(keyword, case=False, na=False, regex=True);
    print(matches)
    return matches;

  @cherrypy.expose
  def index(self, keyword=None, target="all", numberToReturn=20, startFrom=0, tile=None, noText=None):
    print 'search keyword = %s, target = %s' %(keyword, target);
    body = '';
    result_count = int(numberToReturn);
    result_start = int(startFrom);

    if tile is None:
      tile = True;
    else:
      tile = False;
    if noText is None:
      noText = False;
    else:
      noText = True;

    # filename,file_size,file_attributes,region_count,region_id,region_shape_attributes,region_attributes
    results = self.file_attributes.region_attributes['region_attributes'].str.contains(keyword, case=False, na=False, regex=True);
    match_region_attributes = self.file_attributes.region_attributes[results];

    # switch between tiled and list views
    search_result_page_tools = '';
    listview_url = 'text_search?keyword=%s&target=%s&numberToReturn=%d&startFrom=%d' % (keyword, target, result_count, result_start);
    tileview_url = listview_url + '&tile=true';
    tileview_notext_url = tileview_url + '&noText';

    if tile:
        if noText:
            search_result_page_tools = '<li><a href="%s">List View</a></li><li><a href="%s">Tile View</a></li><li>Tile View (images only)</li>' % (listview_url, tileview_url);
        else:
            search_result_page_tools = '<li><a href="%s">List View</a></li><li>Tile View</li><li><a href="%s">Tile View (images only)</a></li>' % (listview_url, tileview_notext_url);
    else:
        search_result_page_tools = '<li>List View</li><li><a href="%s">Tile View</a></li><li><a href="%s">Tile View (images only)</a></li>' % (tileview_url, tileview_notext_url);

    body += '''
<div id="search_result_count">Search Query: %s</div>
<div id="search_result_panel" class="search_result_panel pagerow">
  <div id="search_result_count">Search Result: %d to %d of %d</div>

  <div id="search_result_page_tools">
    <ul>%s</ul>
  </div>

  <div id="search_result_page_nav">
    <ul><li>Prev</li><li>Next</li></ul>
  </div>
</div>''' % (result_count, result_start,
         result_start + min(result_count, len(results)),
         len(results),
         search_result_page_tools);

    body += '<div class="pageresult">';

    for index, row in match_region_attributes.iterrows():
      filename = row['filename'];
      doc_id = self.file_attributes.filename_to_docid_map[filename];

      html_i = '';
      if tile:
        if noText:
          html_i = '''
    <div class="search_result_i pagecell">
      <div class="img_panel" style="float: none; text-align: center;">
        <a href="file_attributes?docID=%d"><img src="getImage?docID=%s&height=300"></a>
      </div>
    </div>''' % (doc_id, doc_id);

        else:
          html_i = '''
    <div class="search_result_i pagecell_with_margin">
      <div class="header">
        <span class="search_result_filename">Filename: <a href="file_attributes?docID=%d">%s</a></span>
      </div>

      <div class="img_panel" style="float: none; text-align: center;">
        <a href="file_attributes?docID=%d"><img src="getImage?docID=%s&height=300"></a>
      </div>

      <div class="search_result_tools">
        <span><a href="#">See matching attributes</a></span>
      </div>
    </div>''' % (doc_id, filename, doc_id, doc_id);

      body += html_i;
    # end of for istc in ...
    body += '\n</div>';
    return self.pT.get(title= "Search Result", headExtra = "", body = body);
