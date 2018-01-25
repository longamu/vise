# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# 8 Nov. 2017

import cherrypy;

import random;
import template;
import copy;

import pandas as pd;

class file_attributes_15cbt:
    
  def __init__(self, pageTemplate, docMap, pathManager_obj, file_attributes_fn= None, file_attributes_filename_colname= "filename", istc_db_fn=None, istc_id_colname="id"):
    self.pT= pageTemplate;
    self.docMap= docMap;
    self.pathManager_obj= pathManager_obj;
    self.dsetname= self.docMap.keys()[0];

    self.attributes_available = False;
    self.file_attributes_index = None;

    self.load_istc_db(istc_db_fn, istc_id_colname);
    self.load_file_attributes(file_attributes_fn, file_attributes_filename_colname);

  def load_istc_db(self, istc_db_fn, istc_id_colname):
    if istc_db_fn != None:
      self.istc_db = pd.read_csv(istc_db_fn, encoding='utf-8');
      self.istc_db.drop([col for col in self.istc_db.columns if "Unnamed" in col], axis=1, inplace=True) # remove unnamed columns
      if istc_id_colname != 'id':
        self.istc_db.rename( columns={istc_id_colname: 'id'}, inplace=True );
      print "Loaded %d entries in istc database " % (self.istc_db.shape[0]);

  def load_file_attributes(self, file_attributes_fn=None, file_attributes_filename_colname=None):
    file_count = len(self.docMap[self.dsetname]);
    dataset_index = {};
    dataset_index['doc_id'] = range(0, file_count);
    dataset_index['filename'] = list();
    for doc_id in range(0,file_count):
      dataset_index['filename'].append(self.pathManager_obj[self.dsetname].displayPath(doc_id));

    if file_attributes_fn != None:
      csv_metadata = pd.read_csv(file_attributes_fn, encoding='utf-8');
      if file_attributes_filename_colname != 'filename':
        csv_metadata.rename( columns={file_attributes_filename_colname: 'filename'}, inplace=True );
      csv_metadata.drop([col for col in csv_metadata.columns if "Unnamed" in col], axis=1, inplace=True) # remove unnamed columns
      dataset_index_df = pd.DataFrame(dataset_index);

      self.file_attributes_index = pd.merge(dataset_index_df, csv_metadata, on='filename')
      self.attributes_available = True;
    else:
      self.file_attributes_index = pd.DataFrame(dataset_index);

    print 'Finished loading attributes for %d files' % (self.file_attributes_index.shape[0])

  def filename_to_docid(self, filename_pattern):
    match = self.file_attributes_index[ self.file_attributes_index['filename'].str.contains(filename_pattern) ]
    return match.iloc[:]['doc_id']


  def extract_istc_id_from_filename(self, filename):
    # filename: ia00152000_00202205_i1v.jpg
    first_underscore = filename.find('_', 0);
    if first_underscore == -1:
      print 'extract_istc_id_from_filename() : invalid filename format %s' % (filename);
    return filename[0:first_underscore];

  def get_file_metadata(self, docID=None, filename=None):
    if docID == None and filename == None:
      print('Error: get_file_metadata() must be provided with either docID or filename')
      return;

    if filename == None:
      filename = self.pathManager_obj[self.dsetname].displayPath(docID)

    istc_id = self.extract_istc_id_from_filename(filename);
    istc_metadata = self.istc_db[ self.istc_db['id'] == istc_id ]
    return istc_metadata;
    

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
    navigation  = '<div id="navbar" style="display: table; width:96%;background-color:#d7f4f6;border: 1px solid #ccc;padding: 1rem;line-height: 2rem">'

    if doc_id_list.size == 1:
      doc_id = doc_id_list.iloc[0]
      navigation += '<div style="display: table-cell;text-align: left;">Showing file %d of total %d files</div>' % (doc_id, file_count);
    else:
      if filename != None:
        navigation += '<div style="display: table-cell;text-align: left;">Search keyword: %s</div>' % (filename)
      else:
        navigation += '<div style="display: table-cell;text-align: left;">&nbsp;</div>'

    navigation += '<div style="display: table-cell;">'
    navigation += '<form action="file_attributes" method="POST" id="filename_search">'
    navigation += '<input type="text" name="filename" value="enter partial filename" title="search filenames using keyword or regular expression" size="12" onclick="this.value=\'\';">'
    navigation += '&nbsp;<button type="submit" form="filename_search" value="Submit">Search</button>'
    navigation += '</form></div>'
    navigation += '<div style="display: table-cell; text-align: right;">'

    if doc_id_list.size == 1:
      doc_id = doc_id_list.iloc[0]
      if doc_id > 0:
          navigation+= '<a href="./file_attributes?docID=%d">Prev</a>&nbsp;&nbsp;|&nbsp;&nbsp;' % ( doc_id - 1 );
      if doc_id < file_count:
          navigation+= '<a href="./file_attributes?docID=%d">Next</a>&nbsp;&nbsp;|&nbsp;&nbsp;' % ( doc_id + 1 );

    navigation += '<a target="_blank" href="file_index" title="Browse index of files in this dataset">Index</a></div>';
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
      body += "<h1>Search result : found %d files</h1>" %(doc_id_list.size)
      body += "<ul>"
      for doc_id in doc_id_list:
        match = self.file_attributes_index[ self.file_attributes_index['doc_id'] == doc_id ]
        body += '<li>[<a title="Search using this image" href="../search?docID=%d">%.5d</a>] <a title="View image attributes" href="file_attributes?docID=%d">%s</a></li>' % (doc_id, doc_id, doc_id, match.iloc[0]['filename']);
      body += "</ul>"

      title = "Search result"

    headExtra = '';
    return self.pT.get( title=title, body=body, headExtra=headExtra );
