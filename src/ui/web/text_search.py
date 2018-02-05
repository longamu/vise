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
import urllib;
import json;

class text_search:

  def __init__(self, pageTemplate, file_attributes):
    self.pT = pageTemplate;
    self.file_attributes = file_attributes;


  def get_istc_metadata_html_table(self, filename):
    istc_metadata = self.file_attributes.get_file_metadata(filename=filename);
    html = 'ISTC metadata for ' + filename + ' not found!';
    if istc_metadata.shape[0] == 1:
      html = '<table class="metadata_table"><tbody>'
      for key in istc_metadata:
        value = istc_metadata.iloc[0][key]
        if key == 'id':
          istc_url = '<a target="_blank" href="http://data.cerl.org/istc/%s">%s</a>' % (value, value);
          html += '<tr><td>%s</td><td>%s</td></tr>' % (key, istc_url);
          continue;
        if value != '':
          html += '<tr><td>%s</td><td>%s</td></tr>' % (key, value);
        else:
          html += '<tr><td>%s</td><td></td></tr>' % (key);
      html += '</tbody></table>';
    return html;

  def get_region_attributes_html_table(self, region_json_str, rank):
    region_metadata_html = '''
    <input type="checkbox" class="show_more_state" id="region_%d_metadata" />
    <table class="metadata_table show_more_wrap"><tbody>
''' % (rank);

    region_json = json.loads(region_json_str);
    metadata_index = 0;
    for key in region_json :
      value = region_json[key];
      if value != '':
        if metadata_index < 4:
          region_metadata_html += '<tr><td>%s</td><td>%s</td></tr>' % (key, value);
        else:
          region_metadata_html += '<tr class="show_more_target"><td>%s</td><td>%s</td></tr>' % (key, value);
      else:
          region_metadata_html += "<tr><td>%s</td><td></td></tr>" % (key);

      metadata_index = metadata_index + 1;

    # end of for metadata_i
    region_metadata_html += '</tbody></table><label for="region_%d_metadata" class="show_more_trigger"></label>' % (rank);
    return region_metadata_html;

  def get_istc_metadata_matches(self, keyword, numberToReturn, startFrom, tile, noText):
    # id,author,title,imprint,format
    ## iterate through each column
    N = self.file_attributes.istc_db.shape[0];

    matched_istc_html = '';
    for col in self.file_attributes.istc_db.columns:
      istc_matches = self.file_attributes.istc_db[ self.file_attributes.istc_db[col].str.contains(keyword, case=False, na=False, regex=True) ];
      if istc_matches.shape[0] != 0:
        matched_istc_html += '''
<div class="search_result_i pagerow">
  <div class="header">
    <span class="search_result_filename">Search results matching ISTC field "%s"</span>
    <span></span>
  </div>
  <p style="word-wrap:break-word; padding: 2rem;">''' % (col);
        for index, row in istc_matches.iterrows():
          matched_istc_html += '<a href="./istc?id=%s">%s</a>, ' % (row['id'], row['id'],);
        matched_istc_html += '</p></div>';
    return matched_istc_html;

  def get_region_attributes_matches(self, keyword, numberToReturn, startFrom, tile, noText):
    # filename,file_size,file_attributes,region_count,region_id,region_shape_attributes,region_attributes
    results = self.file_attributes.region_attributes['region_attributes'].str.contains(keyword, case=False, na=False, regex=True);
    match_region_attributes = self.file_attributes.region_attributes[results];

    region_attributes_matches_html = '';
    region_count = 0;
    for index, row in match_region_attributes.iterrows():
      filename = row['filename'];
      region_json_str = row['region_attributes'];
      doc_id = self.file_attributes.filename_to_docid_map[filename];

      if tile:
        if noText:
          region_attributes_matches_html += '''
    <div class="search_result_i pagecell">
      <div class="img_panel" style="float: none; text-align: center;">
        <a href="./search?docID=%d" title="Search using this image"><img src="getImage?docID=%d&height=300"></a>
      </div>
    </div>''' % (doc_id, doc_id);

        else:
          region_attributes_matches_html += '''
    <div class="search_result_i pagecell_with_margin">
      <div class="header">
        <span class="search_result_filename">Filename: <a href="file_attributes?docID=%d">%s</a></span>
      </div>

      <div class="img_panel" style="float: none; text-align: center;">
        <a href="file_attributes?docID=%d"><img src="getImage?docID=%d&height=300"></a>
      </div>

      <div class="search_result_tools">
        <span><a href="./search?docID=%d" title="Search using this image">Image Search</a></span>
        <span><a href="./file_attributes?docID=%d" title="Show more details about this file">More Details</a></span>
      </div>
    </div>''' % (doc_id, filename, doc_id, doc_id, doc_id, doc_id);
      else:
        istc_metadata_html = self.get_istc_metadata_html_table(filename);
        region_attributes_html = self.get_region_attributes_html_table(region_json_str, region_count);
        region_count = region_count + 1;

        region_attributes_matches_html += '''
    <div class="search_result_i pagerow">
      <div class="header">
        <span class="search_result_filename">Filename: <a href="file_attributes?docID=%d">%s</a></span>
        <span></span>
      </div>

      <div class="img_panel">
        <a href="file_attributes?docID=%d"><img src="getImage?docID=%s&height=300"></a>
      </div>

      <div class="istc_metadata"><strong>ISTC Metadata</strong>%s</div>
      <div class="region_metadata"><strong>Region Metadata</strong>%s</div>

      <div class="search_result_tools">
        <span><a href="./search?docID=%d" title="Search using this image">Image Search</a></span>
      </div>
    </div>''' % (doc_id, filename, doc_id, doc_id, istc_metadata_html, region_attributes_html, doc_id);
    # end of for index, row in match_region_attributes.iterrows():
    return region_attributes_matches_html;

  @cherrypy.expose
  def index(self, keyword=None, target="all", numberToReturn=20, startFrom=0, tile=None, noText=None):
    if keyword is None:
      body = '''
<div class="text_search_panel pagerow">
  <span class="title">Enter search keyword to perform text search.</span>
  <ul>
    <li>You can also enter <a href="https://developers.google.com/edu/python/regular-expressions">regular expression</a> for more complex search queries.</li>
    <li>For example: @todo</li>
  </ul>
</div>''';
      return self.pT.get(title = "Search", headExtra = "", body = body);

    keyword = urllib.unquote(keyword).decode('utf8');
    target = urllib.unquote(target).decode('utf8');

    print 'search keyword = %s, target = %s' %(keyword, target);

    if target == 'Image Filename':
      return self.file_attributes.index( filename=keyword );

    body = '';
    result_count = int(numberToReturn);
    result_start = int(startFrom);
    result_end = result_start + result_count;

    if tile is None:
      tile = False;
    else:
      tile = True;
    if noText is None:
      noText = False;
    else:
      noText = True;

    # switch between tiled and list views
    search_result_page_tools = '';
    listview_url = 'text_search?keyword=%s&target=%s' % (keyword, target);
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
<div class="text_search_panel pagerow">
  <span class="title">Search keyword: "%s" in %s</span>
</div>

<div id="search_result_panel" class="search_result_panel pagerow">
  <div id="search_result_count">Showing all search results</div>

  <div id="search_result_page_tools">
    <ul>%s</ul>
  </div>
<!--
  <div id="search_result_page_nav">
    <ul><li>Prev</li><li>Next</li></ul>
  </div>
-->
</div>''' % (keyword, target, search_result_page_tools);

    body += '<div class="pageresult">';

    if target == 'Region Attributes':
      body += self.get_region_attributes_matches(keyword, numberToReturn, startFrom, tile, noText);
    if target == 'ISTC Metadata':
      body += self.get_istc_metadata_matches(keyword, numberToReturn, startFrom, tile, noText);
      #body += '<p color="red">Not implemented yet!</p>';

    body += '\n</div>';
    return self.pT.get(title= "Search Result", headExtra = "", body = body);
