# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# 8 Nov. 2017

import cherrypy;

import random;
import template;
import copy;

import pandas as pd;

class file_attributes:
    
  def __init__(self, pageTemplate, docMap, pathManager_obj, examples= None, externalExamples= None, browse= True, doShowPath= True, file_attributes_fn= None, file_attributes_filename_colname= None):
    self.pT= pageTemplate;
    self.docMap= docMap;
    self.examples= examples;
    self.externalExamples= externalExamples;
    self.pathManager_obj= pathManager_obj;
    self.browse= browse;
    self.doShowPath= doShowPath;
    self.dsetname= self.docMap.keys()[0];

    self.attributes_available = False;
    self.file_attributes_index = None;
    if file_attributes_fn != None:
      self.attributes_available = True;
      self.load_file_attributes(file_attributes_fn, file_attributes_filename_colname);

  def load_file_attributes(self, file_attributes_fn, file_attributes_filename_colname):
    csv_metadata = pd.read_csv(file_attributes_fn);
    csv_metadata.rename( columns={file_attributes_filename_colname: 'filename'}, inplace=True );
    csv_metadata.drop([col for col in csv_metadata.columns if "Unnamed" in col], axis=1, inplace=True) # remove unnamed columns

    file_count = len(self.docMap[self.dsetname]);
    dataset_index = {};
    dataset_index['doc_id'] = range(0, file_count);
    dataset_index['filename'] = list();
    for doc_id in range(0,file_count):
      dataset_index['filename'].append(self.pathManager_obj[self.dsetname].displayPath(doc_id));

    dataset_index_df = pd.DataFrame(dataset_index);

    self.file_attributes_index = pd.merge(dataset_index_df, csv_metadata, on='filename')
    print 'Finished loading attributes for %d files' % (len(self.file_attributes_index.index))

  def filename_to_docid(self, filename_pattern):
    match = self.file_attributes_index[ self.file_attributes_index['filename'].str.contains(filename_pattern) ]
    return match.iloc[:]['doc_id']

  @cherrypy.expose
  def index(self, docID= None, filename=None):
    doc_id_list = pd.Series( data=[], dtype=int );

    if docID == None:
      if filename == None:
        doc_id_list = pd.Series( data=[0], dtype=int ); # show first image if no argument is provided
      else:
        doc_id_list = self.filename_to_docid(filename);
    else:
      doc_id_list = pd.Series( data=[docID], dtype=int );

    file_count = len(self.docMap[self.dsetname]);
    navigation  = '<div id="navbar" style="display: block;background-color:#d7f4f6;border: 1px solid #ccc;padding: 1rem;line-height: 2rem">'

    if doc_id_list.size == 1:
      doc_id = doc_id_list.iloc[0]
      navigation += 'Showing file %d of total %d files' % (doc_id, file_count);
    else:
      if filename != None:
        navigation += 'Search keyword: %s' % (filename)
      else:
        navigation += '&nbsp;'

    navigation += '<span style="line-height: 2rem; float:right;">'
    navigation += '<form action="file_attributes" method="POST" id="filename_search">'
    navigation += '<input type="text" name="filename" value="enter partial filename" title="search filenames using keyword or regular expression" size="12" onclick="this.value=\'\';">'
    navigation += '&nbsp;&nbsp;<button type="submit" form="filename_search" value="Submit">Search</button>'
    navigation += '</form>&nbsp;&nbsp;|&nbsp;&nbsp;'

    if doc_id_list.size == 1:
      doc_id = doc_id_list.iloc[0]
      if doc_id > 0:
          navigation+= '<a href="./file_attributes?docID=%d">Prev</a>&nbsp;&nbsp;|&nbsp;&nbsp;' % ( doc_id - 1 );
      if doc_id < file_count:
          navigation+= '<a href="./file_attributes?docID=%d">Next</a>&nbsp;&nbsp;|&nbsp;&nbsp;' % ( doc_id + 1 );

    navigation += '<a target="_blank" href="file_index" title="Browse index of files in this dataset">Index</a></span>';
    navigation += '</div>';  

    body  = navigation

    if doc_id_list.size == 0:
      body += "<h1>File not found</h1>"
      title = ""
    elif doc_id_list.size == 1:
      doc_id = doc_id_list.iloc[0]
      filename = self.pathManager_obj[self.dsetname].displayPath(doc_id)
      body += "<h1>File: %s</h1>" % (filename)
      body += '<table style="width:100%;">'
      body += '<tr><td valign="top">'
      body += '<p><a title="Search using this image" href="search?docID=%d"><img src="getImage?docID=%d&width=400"></a></p><p><i>Click on the image to use it for searching.</i></p></td>' %(doc_id, doc_id);

      metadata = self.file_attributes_index[ self.file_attributes_index['doc_id'] == doc_id ];
      if metadata.shape[0] == 1:
        body += '<td valign="top"><ul>';
        for key in metadata:
          value = metadata.iloc[0][key]
          if type(value) == str and value.startswith('http://'):
            value = '<a target="_blank" href="%s">%s</a>' % (value, value)

          body += '<li>%s: %s</li>' %(key, value);
        body += '</ul></td></tr></table>';
      else:
        body += '<td><p>File attributes not found</p></td>';
      title = "File: %s" % (filename)
    else:
      # show a list of files
      body += "<h1>Search result</h1>"
      body += "<ul>"
      for doc_id in doc_id_list:
        match = self.file_attributes_index[ self.file_attributes_index['doc_id'] == doc_id ]
        body += '<li>[<a title="Search using this image" href="../search?docID=%d">%.5d</a>] <a title="View image attributes" href="file_attributes?docID=%d">%s</a></li>' % (doc_id, doc_id, doc_id, match.iloc[0]['filename']);
      body += "</ul>"

      title = "Search result"

    headExtra = '';
    return self.pT.get( title=title, body=body, headExtra=headExtra );
