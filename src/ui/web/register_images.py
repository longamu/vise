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

import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();
tmpDir=    os.path.join( scriptroot, 'tmp/' );

import cherrypy;

from PIL import Image;
try:
    import PngImagePlugin, JpegImagePlugin, TiffImagePlugin, GifImagePlugin, BmpImagePlugin, PpmImagePlugin; # all this stuff for cx_freeze
except:
    pass;


import StringIO;

from dynamic_image import dynamicImage;

from upload import savedTemp;



class registerImages:
    
    def __init__(self, pageTemplate, API_obj):
        self.pT= pageTemplate;
        self.API_obj= API_obj;
        self.def_dsetname= self.API_obj.keys()[0];
    
    
    @cherrypy.expose
    def index(self, docID1= None, uploadID1= None, docID2= None, xl= None, xu= None, yl= None, yu= None, dsetname= None):
        
        if dsetname==None: dsetname= self.def_dsetname;
        if docID1!=None:
            docID1= int(docID1);
        if docID2!=None:
            docID2= int(docID2);
        if xl!=None: xl= float(xl);
        if xu!=None: xu= float(xu);
        if yl!=None: yl= float(yl);
        if yu!=None: yu= float(yu);
        
        if uploadID1==None:
            registerID= self.API_obj[dsetname].register( docID1= docID1, docID2= docID2, xl= xl, xu= xu, yl= yl, yu= yu );
        else:
            st= savedTemp.load(uploadID1);
            registerID= self.API_obj[dsetname].registerExternal( st['compDataFilename'], uploadID1, docID2= docID2, xl= xl, xu= xu, yl= yl, yu= yu );
            del st;
        
        outFnPrefix= os.path.join( scriptroot, 'tmp' );
        title= "Compare Match";

        im1_id = ("docID=%s" % docID1) if uploadID1==None else ("uploadID=%s" % uploadID1);
        im2_id = ("docID=%s" % docID2);
        body = """
<div class="compare_panel">
  <div class="row">
    <div class="col">Query</div>
    <div class="col">Comparison</div>
    <div class="col">Match</div>
  </div>
  <div class="row">
    <div class="query"><img name="im1" src="tmpImage?registerID=%s&imName=im1"></div>
    <div class="compare"><img name="compare_img" onmousedown="change_to_im2t();" onmouseup="change_to_im1();"></div>
    <div class="match"><img name="im2t" src="tmpImage?registerID=%s&imName=im2t"></div>
  </div>
  <div  class="row">
    <div class="col"><a href="search?%s">Search on full image</a> (or, view <a href="getImageFull?%s">full image</a>)</div>
    <div class="col">Click on the image to <a onclick="change_compare_img();">switch</a> between regions being compared.</div>
    <div class="col"><a href="search?docID=%s">Search on full image</a> (or, view <a href="getImageFull?docID=%s">full image</a>)</div>
  </div>
</div>

<script language="javascript">
    jsIm1 = new Image();
    jsIm2t= new Image();
    jsIm1.src ="tmpImage?registerID=%s&imName=im1";
    jsIm2t.src="tmpImage?registerID=%s&imName=im2t";
    
    var currentImage= 1;

    function change_to_im1(){
        document['compare_img'].src= jsIm1.src;
        currentImage= 1;
    }

    function change_to_im2t(){
        document['compare_img'].src= jsIm2t.src;
        currentImage= 2;
    }
    function change_compare_img() {
      switch(currentImage) {
      case 1:
        change_to_im2t();
        break;
      case 2:
        change_to_im1();
        break;
      }
    }

    change_to_im1();
</script>
""" % (registerID, registerID, \
      im1_id, im1_id, docID2, docID2, registerID, registerID);
        return self.pT.get(title= title, headExtra= "", body= body, outOfContainer= True);
        
    
    
    @cherrypy.expose
    def tmpImage(self, registerID, imName, width= None):
        
        outFnPrefix= os.path.join( scriptroot, 'tmp' );
        fn= os.path.join( outFnPrefix, '%s_%s.jpg' % (registerID,imName) );
        
        # for security check filename - !!TODO
        
        cherrypy.response.headers['Content-Type'] = 'image/jpeg';
        return dynamicImage.getImageFromFile( fn, width= width );
        
