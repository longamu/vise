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

  @cherrypy.expose
  def index(self, id=None, search_pattern=None, listview=None):
    # groupby = {folio1, folio3}
    body = "";
    if id is None:
      if search_pattern is None:
        ## show an input box where users can type partial istc id
        body += "@todo: show an input box where users can type partial istc id";
      else:
        ## show a list of matched istc entries
        body += "@todo: show a list of istc entries matching the pattern %s" % (search_pattern);
    else:
      if listview is None:
        view_sel_html = '<ul><li><a href="istc?id=%s&listview">List View</a></li><li>Folio View</li></ul>' % (id);
      else:
        view_sel_html = '<ul><li>List View</li><li><a href="istc?id=%s">Folio View</a></li></ul>' % (id);

      # show an index of all images associated with an ISTC
      istc_filelist = self.file_attributes.file_attributes_index[ self.file_attributes.file_attributes_index['istc_id'] == id ];

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
    <span class="search_result_filename">Folio Group: %s</span>
    <span></span>
    <span>Jump to folio group: %s</span>
  </div>
  %s
</div><!-- end of search_result_i -->''' % (folio_group, folio_group, folio_group_nav_html, grouped_html[folio_group]);
        # end of if listview == True
      # end of for index, row ...

      body += '\n</div><!-- end of pageresult -->';
    # end of if id == None
    return self.pT.get(title= "ISTC Images", headExtra = "", body = body);
