# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# 8 Nov. 2017

import cherrypy;

import random;
import template;
import copy;

class file_index:
    
  def __init__(self, pageTemplate, docMap, pathManager_obj):
    self.pT= pageTemplate;

    self.docMap = docMap;
    self.pathManager_obj= pathManager_obj;
    self.dsetname = self.docMap.keys()[0];
    self.file_index_html = '';

  @cherrypy.expose
  def index(self):
    body = '<div class="search_result_i pagerow_with_margin" style="padding-bottom: 0 !important;"><ul>'
    body += "<li>Use the web browser search functionality to search for keywords (press Ctrl + F)</li>";
    body += '<li>Click on the image-id (for example: [<a target="_blank" href="../search?docID=1">00001</a>]) to perform instance search using this image.</li>';
    body += "</ul></div>";

    # cache the index for subsequent requests
    if self.file_index_html == '':
      html = list();
      html.append('<div class="pagerow"><ul style="margin-top: 2rem;">');
  
      file_count = len(self.docMap[self.dsetname]);
      print(file_count)
      for doc_id in range(0,file_count):
      #for doc_id in range(0,100):
        #print "Processing doc_id %d" % (doc_id);
        filename = self.pathManager_obj[self.dsetname].displayPath(doc_id);
        html.append('<li>[<a href="search?docID=%d">%.5d</a>] <a href="file_attributes?docID=%d">%s</a></li>' % (doc_id, doc_id, doc_id, filename) );

      html.append('</ul></div>');
      self.file_index_html = ''.join(html);

    body += self.file_index_html;
    headExtra = '';
    return self.pT.get( title= "File Index", headExtra='', body=body );
