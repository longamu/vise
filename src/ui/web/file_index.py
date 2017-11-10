# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# 8 Nov. 2017

import cherrypy;

import random;
import template;
import copy;

class file_index:
    
  def __init__(self, pageTemplate, docMap, pathManager_obj, examples= None, externalExamples= None, browse= True, doShowPath= True):
    # Update a copy of the template so that other pages are not impacted
    pageTemplateCopy = copy.deepcopy(pageTemplate);
    self.title_prefix = pageTemplateCopy.titlePrefix;
    pageTemplateCopy.get = self.get;
    self.pT= pageTemplateCopy;

    self.docMap = docMap;
    self.pathManager_obj= pathManager_obj;
    self.dsetname = self.docMap.keys()[0];
    self.file_index_html = '';

  def get(self, title= "", headExtra= "", body= "", outOfContainer= False ):
    html = """
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>%s : %s</title>
<style>
body { 
  font-size: 1rem;
  margin: 0;
  padding-left: 1rem;
  padding-right: 1rem;
  border: 0;
  background-color: #fff;
  color: #000;
}
ul { list-style-type: none; margin: 0; padding: 0;}
li { background-color: #fff; margin: 0; padding: 0; padding: 0.5rem 0;}
li:nth-child(odd) { background-color: #f2f2f2;}
</style>
</head>
<body>%s</body>
</html>
    """ % (self.title_prefix, title, body)
    return html;

  @cherrypy.expose
  def index(self ):
    body  = "<h1>Index of files in %s</h1>" % (self.dsetname)
    body += '<cite><ul style="list-style-type: disc; padding-left: 2rem;">'
    body += "<li>Use the web browser search functionality to search for keywords (press Ctrl + F)</li>";
    body += '<li>Click on the image-id (for example: [<a target="_blank" href="../search?docID=1">00001</a>]) to perform instance search using this image. </li>';
    body += "</ul></cite>";

    # cache the index for subsequent requests
    if self.file_index_html == '':
      self.file_index_html = '<ul style="margin-top: 2rem;">'
  
      file_count = len(self.docMap[self.dsetname]);
      for doc_id in range(0,file_count):
      #for doc_id in range(0,100):
          self.file_index_html += '<li>[<a target="_blank" href="search?docID=%d">%.5d</a>] <a target="_blank" href="file_attributes?docID=%d">%s</a></li>' % (doc_id, doc_id, doc_id, self.pathManager_obj[self.dsetname].displayPath(doc_id));

      self.file_index_html += '</ul>';

    body += self.file_index_html;
    headExtra = '';


    return self.pT.get( title= "file index", body=body, headExtra=headExtra );
