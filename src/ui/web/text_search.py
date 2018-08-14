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

  def get_search_input_page(self, search_keyword1=None, search_target1=None, search_operator='AND', search_keyword2=None, search_target2=None, search_year_from=None, search_year_to=None,):
      return '''
<div class="text_search_panel pagerow">
  <span class="title">Search Metadata</span>
  <form action="text_search" method="GET" id="text_search">
  <div class="search_input">
    <div class="pagerow">
      <input type="text" id="search_keyword1" name="search_keyword1" placeholder="search keyword e.g. angel" size="30" value="%s">
      <span>&nbsp;in&nbsp;</span>
      <select id="search_target1" name="search_target1">
        <option value="region_metadata" %s>Region Metadata</option>
        <option value="istc_metadata" %s>ISTC Metadata</option>
      </select>
    </div>

    <div class="pagerow">
      <select id="search_operator" name="search_operator">
        <option value="AND" %s>AND</option>
        <option value="OR" %s>OR</option>
      </select>
    </div>

    <div class="pagerow">
      <input type="text" id="search_keyword2" name="search_keyword2" placeholder="second keyword e.g. Venice" size="30" value="%s">
      <span>&nbsp;in&nbsp;</span>
      <select id="search_target2" name="search_target2">
        <option value="region_metadata" %s>Region Metadata</option>
        <option value="istc_metadata" %s>ISTC Metadata</option>
      </select>
    </div>


    <div class="pagerow">
      <span>between year</span>
      <input type="text" id="search_year_from" name="search_year_from" placeholder="e.g. 1491" size="6" value="%s">
      <span>&nbsp;and&nbsp;</span>
      <input type="text" id="search_year_to" name="search_year_to" placeholder="e.g. 1495" size="6" value="%s">
    </div>
  </div>
  <div class="buttons">
    <button type="submit" value="Search">Search</button>
  </div>
  </form>
</div>''' % (search_keyword1 if search_keyword1 is not None else '', 
            'selected="true"' if search_target1  == 'region_metadata' else '',
            'selected="true"' if search_target1  == 'istc_metadata' else '',
            'selected="true"' if search_operator == 'AND' else '',
            'selected="true"' if search_operator == 'OR' else '',
            search_keyword2 if search_keyword2 is not None else '',
            'selected="true"' if search_target2  == 'region_metadata' else '',
            'selected="true"' if search_target2  == 'istc_metadata' else '',
            search_year_from if search_year_from is not None else '',
            search_year_to if search_year_to is not None else '',  
            );


  # return a list of matching filename
  def search_region_metadata(self, keyword, year_from=None, year_to=None):
    # filename,file_size,file_attributes,region_count,region_id,region_shape_attributes,region_attributes
    match = self.file_attributes.region_attributes['region_attributes'].str.contains(keyword, case=False, na=False, regex=True);
    match_filenames = set(self.file_attributes.region_attributes[match]['filename'].unique())

    # create subset so that further search only happens in entries between year [from, to)
    istc_subset = None;
    if year_from is not None or year_to is not None:
      if year_from is not None:
        istc_subset = self.file_attributes.istc_db[ self.file_attributes.istc_db['imprint_year'] >= int(year_from) ]
        if year_to is not None:
          istc_subset = istc_subset[ istc_subset['imprint_year'] <= int(year_to) ]
        #
        matched_year_filenames = set(self.file_attributes.file_attributes_index.loc[ self.file_attributes.file_attributes_index['istc_id'].isin( istc_subset['id'] )]['filename'].unique());
        return match_filenames.intersection(matched_year_filenames);
    else:
      return match_filenames;

  def search_istc_metadata(self, keyword, year_from=None, year_to=None):
    # id,author,title,imprint,format
    ## iterate through each column
    N = self.file_attributes.istc_db.shape[0];

    # create subset so that further search only happens in entries between year [from, to)
    istc_subset = None;
    if year_from is not None or year_to is not None:
      if year_from is not None:
        istc_subset = self.file_attributes.istc_db[ self.file_attributes.istc_db['imprint_year'] >= int(year_from) ]
        if year_to is not None:
          istc_subset = istc_subset[ istc_subset['imprint_year'] <= int(year_to) ]
    else:
      istc_subset = self.file_attributes.istc_db;

    matched_istc_id = set()
    #for col in self.file_attributes.istc_db.columns:
    for col in {"id","author","title","imprint","format"}:
      istc_matches = istc_subset[ istc_subset[col].str.contains(keyword, case=False, na=False, regex=True) ];
      if istc_matches.shape[0] != 0:
        for index, row in istc_matches.iterrows():
          matched_istc_id.add(row['id']);

    # convert istc-id to unique filenames
    return set(self.file_attributes.file_attributes_index.loc[ self.file_attributes.file_attributes_index['istc_id'].isin( matched_istc_id )]['filename'].unique())


  def get_search_result_navigation_html(self, start_from, count, total_result, text_search_uri):
    nav_buttons = '';
    if start_from != 0:
      nav_buttons += '<li><a href="%s&start_from=%d&count=%d">Prev</a></li>' % (text_search_uri, (start_from - count), count);
    else:
      nav_buttons += '<li>Prev</li>';

    if total_result > (start_from + count):
      nav_buttons += '<li><a href="%s&start_from=%d&count=%d">Next</a></li>' % (text_search_uri, (start_from + count), count);
    else:
      nav_buttons += '<li>Next</li>';

    end_index = start_from + count;
    if end_index > total_result:
      end_index = total_result

    return '''
<div class="text_search_result_nav_panel pagerow">
  <div>Total %d matches, showing from %d to %d</div>
  <div id="search_result_page_nav">
    <ul>%s</ul>
  </div>
</div>''' % ( total_result, start_from, end_index, nav_buttons )

  def get_search_result_html(self, matching_filenames, start_from, count, tile=None, showText=None):
    search_result = self.file_attributes.file_attributes_index.loc[ self.file_attributes.file_attributes_index['filename'].isin( matching_filenames )]

    html = '<div class="text_search_result_panel">';
    
    i = 0;
    for index, row in search_result.iterrows():
      if i >= start_from:
        #print 'adding index %d (start_from=%d, count=%d))' % ( index, start_from, count )
        html += '<a href="file_attributes?docID=%s"><img title="%s" src="getImage?docID=%s&height=300"></a>' % (row['doc_id'], row['filename'], row['doc_id']);

      if i > (start_from + count):
        break;
      i = i + 1;

    html += '</div>'
    return html

  @cherrypy.expose
  def index(self, search_keyword1=None, search_target1='region_metadata', search_operator='AND', search_keyword2=None, search_target2='istc_metadata', search_year_from=None, search_year_to=None, start_from=0, count=25, ):
    # display main search page (without any results) if no search keyword is provided
    text_search_uri = 'text_search?'
    if search_year_to == '':
      search_year_to = None;
    if search_year_from == '':
      search_year_from = None;
    if search_keyword2 == '':
      search_keyword2 = None;
    if search_keyword1 == '':
      search_keyword1 = None;

    body = self.get_search_input_page(search_keyword1, search_target1, search_operator, search_keyword2, search_target2, search_year_from, search_year_to)
    if search_keyword1 is None:
      return self.pT.get(title = "Search Metadata", headExtra = "", body = body);

    search_keyword1 = urllib.unquote(search_keyword1).decode('utf8');
    match = None
    match1 = None;
    if search_target1 == 'region_metadata':
      match1 = self.search_region_metadata(search_keyword1, search_year_from, search_year_to);
    elif search_target1 == 'istc_metadata':
      match1 = self.search_istc_metadata(search_keyword1, search_year_from, search_year_to);
    text_search_uri += 'search_keyword1=' + search_keyword1 + '&search_target1=' + search_target1;

    # we have search_keyword1, check if we have another keyword as well
    if search_keyword2 is not None:
      # search using two keywords: search_keyword1, search_keyword2
      search_keyword2 = urllib.unquote(search_keyword2).decode('utf8');

      match2 = None;
      if search_target2 == 'region_metadata':
        match2 = self.search_region_metadata(search_keyword2, search_year_from, search_year_to);
      elif search_target2 == 'istc_metadata':
        match2 = self.search_istc_metadata(search_keyword2, search_year_from, search_year_to);
      text_search_uri += '&search_keyword2=' + search_keyword2 + '&search_target2=' + search_target2;
    else:
      # search using a single keyword i.e. search_keyword1    
      match2 = None;

    match = None;
    if match2 is not None:
      if search_operator == 'AND':
        match = match1.intersection(match2)
      else:
        match = match1.union(match2)
    else:
      match = match1

    text_search_uri += '&search_operator=' + search_operator;
    if search_year_from is not None:
      text_search_uri += '&search_year_from=' + search_year_from;
    if search_year_to is not None:
      text_search_uri += '&search_year_to=' + search_year_to;

    start_from = int(start_from)
    count = int(count)
    body += self.get_search_result_navigation_html(start_from, count, len(match), text_search_uri)
    body += self.get_search_result_html(match, start_from, count);
    if len(match):
      body += self.get_search_result_navigation_html(start_from, count, len(match), text_search_uri)
    return self.pT.get(title= "Search Result", headExtra = "", body = body);
