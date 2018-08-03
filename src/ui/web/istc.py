##
## VISE Customized for 15th Century Booktrade Project
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 25 Jan. 2018
##


import cherrypy;

import random;
import template;
import copy;
import collections;

class istc:

  def __init__(self, pageTemplate, file_attributes):
    self.pT = pageTemplate;
    self.file_attributes = file_attributes;
    self.istc_index_html = '';

  def get_istc_metadata_html_table(self, istc_id):
      istc_metadata = self.file_attributes.get_istc_metadata(istc_id=istc_id);
      html = 'ISTC metadata not found!';
      if istc_metadata.shape[0] == 1:
          html = '  <span class="title">%s: %s</span><table class="metadata_table"><tbody>' % (istc_metadata.iloc[0]['author'], istc_metadata.iloc[0]['title']);
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

  def get_istc_matches_html(self, istc_search_keyword):
    istc_matches = self.file_attributes.istc_db[ self.file_attributes.istc_db['id'].str.contains(istc_search_keyword, case=False, na=False, regex=True) ];
    if istc_matches.shape[0] == 0:
      return '<p>No match found</p>'
    else:
      istc_matches_html = '<div class="header"><span>Showing %d matches</span><span></span></div><ul>' % (istc_matches.shape[0]);
      for index, row in istc_matches.iterrows():
        istc_id = row['id'];
        img_count = self.file_attributes.file_attributes_index[ self.file_attributes.file_attributes_index['istc_id'] == istc_id ].shape[0];
        istc_matches_html += '<li><a href="./istc?id=%s">%s</a> : (%d images) %s - %s</li>' % ( istc_id, istc_id, img_count, row['author'], row['title'] );
      istc_matches_html += '</ul>';
      return istc_matches_html;

  @cherrypy.expose
  def index(self, id=None, istc_search_keyword=None, listview=None):
    # groupby = {folio1, folio3}
    body = "";
    head = "";
    if id is None:
      if istc_search_keyword is None or istc_search_keyword == '':
        ## show an input box where users can type partial istc id
        body += '''
<div class="istc_search_panel pagerow">
  <form method="GET" action="./istc" id="istc_search">
    <input type="text" id="istc_search_keyword" name="istc_search_keyword" placeholder="Search ISTC id (e.g. ia00154000)" size="25">
    <button type="submit" value="Search">Search</button>
  </form>
</div>''';
        head += ''; # @todo add javascript to show auto-complete
      else:
        ## show a list of matched istc entries
        istc_matches_html = self.get_istc_matches_html(istc_search_keyword);
        body += '''
<div class="istc_search_panel pagerow">
  <form method="GET" action="./istc" id="istc_search">
    <input type="text" id="istc_search_keyword" name="istc_search_keyword" placeholder="Search ISTC id (e.g. ia00154000)" size="25">
    <button type="submit" value="Search">Search</button>
  </form>
</div>
<div class="search_result_i pagerow">
%s
</div>
''' % (istc_matches_html);
    else:
      if listview is None:
        view_sel_html = '<ul><li><a href="istc?id=%s&listview">List View</a></li><li>Folio View</li></ul>' % (id);
      else:
        view_sel_html = '<ul><li>List View</li><li><a href="istc?id=%s">Folio View</a></li></ul>' % (id);

      # show an index of all images associated with an ISTC
      istc_filelist_unsorted = self.file_attributes.file_attributes_index[ self.file_attributes.file_attributes_index['istc_id'] == id ];
      istc_filelist = istc_filelist_unsorted.sort_values('folio', inplace=False);

      # get istc metadata
      istc_metadata_html_table = self.get_istc_metadata_html_table(id);

      grouped_html = {};
      for index, row in istc_filelist.iterrows():
        folio_group = row['folio_group'];
        entry = '''
<div class="padded_img">
  <span class="top_title">
    <ul class="hlist">
      <li>MEI: <a target="_blank" href="http://data.cerl.org/mei/%s">%s</a></li>
      <li>Folio: %s</li>
    </ul>
  </span>
  <a href="search?docID=%s"><img src="getImage?docID=%s&height=300"></a>
        <span class="bottom_title">%s</span>
</div>''' % (row['mei_id'], row['mei_id'], row['folio'], row['doc_id'], row['doc_id'], row['filename']);
        if folio_group in grouped_html:
          grouped_html[folio_group] += entry;
        else:
          grouped_html[folio_group] = entry;

      grouped_html_sorted = collections.OrderedDict( sorted(grouped_html.items()) );
      body += '''
<div id="istc_info_panel" class="istc_info_panel pagerow">
  %s
</div>
<div class="search_result_panel">
  <div>Showing %d images with ISTC id %s</div>
  <div>%s</div>
</div>''' % (istc_metadata_html_table, istc_filelist.shape[0], id, view_sel_html);

      # create html containing all groups
      body += '\n<div class="pageresult">';
      if listview is not None:
        body += '<div class="search_result_i pagerow">'

      for folio_group in grouped_html_sorted:
        if listview is not None:
          body += '%s' % (grouped_html[folio_group]);
        else:
          folio_group_nav_html = '<ul class="hlist">';
          for fg in grouped_html_sorted:
            if folio_group == fg:
              folio_group_nav_html += '<li>%s</li>' % (fg);
            else:
              folio_group_nav_html += '<li><a href="#fg%s">%s</a></li>' % (fg, fg);

          folio_group_nav_html += '</ul>';

          body += '''
<div id="fg%s" class="search_result_i pagerow">
  <div class="header">
    <span class="search_result_filename">Gathering: %s</span>
    <span></span>
    <span>Jump to gathering: %s</span>
  </div>
  %s
</div><!-- end of search_result_i -->''' % (folio_group, folio_group, folio_group_nav_html, grouped_html[folio_group]);
        # end of if listview == True
      # end of for index, row ...

      body += '\n</div><!-- end of pageresult -->';
    # end of if id == None
    return self.pT.get(title= "ISTC Images", headExtra = "", body = body);
