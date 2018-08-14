##
## VISE User Interface template
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 24 Jan. 2018
##


import os, sys;
import ConfigParser;

import cherrypy;
import socket;

import api;

import template, page0, dynamic_image, search_page, do_search, details, home;
import template_15cbt;
import file_index;
import file_attributes_15cbt;
import istc;
import text_search;

import csv;

if True:
    import register_images;

import upload, web_api, drop;

if False:
    import send_comments;

import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();



class pathManager_ballad:

    def __init__(self, docMap):
        self.docMap= docMap;
        self.fnToShelf= {};
        f= open('/home/relja/Relja/Databases/Ballads/highres/shelfmarks.txt','r'); lines= f.readlines(); f.close();
        for line in lines:
            if len(line)>5:
                ln= line.strip();
                rightSpace= ln.rfind(' ');
                fn= ln[ (rightSpace+1): ];
                self.fnToShelf[ fn ]= ln[:rightSpace].strip();

    def getFullSize( self, path ):
        return path.replace(".jpg",".tif").replace("jpg/", "orig/");
    def hidePath( self, docID ):
        return self.hide( self.docMap.getFn(docID) );
    def hide( self, path ):
        return self.fnToShelf[ path[ (path.rfind('/')+1):-6 ] ]; # path/fn-M.jpg -> fn
        #return path[len("/home/relja/Relja/Databases/Ballads/jpg/"):].replace(".jpg",".tif");
    def displayPath( self, docID ):
        return self.display( self.docMap.getFn(docID) );
    def display( self, path ):
        #hidden= self.hide(path);
        #return hidden[0:hidden.find('/')]+':<br>'+hidden[(hidden.find('/')+1):];
        return self.hide(path);



class pathManager_hidder:
    def __init__(self, docMap, pathToHide):
        self.docMap= docMap;
        self.pathToHide= pathToHide;
    def getFullSize( self, path ):
        return path;
    def hidePath( self, docID ):
        return self.hide( self.docMap.getFn(docID) );
    def hide( self, path ):
        return path[len(self.pathToHide):];
    def unhide( self, path ):
        return os.path.join(self.pathToHide, path);
    def displayPath( self, docID ):
        return self.display( self.docMap.getFn(docID) );
    def display( self, path ):
        hidden= self.hide(path);
        return hidden;


class pathManager_open:
    def __init__(self, docMap):
        self.docMap= docMap;
    def getFullSize( self, path ):
        return path;
    def hidePath( self, docID ):
        return self.docMap.getFn(docID);
    def hide( self, path ):
        return path;
    def unhide( self, path ):
        return path;
    def displayPath( self, docID ):
        return self.display( self.docMap.getFn(docID) );
    def display( self, path ):
        return path;



