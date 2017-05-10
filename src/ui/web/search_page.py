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

import os.path;
import cherrypy;
from cherrypy.lib.static import serve_file;
from PIL import Image;
try:
    import PngImagePlugin, JpegImagePlugin, TiffImagePlugin, GifImagePlugin, BmpImagePlugin, PpmImagePlugin; # all this stuff for cx_freeze
except:
    pass;
import StringIO;

# see the comment for IBBA
# TODO make pytiff optional (only needed for IBBA ballads)
try:
    import pytiff;
except:
    pass;
import uuid;

from upload import savedTemp;


import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();


class searchPage:
    
    def __init__(self, pageTemplate, docMap, pathManager_obj):
        self.pT= pageTemplate;
        self.docMap= docMap;
        self.pathManager_obj= pathManager_obj;
        self.def_dsetname= self.docMap.keys()[0];
        
        
    @cherrypy.expose
    def index(self, docID= None, fn= None, uploadID= None, startFrom= "0", numberToReturn= "20", dsetname= None):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        if (fn!=None and docID==None):
            print fn
            unhiddenImPath= self.pathManager_obj[dsetname].unhide(fn);
            assert( self.docMap[dsetname].containsFn(unhiddenImPath) );
            docID= self.docMap[dsetname].getDocID(unhiddenImPath);
        
        startFrom= int(startFrom);
        numberToReturn= int(numberToReturn);
        
        if docID==None and uploadID==None:
            print "searchPage image: docID or uploadID!";
            return 0;
        if docID!=None:
            docID= int(docID);
            querySpec="docID=%d" % docID;
            qtype= 'dsetimage';
        else:
            querySpec="uploadID=%s" % uploadID;
            qtype= 'image';
        
        headExtra= """
        <script src="static/scripts/jquery.min.130705.js"></script>
        <script src="static/scripts/jquery.Jcrop.min.130705.js"></script>
        <link rel="stylesheet" href="static/scripts/jquery.Jcrop.130705.css" type="text/css" />
        <script src="static/scripts/vgscripts.130705.js"></script>
        <script language="javascript">
        jQuery(function(){ jQuery('#cropbox').Jcrop({ onChange: recCoords, onSelect: recCoords}); });
        function recCoords(c){ selXMin = c.x; selYMin = c.y; selXMax = c.x2; selYMax = c.y2; selWidth = c.w; selHeight = c.h; }
        URLBASE= 'dosearch?%s';
        </script>
        """ % querySpec;
        
        body= "";                
        imw_limit= 800; imh_limit= 800;
        
        if uploadID==None:
            filename= self.pathManager_obj[dsetname].hide(self.docMap[dsetname].getFn(docID));
            imw, imh= self.docMap[dsetname].getWidthHeight(docID);
        else:
            st= savedTemp.load(uploadID);
            filename= st['originalFullFilename'];
            imw, imh= Image.open( st['localFilename_jpg'] ).size;

        scale= min( float(imw_limit)/imh, float(imh_limit)/imw );
               

        body+='name: %s<br><br>\n' % filename;
        body+='<a href="getImageFull?%s&dsetname=%s">Full size image</a><br><br>' % (querySpec, dsetname);

        body+= '<center><img id="cropbox" src="getImage?%s&dsetname=%s&width=%d&height=%d"></center><br>\n' % (querySpec, dsetname, imw_limit, imh_limit);

        body+= '''
        <center>
        <input type="button" value="Search" onclick="javascript:selSearch(event, %.4f)" style="width: 400px; height:100px; font-size:30">
        <br><br>
        Note: Click on the image and drag to select a query region.
        </center>
        ''' % scale;
        
        return self.pT.get(title= "Search", headExtra= headExtra, body= body);
    
    
    
    @cherrypy.expose
    def getImageFull(self, docID= None, uploadID= None, dsetname= None):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        if docID==None and uploadID==None:
            print "searchPage getImageFull: need docID or uploadID!";
            return 0;
        
        cherrypy.response.headers['Content-Type'] = 'image/jpg';
        if uploadID==None:
            fn= self.pathManager_obj[dsetname].getFullSize(self.docMap[dsetname].getFn(int(docID)));
        else:
            st= savedTemp.load(uploadID);
            fn= st['localFilename_full_jpg'];
        im= Image.open( fn );
        imStr = StringIO.StringIO();
        
        try:
            im.save(imStr, "JPEG");
        except IOError:
            # for ballads IBBA, get this error:
            # IOError: decoder group4 not available
            # since PIL doesn't support this compression..
            
            # convert to decompressed tiff
            class AttrChangeFilter:
                def __init__(self, image, new_metadata):
                    self.image = image
                    self.new_metadata = new_metadata
                def __getattr__(self, attr):
                    return getattr(self.image, attr)
                def get_metadata(self):
                    return self.new_metadata
                metadata = property(get_metadata, None, None)
            r= pytiff.TiffReader( fn );
            tmpfn= "tmp/%s.tiff" % str(uuid.uuid4());
            w= pytiff.TiffWriter( tmpfn );
            for p in r:
                m = dict(p.metadata)
                del m["compression"];
                p = AttrChangeFilter(p, m)
                w.append(p)
            # load decompressed
            im= Image.open(tmpfn);
            im.save(imStr, "JPEG");
            # delete decompressed temp file
            os.remove( tmpfn );
        
        return imStr.getvalue();
        #serve_file( self.fullSizePathGetter.getFullSize(self.docMap[dsetname].getFn(int(docID))) );
        
