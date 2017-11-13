#
# ==== Author:
# 
# Relja Arandjelovic (relja@robots.ox.ac.uk)
# Visual Geometry Group,
# Department of Engineering Science
# University of Oxford
#
# ==== Copyright:
#
# The library belongs to Relja Arandjelovic and the University of Oxford.
# No usage or redistribution is allowed without explicit permission.
#

import os;
import cherrypy;
import urllib2;
import uuid;
import cPickle;
from PIL import Image;
try:
    import PngImagePlugin, JpegImagePlugin, TiffImagePlugin, GifImagePlugin, BmpImagePlugin, PpmImagePlugin; # all this stuff for cx_freeze
except:
    pass;
import time;

import json;

import sys, os;

import multiprocessing;
from multiprocessing import Pool, Lock;
manager= multiprocessing.Manager();
import threading;


import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();
tmpDir=    os.path.join( scriptroot, 'tmp/' );
uploadDir= os.path.join( scriptroot, 'tmp/uploaded/' );

print 'scriptroot = %s' %(scriptroot);
print 'uploadDir = %s' %(uploadDir);
print 'tmpDir = %s' %(tmpDir);

maxSizeDefault= 600;



class savedTemp:
    
    def __init__(self):
        self.allvars={};
    
    def read(self, filename):
        self.allvars = cPickle.load(open(filename, 'rb'));
    
    def write(self, filename):
        cPickle.dump(self.allvars, open(filename,'wb'), -1);
    
    @staticmethod
    def load(filename):
        if isinstance(filename,savedTemp):
            sr= filename;
        else:
            sr= savedTemp(); sr.read( os.path.join(tmpDir, filename) );
        return sr;
  
    def store(self, filename):
        if not( isinstance(filename,savedTemp) ):
            self.write( os.path.join(tmpDir, filename) );
    
    def __getitem__(self, key):
        return self.allvars[key];
    
    def __setitem__(self, key, val):
        self.allvars[key]= val;
    
    def __delitem__(self, key):
        del self.allvars[key];




