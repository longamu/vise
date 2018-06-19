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

import cherrypy;
import os.path;
from PIL import Image;

import numpy as np;

import json;

import multiprocessing;
from multiprocessing import Pool, Lock;
manager= multiprocessing.Manager();


from upload import savedTemp;


import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();
tmpDir=    os.path.join( scriptroot, 'tmp/' );
uploadDir= os.path.join( scriptroot, 'tmp/uploaded/' );


class webAPI:
    
    
    
    def __init__(self, serveraddress, serverroot, upload_obj, API_obj, docMap, pathManager_obj):
        
        self.serveraddress= serveraddress;
        self.serverroot= serverroot;
        self.docMap= docMap;
        self.pathManager_obj= pathManager_obj;
        
        self.upload_obj= upload_obj;
        self.API_obj= API_obj;
        
        self.def_dsetname= self.docMap.keys()[0];
    
    
    
    def getXMLresponse( self, results ):
        
        response= '<xml>\n';
        
        for (rank, docIDres, score, H) in results:
            
            decideMatch= (score>=8.0);
            
            filename= self.pathManager_obj.hide(self.docMap.getFn(docIDres));
            
            imageUrl= "%s%sgetImageFull?docID=%d" % (self.serveraddress, self.serverroot, docIDres);
            
            if decideMatch: confidence= 'High';
            else: confidence= 'Low';
            
            response+= '''<result>
<rank>%d</rank>
<fileName>%s</fileName>
<score>%.5f</score>
<confidence>%s</confidence>
<imageUrl>%s</imageUrl>
<robotsID>%s</robotsID>
</result>
''' % (rank+1, filename, score, confidence, imageUrl, docIDres);
        
        response+= '\n</xml>';
        
        return response;
    
    
    
    @cherrypy.expose
    def uploadAPI( self, uploadURL, startFrom= "1", numberToReturn= "50", dsetname= None ):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        startFrom= int(startFrom)-1;
        numberToReturn= int(numberToReturn);
        
        # upload
        
        uploadID= self.upload_obj.uploadAPI( uploadURL, dsetname= dsetname );
        
        # query
        
        results= self.API_obj[dsetname].externalQuery( os.path.join(tmpDir, "%s.compdata" % uploadID), startFrom= startFrom, numberToReturn= numberToReturn );
        
        cherrypy.response.headers['Content-Type'] = 'text/xml';
        return self.getXMLresponse( results );
    
    
    
    @cherrypy.expose
    def api_version( self ):
        
        ret= {'protocol_version': 1.0,
              'engine_version': 1.0,
              'engine_name':'vgg_relja',
              'supported_datasets': self.API_obj.keys() };
        
        return json.dumps( ret );
    
    
    
    @cherrypy.expose
    def api_engine_reachable( self ):
        
        ret= 1;
        
        for dsetname in self.API_obj:
            if not( self.API_obj[dsetname].running() ):
                ret= 0;
                break;
        
        return json.dumps( ret );
    
    
    
    def uploadOne( self, i, lock, dsetname, sharedMem ):
        
        uploadID= self.upload_obj.uploadAPI( sharedMem.imageURLs[i], dsetname= dsetname );
        
        lock.acquire();
        sharedMem.uploadIDs= sharedMem.uploadIDs + [uploadID,];
        lock.release();
    
    
    
    @staticmethod
    def adjustRoi( roi, uploadID ):
        
        xl, xu, yl, yu= roi;
        
        if xl!=None or xu!=None or yl!=None or yu!=None:
            # uploaded image was potentially resized, adjust the ROI
            st= savedTemp.load(uploadID);
            imwNew, imhNew= Image.open( st['localFilename_jpg'] ).size;
            imwOrig, imhOrig= Image.open( st['localFilename'] ).size;
            if xl!=None: xl= xl/imwOrig*imwNew;
            if xu!=None: xu= xu/imwOrig*imwNew;
            if yl!=None: yl= yl/imhOrig*imhNew;
            if yu!=None: yu= yu/imhOrig*imhNew;
        
        return [xl, xu, yl, yu];
    
    
    
    @cherrypy.expose
    def api_exec_query( self, q= None, roi= None, qtype='imageurl', dsetname= None, page= "1", pagelen= "20" ):
        
        if dsetname==None: dsetname= self.def_dsetname;
        
        
        page= int(page);
        pagelen= int(pagelen);
        
        if page==-1:
            startFrom= 0;
            numberToReturn= 1000;
        else:
            startFrom= (page-1)*pagelen;
            numberToReturn= pagelen;
        
        
        results= None;
        postrainimg_paths= None;
        xl, xu, yl, yu= None, None, None, None;
        
        if qtype=='imageurl':
            
            imageURLs= q.split(',');
            numImages= len(imageURLs);
        
        elif qtype=='dsetimage':
            
            numImages= 1;
        
        
        
        # process ROIs
        
        if qtype in ['imageurl', 'dsetimage']:
            
            if roi== None:
                rois= [ [None,None,None,None] ]*numImages;
            else:
                rois_= roi.split(',');
                if len(rois_)!=numImages:
                    ret= { 'status': 800, 'ranking': [], 'message': 'The list of ROIs must have the same length as the list of images in q' };
                    return json.dumps( ret );
                rois= [];
                for roi in rois_:
                    xy= [coord.strip() for coord in roi.split(':')];
                    if len(xy)==1:
                        # empty list
                        rois.append([None,None,None,None]);
                    else:
                        if len(xy)%2!=0:
                            ret= { 'status': 800, 'ranking': [], 'message': 'One of the ROIs does not have an even number of coordinates (roi=%s)' % roi };
                            return json.dumps( ret );
                        else:
                            try:
                                xy= np.reshape( [float(coord) for coord in xy], (len(xy)/2,2) );
                                x= xy[:,0];
                                y= xy[:,1];
                                rois.append( [x.min(), x.max(), y.min(), y.max()] );
                                
                            except Exception:
                                ret= { 'status': 800, 'ranking': [], 'message': 'Error parsing a ROI (roi=%s)' % roi };
                                return json.dumps( ret );
                    
            
            print "----------- api_exec_query: %d\n\n" % numImages;
            print "rois: ", rois;
        
        
        
        if qtype=='imageurl':
            
            if numImages>1:
                
                pool= [];
                sharedMem= manager.Namespace();
                sharedMem.uploadIDs= [];
                sharedMem.imageURLs= imageURLs;
                mqLock= Lock();
                
                print "----------- api_exec_query: starting pool";
                for i in range(0, numImages):
                    process_= multiprocessing.Process( target= self.uploadOne, args=(i,mqLock,dsetname,sharedMem) );
                    pool.append( process_ );
                    process_.start();
                while pool:
                    print "----------- api_exec_query: joining";
                    process_= pool.pop();
                    process_.join( timeout= None );
                    del process_;
                print "----------- api_exec_query: querying";
                
                
                roisFinal= [ webAPI.adjustRoi(roi,uploadID) for (roi,uploadID) in zip(rois,sharedMem.uploadIDs) ];
                
                compData= [];
                for uploadID in sharedMem.uploadIDs:
                    st= savedTemp.load(uploadID);
                    compData.append(st['compDataFilename']);
                    del st;
                
                results= self.API_obj[dsetname].multiQuery( compData, rois= roisFinal, startFrom= startFrom, numberToReturn= numberToReturn );
                
                print "----------- api_exec_query: done querying";
            
            else:
                
                # upload
                
                uploadID= self.upload_obj.uploadAPI( q, dsetname= dsetname );
                st= savedTemp.load(uploadID);
                
                # ROI
                if rois[0][0]==None:
                    rois[0]= [0, st['w'], 0, st['h']];
                xl, xu, yl, yu= webAPI.adjustRoi(rois[0],uploadID);
                
                # query
                
                results= self.API_obj[dsetname].externalQuery( st['compDataFilename'], xl= xl, xu= xu, yl= yl, yu= yu, startFrom= startFrom, numberToReturn= numberToReturn );
                del st;
            
            
            
        elif qtype=="dsetimage":
            
            xl, xu, yl, yu= rois[0];
            if not(q.endswith('jpg')):
                q= q+'.jpg';
            docID= self.docMap[dsetname].getDocID( str(q) );
            if xl==None: xl= 0;
            if yl==None: yl= 0;
            if xu==None or yu==None: imw, imh= self.docMap[dsetname].getWidthHeight(docID);
            if xu==None: xu= imw;
            if yu==None: yu= imh;
            results= self.API_obj[dsetname].internalQuery( docID= docID, xl= xl, xu= xu, yl= yl, yu= yu, startFrom= startFrom, numberToReturn= numberToReturn );
        
        
        
        
        if results!=None:
            
            # put in response format
            
            status= 100;
            ranking= [];
            
            for (rank, docID, score, H) in results:
                
                #decideMatch= (score>=6.0);
                #decideMatch= (score>=3.0);
                decideMatch= True;
                
                if not(decideMatch):
                    break;
                
                filename= self.pathManager_obj[dsetname].hide(self.docMap[dsetname].getFn(docID));
                thisRes= {'path': filename, 'score':score};
                if xl!=None and xu!=None and yl!=None and yu!=None and H!=None:
                    H_= [float(hi) for hi in H.split(',')];
                    H= np.array( H_ ).reshape( (3,3) );
                    X= np.array([[xl,yl,1.0], [xu,yl,1.0], [xu,yu,1.0], [xl,yu,1.0]]).T;
                    XP = np.dot(H, X);
                    for pointI in range(0,4):
                        XP[:,pointI] /= XP[2,pointI];
                    roiOut= [];
                    for pointI in range(0,4):
                        roiOut.extend( [float(XP[0,pointI]), float(XP[1,pointI])] );
                    thisRes['roi']= ':'.join(['%.2f'%num for num in roiOut]);
                ranking.append( thisRes );
        
        else:
            
            status= 800;
            ranking= [];
        
        ret= { 'status': status, 'ranking': ranking };
        if postrainimg_paths!=None:
            ret['trainimgs']= postrainimg_paths;
        return json.dumps( ret );