class webserver2:

    def __init__(self, API_obj, serveraddress, serverroot, docMap, enableUpload, guiOpts, pathManager=None, examples=None, externalExamples=None, file_attributes_fn=None, file_attributes_filename_colname=None, istc_db_fn=None, istc_id_colname=None, region_attributes_fn=None, region_attributes_filename_colname=None, preprocess_fn=None, preprocess_filename_colname=None):

        def_dsetname= docMap.keys()[0];

        if pathManager==None:
            self.pathManager_obj= pathManager_open(docMap);
        else:
            self.pathManager_obj= pathManager;

        self.API_obj= API_obj;
        self.docMap= docMap;

        # page template for all pages
        self.template_15cbt = template_15cbt.template_15cbt();
        self.home_obj= home.home( self.template_15cbt, docMap);

        self.upload_obj= upload.upload(self.template_15cbt, API_obj);
        self.page0_obj= page0.page0( self.template_15cbt, self.docMap, self.pathManager_obj, examples= examples, externalExamples= externalExamples, browse= True );
        self.dynamicImage_obj= dynamic_image.dynamicImage( docMap );
        self.searchPage_obj= search_page.searchPage( self.template_15cbt, docMap, self.pathManager_obj );


        # file attributes page
        self.file_index_obj = file_index.file_index( self.template_15cbt, self.docMap, self.pathManager_obj );
        self.file_index = self.file_index_obj.index;
        #self.file_attributes_obj = file_attributes.file_attributes( self.pT, self.docMap, self.pathManager_obj, examples= examples, externalExamples= externalExamples, browse= True, file_attributes_fn=file_attributes_fn, file_attributes_filename_colname=file_attributes_filename_colname );
        self.file_attributes_obj = file_attributes_15cbt.file_attributes_15cbt( self.template_15cbt, self.docMap, self.pathManager_obj, file_attributes_fn, file_attributes_filename_colname, istc_db_fn, istc_id_colname, region_attributes_fn, region_attributes_filename_colname, preprocess_fn, preprocess_filename_colname );
        self.file_attributes= self.file_attributes_obj.index;

        self.doSearch_obj= do_search.doSearch( self.template_15cbt,
                                               API_obj,
                                               docMap,
                                               self.pathManager_obj,
                                               upload_obj=self.upload_obj,
                                               examples=examples,
                                               guiOpts=guiOpts,
                                               file_attributes=self.file_attributes_obj );

        # text search
        self.text_search_obj = text_search.text_search(self.template_15cbt,
                                                       file_attributes = self.file_attributes_obj);
        self.text_search = self.text_search_obj.index;

        # istc index
        self.istc_obj = istc.istc(self.template_15cbt,
                                  file_attributes = self.file_attributes_obj);
        self.istc = self.istc_obj.index;

        self.details_obj= details.details( self.template_15cbt, API_obj, docMap, self.pathManager_obj, doRegistration= guiOpts['registration'][def_dsetname] );
        self.registerImages_obj= register_images.registerImages( self.template_15cbt, API_obj );

        # definition of link between url and corresponding python script that handle those request
        self.index= self.home_obj.index;
        self.home= self.index;
        self.page0= self.page0_obj.index;
        self.getImage= self.dynamicImage_obj.index;
        self.search= self.searchPage_obj.index;
        self.dosearch= self.doSearch_obj.index;
        self.details= self.details_obj.index;
        self.drawMatches= self.details_obj.drawMatches;

        self.upload= self.upload_obj.upload;
        self.uploadNext= self.upload_obj.uploadNext;
        self.uploadSave= self.upload_obj.uploadSave;
        self.uploadProcess= self.upload_obj.uploadProcess;
        self.drop_obj= drop.drop();
        self.drop= self.drop_obj.index;

        self.webAPI_obj= web_api.webAPI( serveraddress, serverroot, self.upload_obj, self.API_obj, self.docMap, self.pathManager_obj );
        self.webAPI= self.webAPI_obj.uploadAPI;

        if True:
            self.register= self.registerImages_obj.index;
            self.tmpImage= self.registerImages_obj.tmpImage;

        self.getImageFull= self.searchPage_obj.getImageFull;

        if False:
            self.sendComments= self.sendComments_obj.index;


        # axes/ballads API stuff
        self.api_version= self.webAPI_obj.api_version;
        self.api_engine_reachable= self.webAPI_obj.api_engine_reachable;
        self.api_exec_query= self.webAPI_obj.api_exec_query;

def getOptional( f, defaultValue ):

    try:
        value= f();
    except ConfigParser.NoOptionError:
        value= defaultValue;
    return value;


