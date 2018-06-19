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

import os, sys;
import cherrypy;
import uuid;
from PIL import Image
try:
    PngImagePlugin, JpegImagePlugin, TiffImagePlugin, GifImagePlugin, BmpImagePlugin, PpmImagePlugin; # all this stuff for cx_freeze
except:
    pass;
from PIL import ImageDraw, ImageFont;
import StringIO;

import numpy as np;

try:
    from jp_draw import jp_draw;
except:
    pass;
from upload import savedTemp;

import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();
tmpDir=    os.path.join( scriptroot, 'tmp/' );
uploadDir= os.path.join( scriptroot, 'tmp/uploaded/' );




class details:
    
    
    
    def __init__(self, pageTemplate, API_obj, docMap, pathManager_obj, doRegistration= True):
        self.pT= pageTemplate;
        self.API_obj= API_obj;
        self.docMap= docMap;
        self.pathManager_obj= pathManager_obj;
        self.def_dsetname= self.docMap.keys()[0];
        self.doRegistration= doRegistration;
    
    
    
    @cherrypy.expose
    def index(self, xl, xu, yl, yu, H= None, docID1= None, uploadID1= None, docID2= None, drawPutative= "false", drawBoxes= "true", drawLines= "false", drawRegions= "false", dsetname= None ):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        if uploadID1==None:
            docID1= int(docID1);
            querySpec0= "docID=%d" % docID1;
            querySpec1= "docID1=%d" % docID1;
        else:
            querySpec0= "uploadID=%s" % uploadID1;
            querySpec1= "uploadID1=%s" % uploadID1;
        
        docID2= int(docID2);
        
        headExtra= """
        <script language="javascript">
        
        refreshURL= "details?%s&docID2=%d&xl=%s&xu=%s&yl=%s&yu=%s%s";
        
        function redraw(){
            var drawPutative= "false";
            var drawBoxes= "false";
            var drawLines= "false";
            var drawRegions= "false";
            if (document.getElementById('drawPutative').checked) {
                drawPutative= "true";
            }
            if (document.getElementById('drawBoxes').checked) {
                drawBoxes= "true";
            }
            if (document.getElementById('drawLines').checked) {
                drawLines= "true";
            }
            if (document.getElementById('drawRegions').checked) {
                drawRegions= "true";
            }
            document.location.href= refreshURL+"&drawPutative="+drawPutative+"&drawBoxes="+drawBoxes+"&drawLines="+drawLines+"&drawRegions="+drawRegions;
        }
        
        function check(s){
            el= document.getElementById(s);
            el.checked= !el.checked;
        }
        
        </script>
        """ % ( querySpec1, docID2, xl, xu, yl, yu, ("&H=%s" % H) if H!=None else "" );
        
        body= "";
        body+= "<center>";
        
        body+= """
        <img src="drawMatches?%s&docID2=%d&xl=%s&xu=%s&yl=%s&yu=%s%s&drawPutative=%s&drawBoxes=%s&drawLines=%s&drawRegions=%s">
        """ % ( querySpec1, docID2, xl, xu, yl, yu, ("&H=%s" % H) if H!=None else "", drawPutative, drawBoxes, drawLines, drawRegions );
        
        drawPutative= (drawPutative=="true") or (H==None);
        drawBoxes= (drawBoxes=="true");
        drawLines= (drawLines=="true");
        drawRegions= (drawRegions=="true");
        
        putativeChecked= "checked" if drawPutative else "";
        boxesChecked= "checked" if drawBoxes else "";
        linesChecked= "checked" if drawLines else "";
        regionsChecked= "checked" if drawRegions else "";
        
        body+= "<br>";
        if H==None:
            body+= """
            <input type="checkbox" id="drawPutative" checked disabled> Putative &nbsp;&nbsp;
            """;
        else:
            body+= """
            <input type="checkbox" id="drawPutative" %s> <a href="javascript:check('drawPutative');">Putative</a> &nbsp;&nbsp;
            """ % putativeChecked;
        body+= """
        <input type="checkbox" id="drawBoxes" %s>       <a href="javascript:check('drawBoxes');">Boxes</a> &nbsp;&nbsp;
        <input type="checkbox" id="drawLines" %s>       <a href="javascript:check('drawLines');">Lines</a> &nbsp;&nbsp;
        <input type="checkbox" id="drawRegions" %s>     <a href="javascript:check('drawRegions');">Regions</a> &nbsp;&nbsp;
        <input type="button" value="Draw again" onclick="javascript:redraw()">
        <br><br>
        """ % (boxesChecked, linesChecked, regionsChecked, );
        
        
        if True:
            
            maxImageW= 370;
            maxImageH= 400;
            
            if uploadID1==None:
                queryFn= self.pathManager_obj[dsetname].hidePath(docID1);
            else:
                st= savedTemp.load(uploadID1);
                queryFn= st['originalFullFilename'];
                del st;
            
            body+= """
            <table width="100%%">
            
            <tr>
                <td align="center">
                    name: %s
                </td>
                <td align="center">
                    name: %s
                </td>
            </tr>
            
            <tr>
            <td width="50%%" align="center">
                <a href="getImageFull?%s">
                <img src="getImage?%s&xl=%s&xu=%s&yl=%s&yu=%s&width=%d&height=%d&crop">
                </a>
            </td>
            
            <td width="50%%" align="center">
                <a href="getImageFull?docID=%d">
                <img src="getImage?docID=%d%s&drawBox=false&width=%d&height=%d&crop">
                </a>
            </td>
            </tr>
            
            <tr>
                <td align="center">
                <a href="getImageFull?%s">High resolution full image</a><br>
                </td>
                <td align="center">
                <a href="getImageFull?docID=%d">High resolution full image</a><br>
                </td>
            </tr>
            
            <tr>
                <td align="center">
                <a href="search?%s">Search on full image</a><br>
                </td>
                <td align="center">
                <a href="search?docID=%d">Search on full image</a><br>
                </td>
            </tr>
            
            </table>
            """ % ( queryFn,
                    self.pathManager_obj[dsetname].hidePath(docID2), \
                    querySpec0, querySpec0, xl,xu,yl,yu, maxImageW, maxImageH, \
                    docID2, docID2, ("&xl=%s&xu=%s&yl=%s&yu=%s&H=%s" % (xl,xu,yl,yu,H)) if H!=None else "", \
                    maxImageW, maxImageH, \
                    querySpec0, docID2, querySpec0, docID2 );
        
        
        if not(drawPutative) and self.doRegistration:
            
            body+= """
        <br>
        <a href="register?%s&docID2=%d&xl=%s&xu=%s&yl=%s&yu=%s">Image comparison</a><br><br><br>
        """ % (querySpec1, docID2, xl,xu,yl,yu);
            
        body+= "</center>";
        
        return self.pT.get(title= "Detailed Matches", headExtra= headExtra, body= body);
    
    
    @cherrypy.expose
    def drawMatches(self, xl, xu, yl, yu, H= None, docID1= None, uploadID1= None, docID2= None, horizontal="true", vertical=None, drawPutative= "false", drawBoxes= "true", drawLines= "true", drawRegions= "true", dsetname= None ):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        drawPutative= (drawPutative=="true");
        drawBoxes= (drawBoxes=="true");
        drawLines= (drawLines=="true");
        drawRegions= (drawRegions=="true");
        
        reljaDraw= False; # if this changes remove jp_draw !
        
        cherrypy.response.headers['Content-Type'] = 'image/jpeg';
        
        if docID1!=None:
            docID1= int(docID1);
        if docID2!=None:
            docID2= int(docID2);
        if xl!=None: xl= float(xl);
        if xu!=None: xu= float(xu);
        if yl!=None: yl= float(yl);
        if yu!=None: yu= float(yu);
        
        if uploadID1==None:
            if drawPutative:
                matches= self.API_obj[dsetname].getPutativeInternalMatches( docID1= docID1, docID2= docID2, xl= xl, xu= xu, yl= yl, yu= yu );
            else:
                matches= self.API_obj[dsetname].getInternalMatches( docID1= docID1, docID2= docID2, xl= xl, xu= xu, yl= yl, yu= yu );
        else:
            st= savedTemp.load(uploadID1);
            if drawPutative:
                matches= self.API_obj[dsetname].getPutativeExternalMatches( st['compDataFilename'], docID2= docID2, xl= xl, xu= xu, yl= yl, yu= yu );
            else:
                matches= self.API_obj[dsetname].getExternalMatches( st['compDataFilename'], docID2= docID2, xl= xl, xu= xu, yl= yl, yu= yu );
            del st;
        
        if uploadID1==None:
            img_fn1= self.docMap[dsetname].getFn(docID1);
        else:
            st= savedTemp.load(uploadID1);
            img_fn1= st['localFilename_jpg'];
        
        img_fn2= self.docMap[dsetname].getFn(docID2);
        
        show_horizontal= (horizontal!=None) and (vertical==None);
        IMG_W= 400;
        IMG_H= 400;
        GAP= 10;
        LINE_COL= '#00ff00';
        ELLIPSE_COL= '#ff0000';
        BOUNDING_COL= '#ffff00';
        TEXT_SPACE= 50;
        
        w1, h1= Image.open(img_fn1).size;
        w2, h2= Image.open(img_fn2).size;
        
        s1= min( float(IMG_W)/w1, float(IMG_H)/h1 );
        s2= min( float(IMG_W)/w2, float(IMG_H)/h2 );
        
        if (show_horizontal):
            #s1, s2= float(IMG_H)/h1, float(IMG_H)/h2;
            dx2= int(s1*w1)+GAP; dy2= 0;
            total_w= dx2 + int(s2*w2);
            total_h= int(max(s1*h1, s2*h2));
        else:
            #s1, s2 = float(IMG_W)/w1, float(IMG_W)/w2;
            dx2= 0;   dy2= int(s1*h1)+GAP;
            total_w= int(max(s1*w1, s2*w2));
            total_h= dy2 + int(s2*h2);
        
        total_h+= TEXT_SPACE;
        
        # create the image
        if reljaDraw:
            im= Image.new( "RGB", (total_w,total_h), '#FFFFFF' );
        else:
            draw_obj= jp_draw.image( total_w, total_h );
        
        # Draw the images
        dx2= (dx2+total_w-int(s2*w2))/2;
        dy2= (dy2+total_h-TEXT_SPACE-int(s2*h2))/2;
        if reljaDraw:
            im1thumb= Image.open(img_fn1); im1thumb.thumbnail( (int(s1*w1),int(s1*h1)) );
            th1w, th1h= im1thumb.size;
            im2thumb= Image.open(img_fn2); im2thumb.thumbnail( (int(s2*w2),int(s2*h2)) );
            th2w, th2h= im2thumb.size;
            im.paste( im1thumb, (0,0,th1w,th1h) );
            im.paste( im2thumb, (dx2,dy2,dx2+th2w,dy2+th2h) );
            imd= ImageDraw.Draw(im);
        else:
            # copy images in case of different extension
            shoulddel1, img_fn1= details.precopy( img_fn1 );
            shoulddel2, img_fn2= details.precopy( img_fn2 );
            draw_obj.image(img_fn1, 0, 0, int(s1*w1), int(s1*h1));
            draw_obj.image(img_fn2, dx2, dy2, int(s2*w2), int(s2*h2));
        
        
                
        # draw boxes
        if drawBoxes:
            
            # first image
            xl*= s1; yl*= s1; xu*= s1; yu*= s1;
            if reljaDraw:
                imd.line( (xl, yl, xu, yl), BOUNDING_COL);
                imd.line( (xu, yl, xu, yu), BOUNDING_COL);
                imd.line( (xu, yu, xl, yu), BOUNDING_COL);
                imd.line( (xl, yu, xl, yl), BOUNDING_COL);
            else:
                draw_obj.line(xl, yl, xu, yl, 1.0, BOUNDING_COL);
                draw_obj.line(xu, yl, xu, yu, 1.0, BOUNDING_COL);
                draw_obj.line(xu, yu, xl, yu, 1.0, BOUNDING_COL);
                draw_obj.line(xl, yu, xl, yl, 1.0, BOUNDING_COL);
            xl/= s1; yl/= s1; xu/= s1; yu/= s1;
            
            if not(drawPutative):
                
                #second image
                H_= [float(hi) for hi in H.split(',')];
                H= np.array( H_ ).reshape( (3,3) );
                S= np.array( [[s2,0,0],[0,s2,0],[0,0,1]] );
                X= np.array([[xl,yl,1.0], [xu,yl,1.0], [xu,yu,1.0], [xl,yu,1.0]]).T;
                XP = np.dot(np.dot(S, H), X);
                for pointI in range(0,4):
                    XP[:,pointI] /= XP[2,pointI];
                XP[0,:]+= dx2;
                XP[1,:]+= dy2;
                if reljaDraw:
                    imd.line( (XP[0,0], XP[1,0], XP[0,1], XP[1,1]), BOUNDING_COL );
                    imd.line( (XP[0,1], XP[1,1], XP[0,2], XP[1,2]), BOUNDING_COL );
                    imd.line( (XP[0,2], XP[1,2], XP[0,3], XP[1,3]), BOUNDING_COL );
                    imd.line( (XP[0,3], XP[1,3], XP[0,0], XP[1,0]), BOUNDING_COL );
                else:
                    draw_obj.line( XP[0,0], XP[1,0], XP[0,1], XP[1,1], 1.0, BOUNDING_COL );
                    draw_obj.line( XP[0,1], XP[1,1], XP[0,2], XP[1,2], 1.0, BOUNDING_COL );
                    draw_obj.line( XP[0,2], XP[1,2], XP[0,3], XP[1,3], 1.0, BOUNDING_COL );
                    draw_obj.line( XP[0,3], XP[1,3], XP[0,0], XP[1,0], 1.0, BOUNDING_COL );
        
        
        # draw matches
        if drawLines or drawRegions:
            for (el1,el2) in matches:
                
                x1,y1,a1,b1,c1= el1;
                x2,y2,a2,b2,c2= el2;
                x1*= s1; y1*= s1; a1/= s1**2; b1/= s1**2; c1/= s1**2;
                x2*= s2; y2*= s2; a2/= s2**2; b2/= s2**2; c2/= s2**2;
                
                if drawLines:
                    if reljaDraw:
                        imd.line( (x1, y1, dx2 + x2, dy2 + y2), LINE_COL);
                    else:
                        draw_obj.line(x1, y1, dx2 + x2, dy2 + y2, 1.0, LINE_COL);
                if drawRegions:
                    if reljaDraw:
                        details.drawEllipse( im, x1, y1, a1, b1, c1, ELLIPSE_COL );
                        details.drawEllipse( im, dx2 + x2, dy2 + y2, a2, b2, c2, ELLIPSE_COL );
                    else:
                        draw_obj.ellipse(x1, y1, a1, b1, c1, 1.0, ELLIPSE_COL, 'none');
                        draw_obj.ellipse(dx2 + x2, dy2 + y2, a2, b2, c2, 1.0, ELLIPSE_COL, 'none');
        
        if True:
            if drawPutative:
                text= '#putative= %d' % len(matches);
            else:
                text= '#inliers= %d' % len(matches);
            if reljaDraw:
                font= ImageFont.truetype( os.path.join(scriptroot,"resource/FreeSerif.ttf"),20);
                imd.text( (20, total_h - TEXT_SPACE + 5), text, font= font, fill='#FF0000');
            else:
                draw_obj.text(20, total_h - 5, text, 25.0, '#FF0000');
        
        if not(reljaDraw):
            tmpfn= '/tmp/matches_%s.png' % str(uuid.uuid4());
            draw_obj.save(tmpfn);
            im= Image.open(tmpfn);
            try:
                os.remove(tmpfn);
            except OSError, e:
                pass;
        
        fo = StringIO.StringIO();
        im.save(fo, "PNG");
        
        
        if not(reljaDraw):
            # remove jp_draw precopied images..
            try:
                if (shoulddel1):
                    os.remove(img_fn1);
                if (shoulddel2):
                    os.remove(img_fn2);
            except OSError, e:
                pass
        
        return fo.getvalue();
    
    
    @staticmethod
    def precopy( img_fn ):
        if not ( img_fn.endswith('.jpg') ):
            tmpimage_fn=os.path.join(scriptroot,'tmp',str(uuid.uuid4()))+'.jpg';
            os.popen('cp "%s" "%s"' % (img_fn, tmpimage_fn));
            return True, tmpimage_fn;
        return False, img_fn;
    
    
    # would like to use this but it's too ugly
    @staticmethod
    def drawEllipse( im, x, y, a, b, c, ELLIPSE_COL ):
        
        # transform circle x^2+y^2=1 into ellipse A= [a,b;b,c], simply
        # A= C^T * C, where C is lower triangular
        # then H= C^(-1), but since Image.transform takes Hinv then just C
        ct= np.sqrt(c); bt= b/ct; at= np.sqrt(a-bt**2);
        Haffinv=[ at, 0, 0,  bt, ct, 0 ];
        
        # figure out the image size: 1/at and 1/ct are eigenvalues,
        # so final image size: width= 1/at *2, height= 1/ct *2
        elW= 1.0/at; elH= 1.0/ct;
        elImW= np.ceil(elW*2); elImH= np.ceil(elH*2);
        
        # transform
        #circleR= 20;
        circleR= np.ceil(max(elW,elH));
        extraSpace= 5;
        imDim= (circleR+extraSpace)*2;
        mask= Image.new( "L", (imDim, imDim), 0 );
        maskd= ImageDraw.Draw(mask);
        maskd.ellipse( (extraSpace,extraSpace,extraSpace+2*circleR,extraSpace+2*circleR), outline= 255 );
        Haffinv[0]*= circleR;
        Haffinv[3]*= circleR; Haffinv[4]*= circleR;
        elImW+= np.ceil(2*extraSpace/(at*float(circleR)));
        elImH+= np.ceil(2*extraSpace/(ct*float(circleR)));
        mask= mask.transform( (elImW,elImH), Image.AFFINE, tuple(Haffinv), Image.NEAREST );
        #mask.show()
        
        #imd.bitmap( (x-float(mask.size[0])/2, y-float(mask.size[1])/2), mask );
        xlu= x-float(mask.size[0])/2;
        ylu= y-float(mask.size[1])/2;
        im.paste( ELLIPSE_COL, (xlu,ylu,xlu+mask.size[0],ylu+mask.size[1]), mask );
    
