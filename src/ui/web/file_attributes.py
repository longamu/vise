# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# 8 Nov. 2017

import cherrypy;

import random;
import template;
import copy;

class file_attributes:
    
  def __init__(self, pageTemplate, docMap, pathManager_obj, examples= None, externalExamples= None, browse= True, doShowPath= True):
    self.pT= pageTemplate;
    self.docMap= docMap;
    self.examples= examples;
    self.externalExamples= externalExamples;
    self.pathManager_obj= pathManager_obj;
    self.browse= browse;
    self.doShowPath= doShowPath;
    self.def_dsetname= self.docMap.keys()[0];

  @cherrypy.expose
  def index(self, docID= None):
    if docID == None:
      doc_id = 0;
    else:
      doc_id = int(docID);

    filename = self.pathManager_obj[self.def_dsetname].displayPath(doc_id)

    body  = "<h1>File: %s</h1>" % (filename)
    body += '<p><a href="search?docID=%d"><img src="getImage?docID=%d&width=400"></a><br><cite>Click on the image to use it for searching.</cite></p>' %(doc_id, doc_id);
    body += '<p>Metadata</p><ul>';
    body += '<li>key=value</li>';    
    body += '</ul>';

    headExtra = '';

    title = "File: %s" % (filename)
    return self.pT.get( title=title, body=body, headExtra=headExtra );