class upload:
    
    
    
    class states:
        uploading=      0
        uploadFinished= 1
        processing=     2
        querying=       3
    
    
    
    def __init__(self, pageTemplate, API_obj):
        
        self.pT= pageTemplate;
        self.API_obj= API_obj;
        self.datasets= self.API_obj.keys();
        
        self.def_dsetname= self.API_obj.keys()[0];
        
        self.uploadFile= {};
        self.uploadFileLock= threading.Lock();
        
        # ensure uploadDir exists as it might have been deleted through cleanup..
        os.popen('mkdir -p "%s"' % uploadDir);
    
    
    
    def initUpload(self, uploadFile, uploadURL, **kwargs):
        
        st= savedTemp();
        
        upID= str(uuid.uuid4());
        st['upID']= upID;
        st['kwargs']= kwargs;
        
        st['uploadWhat']= -1;
        if uploadFile!=None and uploadFile.filename != '':
            st['uploadWhat']= 0;
            st['originalFullFilename']= uploadFile.filename;
            self.uploadFileLock.acquire();
            self.uploadFile[upID]= uploadFile;
            self.uploadFileLock.release();
        elif uploadURL!=None and uploadURL!='':
            st['uploadWhat']= 1;
            st['originalFullFilename']= uploadURL;
        newName= os.path.basename(st['originalFullFilename'])+'.jpg';
        if len(newName)>24:
            newName=newName[0:11]+'..'+newName[-11:]
        st['localFilename']= os.path.join(uploadDir, '%s/orig' % upID);
        st['localFilename_full_jpg']= os.path.join(uploadDir, '%s/orig_full.jpg' % upID);
        st['localFilename_jpg']= os.path.join(uploadDir, upID, newName );
        st['compDataFilename']= os.path.join(tmpDir, '%s.compdata' % upID);
        
        st['size']= 0;
        st['allData']= '';

        os.popen('mkdir -p "%s%s"' % (uploadDir,upID) );
        st.store( upID );
        
        return st, upID;
    
    
    
    @cherrypy.expose
    def upload(self, uploadFile= None, uploadURL= None, **kwargs):
        print 'cherrypy.request.headers.get("Content-Length")=%s' %(cherrypy.request.headers.get("Content-Length"));
        print 'uploadFile = %s' %(uploadFile.filename);
        print 'uploadFile type = %s' %( type(uploadFile.filename) );
        print 'uploadURL = %s' %(uploadURL);
        if ( not uploadFile.filename and not uploadURL ):
          body = "<h2>Error: image not selected</h2><p>Select a local image file or a URL to an image and press <i>Upload and Search</i> button.</p>"
          return self.pT.get( title= "File upload error", headExtra= '', body= body );

        st, upID= self.initUpload( uploadFile, uploadURL, **kwargs );
        returnURL= "dosearch?uploadID=%s" % upID;
        
        headExtra= """
<script language="javascript">

  var http = new XMLHttpRequest();
  var states = {
    uploading:          %d,
    uploadFinished:     %d,
    processing:         %d,
    querying:           %d,
  }
  var state = states.uploading;

/*
  function pausecomp(millis) {
    var date = new Date();
    var curDate = null;
    do { curDate = new Date(); } while(curDate-date < millis);
  }
*/

  function getResults(){
  
    if(http.readyState==4){
          
      response=http.responseText;
      error = false;
      
      if (state == states.uploading){
        if (response.substring(0,9) == 'Uploading'){
          // still need to upload more
          document.getElementById('uploadProgress').innerHTML = response;
          //pausecomp(1000)
          sendRequest(states.uploading, 'uploadNext?')
        }
        else if (response.substring(0,8) == 'Finished'){
          // finished uploading, save the file
          document.getElementById('uploadProgress').innerHTML = response;
          sendRequest(states.uploadFinished, 'uploadSave?')
        } else { error = true; }
      }
      else if (state == states.uploadFinished){
        // Saved the file to disk
        document.getElementById('uploadProgress').innerHTML += '<div align="center"><img width="200" src="getImage?uploadID=%s"></br>';
        document.getElementById('procProgress').innerHTML = 'Processing image'
        sendRequest(states.processing, 'uploadProcess?state='+states.processing+'&')
      }
      else if (state == states.processing){
        document.getElementById('procProgress').innerHTML += ': DONE'
        document.getElementById('queryProgress').innerHTML = 'Querying'
        document.location.href='%s'
      }
      else { error = true; }

      if (error) { alert(response); } // shoudln't happen
      
    }
    
  }
  
  function sendRequest(aState, command){
    state = aState
    http.open('get', command+'upID=%s');
    http.onreadystatechange=getResults
    http.send(null);
  }
  
</script>
        """ % (self.states.uploading, self.states.uploadFinished, self.states.processing, self.states.querying, \
               upID, \
               returnURL, \
               upID );
        
        body= """
<div id="uploadFilename">Uploading file: %s</div>
<div id="uploadProgress"></div>
<div id="procProgress"></div>
<div id="queryProgress"></div>

<script language="javascript">
sendRequest(states.uploading, 'uploadNext?');
</script>
        """ % st['originalFullFilename'];
        
        return self.pT.get( title= "File Upload", headExtra= headExtra, body= body );
    
    
    
    @cherrypy.expose
    def uploadNext(self, upID):
        
        st= savedTemp.load( upID );
        
        if st['uploadWhat']==0: # from user's local file
            
            self.uploadFileLock.acquire();
            try:
                data= self.uploadFile[upID].file.read(8192);
                if not(data):
                    del self.uploadFile[upID];
                    # finally apparently releases the lock (otherwise exception - releasing already released lock)
                    return 'Finished uploading: %.10d bytes' % st['size'];
            finally:
                self.uploadFileLock.release();
            st['allData']+= data;
            st['size']+= len(data);
            st.store( upID );
            
            return 'Uploading file: %.10d bytes' % st['size'];
            
        elif st['uploadWhat']==1: # from url
            
            opener= urllib2.build_opener();
            opener.addheaders= [('User-agent', 'Mozilla/5.0')]; # pretend to be Firefox (for Wiki etc)
            print "\n\n-%s-\n\n" % st['originalFullFilename'];
            # timeout= X doesn't work for some reason and I get timeouts..
            for itime in range(0,5*5):
                try:
                    infile= opener.open(st['originalFullFilename']);
                    break;
                except Exception:
                    time.sleep(0.2);
                    infile= None;
                    pass;
            
            if infile==None:
                raise "Timed out?";
            page= infile.read();
            
            f= open(st['localFilename'],'wb');
            f.write(page);
            f.close();
            
            return 'Finished uploading';
            
        else:
            
            return 'Error: No file specified';
    
    
    
    @cherrypy.expose
    def uploadSave(self, upID):
        
        st= savedTemp.load( upID );
        
        if st['uploadWhat']==0: # from user's local file
            savedFile= open(st['localFilename'], 'wb');
            savedFile.write(st['allData']);
            del st['allData'];
            savedFile.close();
        
        # get resolution
        im= Image.open(st['localFilename']);
        
        # convert to RGB (e.g. for saving .gif files as .jpg)
        if im.mode != "RGB":
            im = im.convert("RGB");
        
        im.save( st['localFilename_full_jpg'], "JPEG" );
        
        if 'maxSize' in st['kwargs']:
            maxSize= int(st['kwargs']['maxSize']);
        else:
            maxSize= maxSizeDefault;
        
        if im.size[0] > maxSize or im.size[1] > maxSize:
            # downsize
            print 'Resizing image';
            im.thumbnail((maxSize,maxSize), Image.ANTIALIAS );
        
        st['w'], st['h']= im.size;
        
        # save the image anyway in order to ensure it is jpeg (otherwise feature extraction might not work since it only supports pgm, ppm, png or jpg)
        st['localFilename']= st['localFilename_jpg'];
        im.save( st['localFilename'], "JPEG");
        
        st.store( upID );
        
        return 'Saved file';
    
    
    
    @cherrypy.expose
    def uploadProcess(self, state, upID, dsetname= None):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        state= int(state);
        
        if (state == self.states.querying):
            return;
        
        st= savedTemp.load( upID );
        
        if (state == self.states.processing):
            
            self.API_obj[dsetname].processImage( st['localFilename_jpg'], st['compDataFilename'] );
            
        else: # this shouldn't happen!
            print 'Error, unknown state';
        
        st.store( upID );
        
        return;
    
    
    
    def uploadAPI( self, uploadURL, dsetname= None ):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        st, upID= self.initUpload( None, uploadURL );
        
        resp= "";
        while not( resp.startswith("Finished") ):
            resp= self.uploadNext(st);
        self.uploadSave(st);
        self.uploadProcess(self.states.processing, st, dsetname= dsetname);
        st['upID']= upID;
        st.store( upID );
        
        return upID;