def get(
    datasetOpts,
    serverroot= None,
    webserverPort= 8083,
    webserverHost= "0.0.0.0",
    keepalive= False,
    onlyAPI= False,
    ):

    if serverroot==None:
        serverroot= '/';
    else:
        if not( serverroot.endswith('/') ):
            serverroot+= '/';

    conf = {
            '/': {},
            '/static' : {
                'tools.staticdir.on': True,
                'tools.staticdir.dir': os.path.join(scriptroot,'static'),
                },
            '/robots.txt' : {
                'tools.staticfile.on': True,
                'tools.staticfile.filename': os.path.join(scriptroot,'static/robots.txt'),
                },
            };

    hostname= socket.gethostname();
    #print 'Hostname: ', hostname;
    if hostname=='claros3':
        hostname= 'http://claros3.oerc.ox.ac.uk';
    elif hostname in ['arthur','arthurlegacy']:
        hostname= 'http://arthur.robots.ox.ac.uk';
    elif hostname in ['thaisa','sands','zeus']:
        hostname= 'http://%s.robots.ox.ac.uk' % hostname;
    else:
        hostname= 'http://localhost';
    serveraddress= '%s:%d' % (hostname, webserverPort);

    for datasetOpt in datasetOpts:
        print 'Dataset: ', datasetOpt['dsetname'];
        print 'APIport: ', datasetOpt['APIport'];
        print 'enableUpload: ', datasetOpt['enableUpload'];

    #DEBUG_upload= True;
    DEBUG_upload= False;


    config= ConfigParser.ConfigParser();
    #config.read( os.path.join(scriptroot, 'config/config.cfg') );
    config.read( datasetOpts[0]['config_filename'] );

    # TODO: remove these two once VLAD also doesn't depend on engine_3
    featureWrapper= {};
    clstFn= {};

    docMap_obj= {};
    pathManager_obj= {};
    API_obj= {};
    examples= {};
    externalExamples= {};
    enableUpload= {};
    guiOpts= {};

    guiOptKeys= ['titlePrefix', 'homeText', 'headerImage', 'topLink', 'bottomText', 'defaultView', 'registration'];
    for key in guiOptKeys:
        guiOpts[key]= {};
    registrationDefault= True and True;


    for datasetOpt in datasetOpts:

        dsetname= datasetOpt['dsetname'];
        enableUpload[dsetname]= datasetOpt['enableUpload'];
        print "Loading:", dsetname;

        useVLAD= getOptional( lambda: config.get(dsetname, 'useVLAD'), False );

        # TODO: remove this once VLAD also doesn't depend on engine_3
        # vocabulary

        if useVLAD:

            if datasetOpt['enableUpload']:

                clstFn[dsetname]= os.path.expanduser( config.get( dsetname if not(DEBUG_upload) else 'ballads', 'clstFn') );

                if DEBUG_upload or config.getboolean(dsetname, 'RootSIFT'):
                    import engine_3.processing.features as jpe3_feat;
                    featureWrapper[dsetname]= jpe3_feat.toHellinger;
                else:
                    featureWrapper[dsetname]= None;

                SIFTscale3= getOptional( lambda: config.getboolean(dsetname, 'SIFTscale3'), True );

            else:
                clstFn[dsetname]= None;
                featureWrapper[dsetname]= None;
                SIFTscale3= True;


        # Load all dataset files metadata
        file_attributes_fn = getOptional( lambda: config.get(dsetname, 'file_attributes_fn'), None );
        file_attributes_filename_colname = getOptional( lambda: config.get(dsetname, 'file_attributes_filename_colname'), 'filename' );
        istc_db_fn = getOptional( lambda: config.get(dsetname, 'istc_db_fn'), None );
        istc_id_colname = getOptional( lambda: config.get(dsetname, 'istc_id_colname'), 'id' );
        region_attributes_fn = getOptional( lambda: config.get(dsetname, 'region_attributes_fn'), None );
        region_attributes_filename_colname = getOptional( lambda: config.get(dsetname, 'region_attributes_filename_colname'), None );
        preprocess_fn = getOptional( lambda: config.get(dsetname, 'preprocess_fn'), None );
        preprocess_filename_colname = getOptional( lambda: config.get(dsetname, 'preprocess_filename_colname'), None );
        print preprocess_fn
        print preprocess_filename_colname

        if ( file_attributes_fn != None ):
          print "Loading file attributes from: %s ..." %(file_attributes_fn);
        if ( istc_db_fn != None ):
          print "Loading istc database from: %s ..." %(istc_db_fn);
        if ( region_attributes_fn != None ):
          print "Loading region attributes from: %s ..." %(region_attributes_fn);

        # API construction
        apiScoreThr= getOptional( lambda: config.getfloat(dsetname, 'apiScoreThr'), None );
        print "apiScoreThr = %.f" % (apiScoreThr)
        APIhost = "localhost";
        if not(useVLAD):
            import api;
            API_obj[dsetname]= api.API( datasetOpt['APIport'], APIhost= APIhost, scoreThr= apiScoreThr);
        else:
            import single_api;
            API_obj[dsetname]= single_api.singleAPI( datasetOpt['APIport'], APIhost= APIhost, scoreThr= apiScoreThr, clstFn= clstFn[dsetname], featureWrapper= featureWrapper[dsetname], SIFTscale3= SIFTscale3 );

        docMap_obj[dsetname]= API_obj[dsetname].docMap;


        # path display

        if dsetname.startswith('VOID_ballads'):
            pathManager_obj[dsetname]= pathManager_ballad(docMap_obj[dsetname]);
        elif dsetname.startswith('VOID_SF_'):
            pathManager_obj[dsetname]= pathManager_SF(docMap_obj[dsetname]);
        else:
            pathManager_obj[dsetname]= getOptional( \
                lambda: pathManager_hidder(docMap_obj[dsetname], pathToHide= os.path.expanduser(config.get(dsetname, 'pathManHide')) ), \
                pathManager_open(docMap_obj[dsetname]) );
        API_obj[dsetname].pathManager_obj= pathManager_obj[dsetname];


        # examples

        examples[dsetname]= getOptional( lambda: [example for example in config.get(dsetname, 'examples').split('\n') if len(example)>0], None );

        if datasetOpt['enableUpload']:
            externalExamples[dsetname]= None;
        else:
            # TODO: read this
            externalExamples[dsetname]= None;


        # GUI options

        guiOpts[dsetname]= {};
        for optName,defVal in zip( guiOptKeys, [dsetname, None, 'static/images/headers/oxford_sky.jpg' , None, None, '', registrationDefault]):
            guiOpts[optName][dsetname]= getOptional( lambda: config.get(dsetname, optName), defVal );

        if guiOpts['defaultView'][dsetname]==None:
            guiOpts['defaultView'][dsetname]= [];
        else:
            guiOpts['defaultView'][dsetname]= guiOpts['defaultView'][dsetname].strip().split();


    if onlyAPI:
        return API_obj;

    webserver_obj = webserver2( API_obj,
                                serveraddress,
                                serverroot,
                                docMap_obj,
                                enableUpload,
                                guiOpts,
                                pathManager = pathManager_obj,
                                examples = examples,
                                externalExamples = externalExamples,
                                file_attributes_fn = file_attributes_fn,
                                file_attributes_filename_colname = file_attributes_filename_colname,
                                istc_db_fn = istc_db_fn,
                                istc_id_colname = istc_id_colname,
                                region_attributes_fn=region_attributes_fn,
                                region_attributes_filename_colname=region_attributes_filename_colname,
                                preprocess_fn=preprocess_fn,
                                preprocess_filename_colname=preprocess_filename_colname);

    return webserver_obj, conf;




