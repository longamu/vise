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

import os, sys, traceback;
import SocketServer, socket;

import json;
import threading;


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
                
                response= self.server.backend.addPosTrs( \
                    req['query_id'],
                    req['impath'],
                    featpath= req['featpath'],
                    extra_params= req['extra_params']);
                
            elif func == 'addNegTrs':
                
                if not('featpath' in req):
                    req['featpath']= None;
                if not('extra_params' in req):
                    req['extra_params']= {};
                
                response= self.server.backend.addNegTrs( \
                    req['query_id'],
                    req['impath'],
                    featpath= req['featpath'],
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
    
    
    
    def __init__(self, visorBackendHost= "localhost", visorBackendPort= 45200):
        
        self.queryID= 1;
        
        # set up server
        self.server= ThreadedTCPServer((visorBackendHost, visorBackendPort), ThreadedTCPRequestHandler);
        self.server.daemon_threads= True;
        self.server.backend= self;
        
        print "visorBackendPort=", visorBackendPort;
        print "Waiting for requests"
        self.server.serve_forever();
    
    
    
    def _convertToVisorResults(self, rankedList):
        return [ {'path': path, 'score': score } for (path, score) in rankedList ];
    
    
    
    def getQueryId(self, visorDsetname):
        queryID= self.queryID;
        self.queryID= self.queryID % 1234567890 + 1;
        return {'success': True, 'query_id': queryID};
    
    
    def selfTest(self):
        return {'success': True};
    
    def releaseQueryId(self, query_id):
        return {'success': True};
    
    def addPosTrs(self, query_id, impath, featpath= None, extra_params= {}):
        return {'success': True};
    
    def addNegTrs(self, query_id, impath, featpath= None, extra_params= {}):
        return {'success': True};
    
    def train(self, query_id):
        return {'success': True};
    
    def loadClassifier(self, query_id, filepath):
        return {'success': False, 'message': "Don't do this with instance search"};
    
    def saveClassifier(self, query_id, filepath):
        return {'success': False, 'message': "Don't do this with instance search"};
    
    def getAnnotations(self, filepath):
        return {"success":True, "annos": []};
    
    def saveAnnotations(self, query_id, filepath):
        return {"success":True};
    
    def rank(self, query_id):
        return {'success': True};
    
    
    
    def getRanking(self, query_id):
        return {'success': True, 'ranklist': self._convertToVisorResults([('bla1.jpg', 0.9), ('bla2.jpg', 0.1)])};



if __name__=='__main__':
    visorBackend_obj= visorBackend(visorBackendHost= "localhost", visorBackendPort= 45200);
