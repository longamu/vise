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
    csv_metadata.loc[:, ~csv_metadata.columns.str.contains('^Unnamed')]; # remove unnamed columns

    file_count = len(self.docMap[self.dsetname]);
    dataset_index = {};
    dataset_index['doc_id'] = range(0, file_count);
    dataset_index['filename'] = list();
    for doc_id in range(0,file_count):
      dataset_index['filename'].append(self.pathManager_obj[self.dsetname].displayPath(doc_id));

    dataset_index_df = pd.DataFrame(dataset_index);

    self.file_attributes_index = pd.merge(dataset_index_df, csv_metadata, on='filename')
    print 'Finished loading attributes for %d files' % (len(self.file_attributes_index.index))

  @cherrypy.expose
  def index(self, docID= None):
    if docID == None:
      doc_id = 0;
    else:
      doc_id = int(docID);

    filename = self.pathManager_obj[self.dsetname].displayPath(doc_id)

    body  = "<h1>File: %s</h1>" % (filename)
    body += '<p><a href="search?docID=%d"><img src="getImage?docID=%d&width=400"></a><br><cite>Click on the image to use it for searching.</cite></p>' %(doc_id, doc_id);
    body += '<p>Metadata</p><ul>';

    metadata = self.file_attributes_index[ self.file_attributes_index['doc_id'] == doc_id ];
    for column in metadata:
      body += '<li>%s: %s</li>' %(column, metadata[column]);
    body += '</ul>';

    headExtra = '';

    title = "File: %s" % (filename)
    return self.pT.get( title=title, body=body, headExtra=headExtra );
