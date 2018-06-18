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

import Image;
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
        width1= Image.open( os.path.join( outFnPrefix, '%s_%s.jpg' % (registerID,"im1") ) ).size[0];
        
        title= "Image comparison";
        
        headExtra= """
<script language="javascript">
    
    var isIE = document.all ? true : false;
    document.onmousemove = getMousePosition;
    
    jsIm1 = new Image();
    jsIm2t= new Image();
    jsIm1.src ="tmpImage?registerID=%s&imName=im1";
    jsIm2t.src="tmpImage?registerID=%s&imName=im2t";
    
    var currentImage= 1;
    
    function getMousePosition(e){
        if (!isIE) {
            posX= e.pageX; posY= e.pageY;
        }
        if (isIE) {
            posX= event.clientX + document.body.scrollLeft;
            posY= event.clientY + document.body.scrollTop;
        }
    }
    
    function changeTo1(){
        document['image'].src= jsIm1.src;
        currentImage= 1;
    }

    function changeTo2(){
        document['image'].src= jsIm2t.src;
        currentImage= 2;
    }
    
    function swapImage(){
        if (currentImage==1){
            changeTo2();
        } else {
            changeTo1();
        }
    }
    
    function findPosX( obj ){
        x= 0;
        if (obj.offsetParent){
            while (1) {
                x+= obj.offsetLeft;
                if (!obj.offsetParent) break;
                obj= obj.offsetParent;
            }
        }
        return x;
    }

    function mouseMove( obj, e ){
        clickX= posX - findPosX(obj);
        if (clickX > (obj.width)/2){
            changeTo2();
        } else {
            changeTo1();
        }
    }

</script>
    """ % (registerID, registerID);
        
        body= """
<center>
<table>

<tr>
    
    <td align="center">
        <center>Image 1</center>
    </td>
    
    <td align="center">
        Flip between images by moving the mouse to the left (image 1) or right (image 2) part of the image.
    </td>
    
    <td align="center">
        <center>Image 2</center>
    </td>
    
</tr>

<tr>
    <td align="center">
        <img name="im1" onmouseover="javascript:changeTo1();" onmouseclick="javascript:changeTo1();">
        <script language="javascript">
            document['im1'].src= jsIm1.src
        </script>
    </td>
    
    <td align="center">
        <img name="image" onmousemove="javascript:mouseMove(this);" onmouseclick="javascript:swapImage();">
        <script language="javascript">
            changeTo1();
        </script>
    </td>
    
    
    <td align="center">
        <img name="im2" src="tmpImage?registerID=%s&imName=im2&width=%d" onmouseover="javascript:changeTo2();" onmouseclick="javascript:changeTo2();">
    </td>
</tr>

<tr>
    <td align="center">
    <a href="getImageFull?%s">High resolution full image</a><br>
    </td>
    <td></td>
    <td align="center">
    <a href="getImageFull?docID=%s">High resolution full image</a><br>
    </td>
</tr>

<tr>
    <td align="center">
    <a href="search?%s">Search on full image</a><br>
    </td>
    <td></td>
    <td align="center">
    <a href="search?docID=%s">Search on full image</a><br>
    </td>
</tr>

</table>
</center>

""" % ( registerID, width1, \
        ("docID=%s" % docID1) if uploadID1==None else ("uploadID=%s" % uploadID1), docID2, \
        ("docID=%s" % docID1) if uploadID1==None else ("uploadID=%s" % uploadID1), docID2 );
        
        return self.pT.get(title= title, headExtra= headExtra, body= body, outOfContainer= True);
        
    
    
    @cherrypy.expose
    def tmpImage(self, registerID, imName, width= None):
        
        outFnPrefix= os.path.join( scriptroot, 'tmp' );
        fn= os.path.join( outFnPrefix, '%s_%s.jpg' % (registerID,imName) );
        
        # for security check filename - !!TODO
        
        cherrypy.response.headers['Content-Type'] = 'image/jpeg';
        return dynamicImage.getImageFromFile( fn, width= width );
        