def start(
    datasetOpts,
    serverroot= None,
    webserverPort= 8083,
    webserverHost= "0.0.0.0",
    keepalive= False,
    app_namespace= '/'
    ):


    if serverroot==None:
        serverroot= '/';
    else:
        if not( serverroot.endswith('/') ):
            serverroot+= '/';

    webserver_obj, conf= get( datasetOpts, serverroot= serverroot, webserverPort= webserverPort, webserverHost= webserverHost );

    cherrypy.config.update({
      'server.socket_port' : webserverPort,\
      'server.socket_host' : webserverHost,\
      'server.keepalive' : keepalive,\
      });

    cherrypy.config.update({'tools.encode.on': True, 'tools.encode.encoding': 'utf-8', 'tools.decode.on': True});

    access_log = cherrypy.log.access_log
    for handler in tuple(access_log.handlers):
        access_log.removeHandler(handler)

    error_log = cherrypy.log.error_log
    for handler in tuple(error_log.handlers):
        error_log.removeHandler(handler)


    print 'Application namespace: ', app_namespace;
    print '\nServer listening for requests at %s:%d ...' % (webserverHost, webserverPort);

    if serverroot=='/':
        cherrypy.quickstart( webserver_obj, app_namespace, config= conf );
    else:
        cherrypy.tree.mount( webserver_obj, serverroot, conf);

if __name__=='__main__':

    webserverPort= int(sys.argv[1]);

    datasetOpts= [];
    datasetOpts.append( {'dsetname': sys.argv[2],'APIport': int(sys.argv[3]), 'config_filename': sys.argv[4], 'enableUpload': sys.argv[5]} );
    start( datasetOpts, webserverPort= webserverPort, app_namespace=sys.argv[6] );

'''
    if len(sys.argv)>5:

        assert( (len(sys.argv)-2) % 3 == 0 );
        datasetOpts= [];
        for i in range(2, len(sys.argv), 3):
            datasetOpts.append( {'dsetname': sys.argv[i], 'APIport': int(sys.argv[i+1]), 'enableUpload': sys.argv[i+2]=='u'} );

    else:

        assert( len(sys.argv)>2 );
        datasetOpts=[ {'dsetname': sys.argv[2]} ];
        if len(sys.argv)>3:
            datasetOpts[0]['APIport']= int(sys.argv[3]);
        else:
            datasetOpts[0]['APIport']= 35200;
        if len(sys.argv)>4:
            datasetOpts[0]['enableUpload']= (sys.argv[4]=='u');
        else:
            datasetOpts[0]['enableUpload']= False;

    start( datasetOpts, webserverPort= webserverPort );
'''
