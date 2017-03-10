import os, sys, traceback;
import SocketServer, socket;

import json;
import numpy as np;
import threading;

import uuid;
import cPickle;

import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();
tmpDir=    os.path.join( scriptroot, 'tmp/' );


kTcpTerminator= '$$$';



class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    def server_bind(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind(self.server_address);



class ThreadedTCPRequestHandler(SocketServer.BaseRequestHandler):
    
    def handle(self):
        
        # receive data
        alldata= '';
        while 1:
            data= self.request.recv(1024);
            if not data:
                break;
            alldata+= data;
            if alldata.rstrip().endswith(kTcpTerminator):
                break;
        
        alldata= alldata.strip()[: -len(kTcpTerminator)];
        
        response= {'success': False};
        
        try:
            
            req= json.loads(alldata);
        
            print "Request=", req;
            
            func= req['func'];
            
            if func == 'selfTest':
                
                response= self.server.backend.selfTest();
                
            elif func == 'getQueryId':
                
                response= self.server.backend.getQueryId(req['dataset']);
                
            elif func == 'releaseQueryId':
                
                response= self.server.backend.releaseQueryId(req['query_id']);
                
            elif func == 'addPosTrs':
                
                if not('featpath' in req):
                    req['featpath']= None;
                
                if not('extra_params' in req):
                    req['extra_params']= {};
                
                if not('from_dataset' in req):
                    from_dataset= True;
                else:
                    from_dataset= bool(req['from_dataset']);
                
                response= self.server.backend.addPosTrs( \
                    req['query_id'],
                    req['impath'],
                    featpath= req['featpath'],
                    from_dataset= from_dataset,
                    extra_params= req['extra_params']);
                
            elif func == 'addNegTrs':
                
                if not('featpath' in req):
                    req['featpath']= None;
                
                if not('extra_params' in req):
                    req['extra_params']= {};
                
                if not('from_dataset' in req):
                    from_dataset= True;
                else:
                    from_dataset= bool(req['from_dataset']);
                
                response= self.server.backend.addNegTrs( \
                    req['query_id'],
                    req['impath'],
                    featpath= req['featpath'],
                    from_dataset= from_dataset,
                    extra_params= req['extra_params']);
                
            elif func == 'train':
                
                response= self.server.backend.train(req['query_id']);
                
            elif func == 'loadClassifier':
                
                response= self.server.backend.loadClassifier(req['query_id'], req['filepath']);
                
            elif func == 'saveClassifier':
                
                response= self.server.backend.saveClassifier(req['query_id'], req['filepath']);
                
            elif func == 'getAnnotations':
                
                response= self.server.backend.getAnnotations(req['filepath']);
                
            elif func == 'saveAnnotations':
                
                response= self.server.backend.saveAnnotations(req['query_id'], req['filepath']);
                
            elif func == 'rank':
                
                response= self.server.backend.rank(req['query_id']);
                
            elif func == 'getRanking':
                
                response= self.server.backend.getRanking(req['query_id']);
                
            elif func == 'getRankingSubset':
                
                start_idx= req['start_idx'] if ('start_idx' in req) else 0;
                end_idx= req['end_idx'] if ('end_idx' in req) else 1000;
                response= self.server.backend.getRanking(req['query_id']);
                response['ranklist']= response['ranklist'][start_idx:end_idx];
                response['total_len']= len(response['ranklist']);
            
        except Exception, e:
            traceback.print_exc();
        
        print "Request - DONE (success=%s)" % ('True' if response['success'] else 'False');
        self.request.sendall(json.dumps(response));
        print "Wrote output";



class visorBackend:
    
    
    
    def __init__(self, API_obj, datasetOpts, visorBackendHost= "localhost", visorBackendPort= 45200):
        self.API_obj= API_obj;
        
        self.queryID= 1;
        self.queryIDLock= threading.Lock();
        self.queryData= {};
        
        self.results= {};
        self.resultLock= threading.Lock();
        
        self.qidToDset= {};
        self.qidToDsetLock= threading.Lock();
        
        self.visorToDsetname= {};
        self.dsetnameToVisor= {};
        for datasetOpt in datasetOpts:
            dsetname= datasetOpt['dsetname'];
            if not('visorDsetname' in datasetOpt):
                visorDsetname= dsetname;
            else:
                visorDsetname= datasetOpt['visorDsetname'];
            self.visorToDsetname[visorDsetname]= dsetname;
            self.dsetnameToVisor[dsetname]= visorDsetname;
        
        # set up server
        
        self.server= ThreadedTCPServer((visorBackendHost, visorBackendPort), ThreadedTCPRequestHandler);
        self.server.daemon_threads= True;
        self.server.backend= self;
        
        print "visorBackendPort=", visorBackendPort;
        print "Waiting for requests"
        self.server.serve_forever();
    
    
    
    def _convertToVisorResults(self, rankedList, dsetname, queryROI= None):
        
        pm= self.API_obj[dsetname].pathManager_obj;
        prefix= '/'+self.dsetnameToVisor[dsetname];
        results= [];
        for (rank, docID, score, H) in rankedList:
            item= {'path': prefix+pm.hide(pm.docMap.getFn(docID)), 'score': score };
            if queryROI!=None and H!=None:
                H_= [float(hi) for hi in H.split(',')];
                H= np.array( H_ ).reshape( (3,3) );
                X= np.ones((queryROI.shape[0],queryROI.shape[1]+1),dtype='float32');
                X[:,:-1]= queryROI;
                X= X.T;
                XP = np.dot(H, X);
                values= [];
                inds= range(0,queryROI.shape[0]);
                inds.append(0);
                for i in inds:
                    values.append( XP[0,i] );
                    values.append( XP[1,i] );
                item['roi']= '_'.join(['%.2f' % x for x in values]);
            results.append(item);
        return results;
    
    
    
    @staticmethod
    def getTempFile():
        dirname= os.path.join(tmpDir, 'visor');
        try:
            os.makedirs(dirname);
        except OSError:
            pass;
        return os.path.join(dirname, str(uuid.uuid4()));
    
    
    
    def selfTest(self):
        ret= True;
        
        for dsetname in self.API_obj:
            if not( self.API_obj[dsetname].running() ):
                ret= False;
                break;
        
        return {'success': ret};
    
    
    
    def getQueryId(self, visorDsetname):
        
        if visorDsetname in self.visorToDsetname:
            dsetname= self.visorToDsetname[visorDsetname];
        else:
            return {'success': False};
        
        self.queryIDLock.acquire();
        queryID= self.queryID;
        self.queryID= self.queryID % 1234567890 + 1;
        self.queryData[queryID]= {'lock': threading.Lock(), 'dsetname': dsetname, 'querySpecs': [], 'rois': [], 'annos': []};
        self.queryIDLock.release();
        
        self.qidToDsetLock.acquire();
        self.qidToDset[queryID]= dsetname;
        self.qidToDsetLock.release();
        
        return {'success': True, 'query_id': queryID};
    
    
    
    def releaseQueryId(self, query_id):
        
        self.resultLock.acquire();
        if query_id in self.results:
            del self.results[query_id];
        self.resultLock.release();
        
        self.queryIDLock.acquire();
        if query_id in self.queryData:
            del self.queryData[query_id];
        self.queryIDLock.release();
        
        self.qidToDsetLock.acquire();
        if query_id in self.qidToDset:
            del self.qidToDset[query_id];
        self.qidToDsetLock.release();
        
        return {'success': True};
    
    
    
    def addPosTrs(self, query_id, impath, featpath= None, from_dataset= True, extra_params= {}):
        self.qidToDsetLock.acquire();
        
        if query_id in self.qidToDset:
            dsetname= self.qidToDset[query_id];
        else:
            dsetname= None;
        self.qidToDsetLock.release();
        if dsetname==None:
            return {'success': False};
        
        origQuery= {'image': impath, 'anno': "1"};
        if 'roi' in extra_params:
            origQuery['roi']= extra_params['roi'];
        
        # process ROI
        if 'roi' in extra_params:
            roi= extra_params['roi'];
            roi= np.array([float(x) for x in roi]).reshape(len(roi)/2,2);
            xl, yl= roi.min(axis=0);
            xu, yu= roi.max(axis=0);
            if abs(xu-xl)<5 or abs(yu-yl)<5:
                xl, yl, xu, yu, roi = None, None, None, None, None;
        else:
            xl, yl, xu, yu, roi = None, None, None, None, None;
        extra_params['roi']= (xl, yl, xu, yu, roi);
        
        # check if image from dataset
        
        if from_dataset:
            
            prefix= '/'+self.dsetnameToVisor[dsetname];
            if impath.startswith(prefix):
                impath= impath[ len(prefix): ];
            else:
                origQuery['image']= prefix + '/' + origQuery['image'];
            
            pm= self.API_obj[dsetname].pathManager_obj;
            unhidenImPath= pm.unhide(impath);
            
            if not(pm.docMap.containsFn( unhidenImPath )):
                
                # HACK if the query image (for dsetimage query) is not in the database,
                # check if it exists on disk and consider it a uploaded image query
                # This is because Omkar works with more frames (not just keyframes like me),
                # so I should handle his dsetimage queries even if it is not in my dataset
                from_dataset= False;
                impath= unhidenImPath;
                
            else:
                
                # get imageID
                docID= pm.docMap.getDocID( unhidenImPath );
                
                if roi!=None and not(self.API_obj[dsetname].supportsInternalQueryROI()):
                    # If ROI is supplied but internal query doesn't support ROIs (e.g. VLAD),
                    # consider it as an uploaded image (i.e. recompute features and keep the ones in ROI)
                    from_dataset= False;
                    impath= pm.docMap.getFn(docID);
                    featpath= None; # HACK ignore Visor's featpath as it doesn't play well for me
        
        # do the processing
        
        if from_dataset:
            featpath= docID;
        else:
            # uploaded image: extract features
            if featpath==None:
                featpath= visorBackend.getTempFile()+'.compdata';
            if not(os.path.exists(featpath)):
                dirname= os.path.dirname(featpath);
                try:
                    os.makedirs(dirname);
                except:
                    pass
                if not(os.path.exists(dirname)):
                    return {'success': False};
                self.API_obj[dsetname].processImage(impath, featpath, ROI= extra_params['roi'][:4]);
        
        # save the query specification
        
        self.queryIDLock.acquire();
        queryData= self.queryData[query_id];
        self.queryIDLock.release();
        
        queryData['lock'].acquire();
        queryData['querySpecs'].append(featpath);
        queryData['rois'].append(extra_params['roi']);
        queryData['annos'].append(origQuery);
        queryData['lock'].release();
        
        return {'success': True};
    
    
    
    def addNegTrs(self, query_id, impath, featpath= None, from_dataset= True, extra_params= {}):
        return {'success': True};
    
    
    
    def train(self, query_id):
        return {'success': True};
    
    
    
    def loadClassifier(self, query_id, filepath):
        return {'success': False, 'message': "Don't do this with instance search"};
    
    
    
    def saveClassifier(self, query_id, filepath):
        return {'success': False, 'message': "Don't do this with instance search"};
    
    
    
    def getAnnotations(self, filepath):
        
        annos= cPickle.load(open(filepath, 'rb'));
        return {"success":True, "annos": annos};
    
    
    
    def saveAnnotations(self, query_id, filepath):
        
        self.queryIDLock.acquire();
        queryData= self.queryData[query_id];
        self.queryIDLock.release();
        
        queryData['lock'].acquire();
        try:
            cPickle.dump( queryData['annos'], open(filepath, 'wb'), -1 );
        finally:
            queryData['lock'].release();
        
        return {"success":True};
    
    
    
    def rank(self, query_id):
        
        # get query data
        
        self.queryIDLock.acquire();
        if query_id in self.queryData:
            queryData= self.queryData[query_id];
        else:
            queryData= None;
        self.queryIDLock.release();
        if queryData==None:
            return {'success': False};
        
        queryData['lock'].acquire();
        dsetname= queryData['dsetname'];
        querySpecs= queryData['querySpecs'];
        rois= queryData['rois'];
        queryData['lock'].release();
        
        # check all is ok (e.g. returning numpy.uint32 is not ok as there are type()==int checks which fail
        for querySpec in querySpecs:
            if not(type(querySpec) in [int, str, unicode]):
                print "querySpec: ", querySpec, type(querySpec)
                raise ValueError, "querySpecs needs to be int or str/unicode (check if you're using numpy.int for example in document_map";
        
        # issue query
        
        if len(querySpecs)==0:
            return {'success': False};
            
        elif len(querySpecs)==1:
            
            xl, yl, xu, yu, roi= rois[0];
            if type(querySpecs[0])==int:
                rankedList= self.API_obj[dsetname].internalQuery( docID= querySpecs[0], xl= xl, xu= xu, yl= yl, yu= yu, startFrom= 0, numberToReturn= 1000 );
            else:
                rankedList= self.API_obj[dsetname].externalQuery( querySpecs[0], xl= xl, xu= xu, yl= yl, yu= yu, startFrom= 0, numberToReturn= 1000 );
            
        else:
            rankedList= self.API_obj[dsetname].multiQuery( querySpecs, rois= rois, startFrom= 0, numberToReturn= 1000 );
        
        # return results
        
        # ROI returns supported only for single query image
        if len(querySpecs)==1:
            queryROI= rois[0][4];
        else:
            queryROI= None;
        results= self._convertToVisorResults( rankedList, dsetname, queryROI= queryROI );
        
        self.resultLock.acquire();
        self.results[query_id]= results;
        self.resultLock.release();
        
        return {'success': True};
    
    
    
    def getRanking(self, query_id):
        
        self.resultLock.acquire();
        if query_id in self.results:
            ranklist= self.results[query_id];
        else:
            ranklist= None;
        self.resultLock.release();
        
        if ranklist==None:
            return {'success': False};
        return {'success': True, 'ranklist': ranklist};



if __name__=='__main__':
    
    # add my code and create the real backend object
    sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'web'));
    import webserver;
    datasetOpts= [];
    
    assert( len(sys.argv)>=6 and (len(sys.argv)-2) % 4 == 0 );
    visorBackendPort= int(sys.argv[1]);
    datasetOpts= [];
    for i in range(2, len(sys.argv), 4):
        datasetOpts.append( { \
            'dsetname': sys.argv[i],
            'visorDsetname': sys.argv[i+1] if sys.argv[i+1]!='.' else sys.argv[i],
            'APIport': int(sys.argv[i+2]),
            'enableUpload': sys.argv[i+3]=='u'} );
    
    API_obj= webserver.get( datasetOpts, onlyAPI= True );
    
    visorBackend_obj= visorBackend(API_obj, datasetOpts, visorBackendHost= "localhost", visorBackendPort= visorBackendPort);
