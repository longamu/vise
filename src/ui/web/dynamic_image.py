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

import cherrypy, os;

import numpy as np;
import Image;
try:
    import PngImagePlugin, JpegImagePlugin, TiffImagePlugin, GifImagePlugin, BmpImagePlugin, PpmImagePlugin; # all this stuff for cx_freeze
except:
    pass;
import ImageDraw;
import StringIO;

from upload import savedTemp;


import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();
uploadDir= os.path.join( scriptroot, 'tmp/uploaded/' );

uploadDir= os.path.abspath( os.path.join( os.path.split(__file__)[0], 'tmp/uploaded/' ) )+'/';


class dynamicImage:
    
    
    def __init__(self, docMap):
        self.docMap= docMap;
        self.def_dsetname= self.docMap.keys()[0];
        
        
    @cherrypy.expose
    def index(self, fn= None, docID= None, uploadID= None, H=None, xl=None, xu=None, yl=None, yu=None, width= None, height=None, drawBox="true", crop=None, dsetname= None):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        cherrypy.response.headers['Content-Type'] = 'image/jpeg';
        return self.getImage( docID= docID, uploadID= uploadID, H= H, xl= xl, xu= xu, yl= yl, yu= yu, width= width, height= height, drawBox= drawBox, crop= crop, dsetname= dsetname );
    
    
    
    def getImage( self, docID= None, uploadID= None, H=None, xl=None, xu=None, yl=None, yu=None, width= None, height=None, drawBox="true", crop=None, dsetname= None):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        if docID==None and uploadID==None:
            print "dynamicImage getImage: docID or uploadID!";
            return 0;
        
        if uploadID==None:
            
            docID= int(docID);
            
            fn= os.path.abspath( self.docMap[dsetname].getFn(docID) );
        
        else:
            st= savedTemp.load(uploadID);
            fn= st['localFilename_jpg'];
        
        return dynamicImage.getImageFromFile( fn, H= H, xl= xl, xu= xu, yl= yl, yu= yu, width= width, height= height, drawBox= drawBox, crop= crop );
    
    
    
    @staticmethod
    def getImageFromFile( fn, H=None, xl=None, xu=None, yl=None, yu=None, width= None, height=None, drawBox="true", crop= None):
        
        im= Image.open(fn);
        imw, imh= im.size;
        
        if (xl!=None and xu!=None and yl!=None and yu!=None):
            xl= float(xl); xu= float(xu); yl= float(yl); yu= float(yu);
        
        if H!=None:
            H_= [float(hi) for hi in H.split(',')];
            H= np.array( H_ ).reshape( (3,3) );
            X= np.array([[xl,yl,1.0], [xu,yl,1.0], [xu,yu,1.0], [xl,yu,1.0]]).T;
        
        crop= (crop!=None) and (xl!=None and xu!=None and yl!=None and yu!=None);
        
        drawBox= (drawBox=="true") and not(crop);
        
        if crop:
            if H==None:
                cropBox= (xl,yl,xu,yu);
            else:
                XP = np.dot(H, X);
                for pointI in range(0,4):
                    XP[:,pointI] /= XP[2,pointI];
                cropBox= ( XP.min(axis=1)[0], XP.min(axis=1)[1], XP.max(axis=1)[0], XP.max(axis=1)[1]);
            cropBox= tuple([int(tmp_+0.5) for tmp_ in cropBox]);
            cropw= cropBox[2]-cropBox[0];
            croph= cropBox[3]-cropBox[1];
        else:
            cropw,croph= imw,imh;
        
        if (H!=None and drawBox) or width!=None or height!=None:
            
            # if you change scale computation don't forget to change do_search scale!!
            
            if width==None and height==None:
                scale= 1;
            else:
                if width==None:
                    scale= float(height)/croph;
                elif height==None:
                    scale= float(width)/cropw;
                else:
                    scale= min( float(width)/cropw, float(height)/croph );
                if crop:
                    cropBox= (cropBox[0]*scale, cropBox[1]*scale, cropBox[2]*scale, cropBox[3]*scale);
                    cropBox= tuple([int(tmp_+0.5) for tmp_ in cropBox]);
            
            if scale < 1:
                im.thumbnail( ( int(imw*scale), int(imh*scale) ) );
            else:
                im= im.resize( ( int(imw*scale), int(imh*scale) ) );
            
            if H!=None and drawBox:
                
                S= np.array( [[scale,0,0],[0,scale,0],[0,0,1]] );
                X= np.array([[xl,yl,1.0], [xu,yl,1.0], [xu,yu,1.0], [xl,yu,1.0]]).T;
                XP = np.dot(np.dot(S, H), X);
                for pointI in range(0,4):
                    XP[:,pointI] /= XP[2,pointI];
                
                if (im.mode!='RGB'):
                    im= im.convert('RGB');
                imd = ImageDraw.Draw(im);
                linecol = (255,255,0);
                imd.line( (XP[0,0], XP[1,0], XP[0,1], XP[1,1]), linecol );
                imd.line( (XP[0,1], XP[1,1], XP[0,2], XP[1,2]), linecol );
                imd.line( (XP[0,2], XP[1,2], XP[0,3], XP[1,3]), linecol );
                imd.line( (XP[0,3], XP[1,3], XP[0,0], XP[1,0]), linecol );
        
        if crop:
            im= im.crop( cropBox );
        imStr = StringIO.StringIO();
        im.save(imStr, "JPEG");
        return imStr.getvalue();
        