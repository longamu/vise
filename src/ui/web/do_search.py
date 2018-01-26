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
import math;

from upload import savedTemp;

import uuid;
import json;


import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();
tmpDir=    os.path.join( scriptroot, 'tmp/' );


_username_key_= 'username';


class doSearch:
    def __init__(self, page_template, API_obj, docMap, pathManager_obj, upload_obj=None, examples=None, guiOpts=None, file_attributes=None ):

        self.pt = page_template;
        self.API_obj = API_obj;
        self.docMap = docMap;
        self.pathManager_obj = pathManager_obj;
        self.upload_obj = upload_obj;
        self.datasets = self.API_obj.keys();
        self.examples = examples;
        self.guiOpts = guiOpts;

        self.def_dsetname = self.docMap.keys()[0];
        self.file_attributes = file_attributes;

    def get_istc_metadata_html_table(self, filename):
        istc_metadata = self.file_attributes.get_file_metadata(filename=filename);
        html = 'ISTC metadata for ' + filename + ' not found!';
        if istc_metadata.shape[0] == 1:
            html = '<table class="metadata_table"><tbody>'
            for key in istc_metadata:
                value = istc_metadata.iloc[0][key]
                if key == 'id':
                    istc_url = '<a target="_blank" href="http://data.cerl.org/istc/%s">%s</a>' % (value, value);
                    html += '<tr><td>%s</td><td>%s</td></tr>' % (key, istc_url);
                    continue;
                if value != '':
                    html += '<tr><td>%s</td><td>%s</td></tr>' % (key, value);
                else:
                    html += '<tr><td>%s</td><td></td></tr>' % (key);
            html += '</tbody></table>';
        return html;

    def get_overlap_region_metadata_html_table(self, metadata, rank):
        region_metadata_html = 'No overlap with any manually annotated regions.';
        metadata_tokens = metadata.split("__SEP__");
        if len(metadata_tokens) != 1:
            metadata_index = 0;

            region_metadata_html = '''
<strong>Metadata of overlapping region (manually annotated region shown in blue)</strong>
  <input type="checkbox" class="show_more_state" id="result_%d_region_metadata" />
  <table class="metadata_table show_more_wrap"><tbody>''' % (rank);

            for metadata_i in metadata_tokens :
                keyval = metadata_i.split("__KEYVAL_SEP__");
                if len(keyval) == 2:
                    if metadata_index < 4:
                        region_metadata_html += '<tr><td>%s</td><td>%s</td></tr>' % (keyval[0], keyval[1]);
                    else:
                        region_metadata_html += '<tr class="show_more_target"><td>%s</td><td>%s</td></tr>' % (keyval[0], keyval[1]);
                else:
                    if len(keyval) == 1 and keyval[0] != '':
                        region_metadata_html += "<tr><td>%s</td><td></td></tr>" % (keyval[0]);

                metadata_index = metadata_index + 1;

            # end of for metadata_i
            region_metadata_html += '</tbody></table><label for="result_%d_region_metadata" class="show_more_trigger"></label>' % (rank);
        return region_metadata_html;

    @cherrypy.expose
    def index(self, docID= None, uploadID= None, xl= None, xu= None, yl= None, yu= None, startFrom= "1", numberToReturn= "20", tile= None, noText= None, dsetname= None):

        if dsetname==None: dsetname= self.def_dsetname;

        emailFeedback= False;
        showText = True;
        if tile==None:
            tile= 'tile' in self.guiOpts['defaultView'][dsetname];
        else:
            tile= (tile!='false');
            if noText != None:
                showText = False;

        if xl!=None: xl= float(xl);
        if xu!=None: xu= float(xu);
        if yl!=None: yl= float(yl);
        if yu!=None: yu= float(yu);
        # if query regions are tiny: make them big
        if xl!=None and xu!=None and abs(xu-xl)<5:
            xl= None; xu= None;
        if yl!=None and yu!=None and abs(yu-yl)<5:
            yl= None; yu= None;
        startFrom= int(startFrom)-1;
        numberToReturn= int(numberToReturn);

        if xl==None: xl= 0;
        if yl==None: yl= 0;


        if uploadID==None:
            docID= int(docID);
            if xu==None or xu==None: imw, imh= self.docMap[dsetname].getWidthHeight(docID);
            if xu==None: xu= imw;
            if yu==None: yu= imh;
            queryFn= self.pathManager_obj[dsetname].hidePath(docID);
            query_filename = self.pathManager_obj[dsetname].hidePath(docID).decode('utf-8');
            results= self.API_obj[dsetname].internalQuery( docID= docID, xl= xl, xu= xu, yl= yl, yu= yu, startFrom= startFrom, numberToReturn= numberToReturn+1 );
        else:
            st= savedTemp.load(uploadID);
            query_filename = st['originalFullFilename'].decode('utf-8');
            if xu==None or yu==None:
                if xu==None: xu= st['w'];
                if yu==None: yu= st['h'];
            results= self.API_obj[dsetname].externalQuery( st['compDataFilename'], xl= xl, xu= xu, yl= yl, yu= yu, startFrom= startFrom, numberToReturn= numberToReturn+1 );
            del st;

        #temp
        first= startFrom;
        last= startFrom + len(results)-1;


        if uploadID==None:
            query_spec0 = "docID=%d" % docID;
            query_spec1 = "docID1=%d" % docID;
        else:
            query_spec0 = "uploadID=%s" % uploadID;
            query_spec1 = "uploadID1=%s" % uploadID;


        body= "";

        # query image
        queryImage= '<a href="search?%s"><img width="200" src="getImage?%s&width=200&xl=%.2f&xu=%.2f&yl=%.2f&yu=%.2f&H=1,0,0,0,1,0,0,0,1&%s">' % (query_spec0, query_spec0, xl,xu,yl,yu, "crop" if tile else "");

        query_img_url = 'search?%s' % (query_spec0);
        query_img_src = 'getImage?%s&width=200&xl=%.2f&xu=%.2f&yl=%.2f&yu=%.2f&H=1,0,0,0,1,0,0,0,1%s' % (query_spec0, xl, xu, yl, yu, "&crop" if tile else "");
        query_istc_metadata = self.get_istc_metadata_html_table(query_filename);

        body +='''
<div id="query_image_panel" class="query_image_panel pagerow">
  <span class="title">Query Image Region</span>
  <div id="query_image_metadata">
    <ul>
      <li>Filename: <a href="file_attributes?%s">%s</a></li>
      <li>ISTC Metadata: %s</li>
    </ul>
  </div>
  <a href="file_attributes?%s"><img src="%s"></a>
</div>''' % (query_spec0, query_filename, query_istc_metadata, query_spec0, query_img_src);

        # navigation

        if len(results) > numberToReturn:
            isLastPage= False;
            results=results[:numberToReturn];
        else:
            isLastPage= True;

        tileStr= "tile=true" if tile else "tile=false";
        showTextStr= "" if showText else "noText";
        navigation= "";
        noNavigation= True;
        if startFrom>0:
            noNavigation= False;
            if 1<startFrom+1-numberToReturn:
                navigation+= '<li><a href= "dosearch?%s&xl=%.2f&xu=%.2f&yl=%.2f&yu=%.2f&numberToReturn=%d&startFrom=1&%s&%s">First</a></li>' % (query_spec0,xl,xu,yl,yu,numberToReturn,tileStr,showTextStr);
                navigation+= "&nbsp;&nbsp;";
            navigation+= '<li><a href="dosearch?%s&xl=%.2f&xu=%.2f&yl=%.2f&yu=%.2f&numberToReturn=%d&startFrom=%d&%s&%s">Prev</a></li>' % (query_spec0,xl,xu,yl,yu,numberToReturn,max(1,startFrom+1-numberToReturn),tileStr,showTextStr);
        if startFrom>0 or not(isLastPage):
            noNavigation= False;
        if not(isLastPage):
            noNavigation= False;
            navigation+= '<li><a href="dosearch?%s&xl=%.2f&xu=%.2f&yl=%.2f&yu=%.2f&numberToReturn=%d&startFrom=%d&%s&%s">Next</a></li>' % (query_spec0,xl,xu,yl,yu,numberToReturn,startFrom+1+numberToReturn,tileStr,showTextStr);
        if noNavigation:
            navigation= "";

        # switch between tiled and list views
        search_result_page_tools = '';
        listview_url = 'dosearch?%s&xl=%.2f&xu=%.2f&yl=%.2f&yu=%.2f&numberToReturn=%d&startFrom=%d' % (query_spec0, xl, xu, yl, yu, numberToReturn, startFrom+1);
        tileview_url = listview_url + '&tile=true';
        tileview_notext_url = tileview_url + '&noText';

        if tile :
            if showText:
                search_result_page_tools = '<li><a href="%s">List View</a></li><li>Tile View</li><li><a href="%s">Tile View (images only)</a></li>' % (listview_url, tileview_notext_url);
            else:
                search_result_page_tools = '<li><a href="%s">List View</a></li><li><a href="%s">Tile View</a></li><li>Tile View (images only)</li>' % (listview_url, tileview_url);
        else:
            search_result_page_tools = '<li>List View</li><li><a href="%s">Tile View</a></li><li><a href="%s">Tile View (images only)</a></li>' % (tileview_url, tileview_notext_url);

        body += '''
<div id="search_result_panel" class="search_result_panel pagerow">
  <div id="search_result_count">Search Result: %d to %d</div>

  <div id="search_result_page_tools">
    <ul>%s</ul>
  </div>

  <div id="search_result_page_nav">
    <ul>%s</ul>
  </div>
</div>''' % (startFrom + 1,
             startFrom + min(numberToReturn, len(results)),
             search_result_page_tools,
             navigation)

        ## show list view (one result per row)
        print "Search query returned %d results" % (len(results));
        hidden_search_result_count = 0;
        for (rank, docIDres, score, metadata, metadata_region, H) in results:
            boxArg="xl=%.2f&xu=%.2f&yl=%.2f&yu=%.2f" % (xl,xu,yl,yu);
            match_compare_url = 'register?%s&docID2=%d&%s' % (query_spec1, docIDres, boxArg);
            if H!=None:
                boxArg+= "&H=%s" % H;
                match_details_url = "details?%s&docID2=%d&%s" % (query_spec1, docIDres, boxArg);
            else:
                match_details_url = "details?%s&docID2=%d&%s&drawPutative=true" % (query_spec1, docIDres, boxArg);

            match_filename = self.pathManager_obj[dsetname].hidePath(docIDres).decode('utf-8');

            is_result_hidden = False;
            if (score < 50.0):
                is_result_hidden = True;

            ## We add a footer mentioning the hidden search results
            if is_result_hidden:
                hidden_search_result_count = hidden_search_result_count + 1;
                if hidden_search_result_count == 1:
                    body += '<div class="hidden_search_result_msg_panel">We have removed search results with low matching scores because these matches may be incorrect. <button id="toggle_hidden_search_result_button" type="button" onclick="toggle_hidden_search_result()">Toggle hidden search results</button></div>';

            ## tiled view does not require metadata
            if not tile:
                overlap_metadata_html = self.get_overlap_region_metadata_html_table(metadata, rank);
                if metadata_region != "":
                    boxArg+= "&metadata_region=" + metadata_region;

                # each file has ISTC metadata
                istc_metadata_html = self.get_istc_metadata_html_table(match_filename);

                body+= '''
<div class="search_result_i pagerow %s">
  <div class="header">
    <span class="search_result_id" title="Rank of search result">%d</span>
    <span class="search_result_filename">Filename: <a href="file_attributes?docID=%d">%s</a></span>
    <span class="search_result_score" title="Matching score">%.1f</span>
  </div>

  <div class="img_panel">
    <a href="file_attributes?docID=%d"><img src="getImage?docID=%s&width=200&%s"></a>
  </div>
  <div class="istc_metadata"><strong>ISTC Metadata</strong>%s</div>
  <div class="region_metadata">%s</div>

  <div class="search_result_tools">
  <ul class="hlist">
    <li><a href="%s">Details of Match</a></li>
    <li><a href="%s">Compare Matching Regions</a></li>
    <li><a href="#">Use this image for further search</a></li>
  </ul>
  </div>

</div>''' % ("hidden_by_default display-none" if is_result_hidden else "",
             rank + 1,
             docIDres,
             match_filename,
             score,
             docIDres,
             docIDres,
             boxArg,
             istc_metadata_html,
             overlap_metadata_html,
             match_details_url,
             match_compare_url);

            else:
                if showText:
                    body+= '''
<div class="search_result_i pagecell %s">
  <div class="header">
    <span class="search_result_filename">Filename: <a href="file_attributes?docID=%d">%s</a></span>
  </div>

  <div class="img_panel" style="float: none; text-align: center;">
    <a href="file_attributes?docID=%d"><img src="getImage?docID=%s&height=200&%s&crop"></a>
  </div>

  <div class="search_result_tools">
    <span><a href="%s">Details of Match</a></span>
  </div>
</div>''' % ("hidden_by_default display-none" if is_result_hidden else "",
             docIDres,
             match_filename,
             docIDres,
             docIDres,
             boxArg,
             match_details_url);

                else:
                    body+= '''
<div class="search_result_i pagecell %s">
  <div class="img_panel">
    <a href="file_attributes?docID=%d"><img src="getImage?docID=%s&height=200&%s&crop"></a>
  </div>
</div>''' % ("hidden_by_default display-none" if is_result_hidden else "",
             docIDres,
             docIDres,
             boxArg);

        # end of for () in results:
        return self.pt.get(title= "Exact Matches", headExtra= "", body= body);
