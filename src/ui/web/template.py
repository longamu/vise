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


class template:
    
    
    
    def __init__( self, titlePrefix= None, homeText= None, headerImage= None, topLink=None, bottomText= None, haveLogout= False, enableUpload= False, ballads= False ):
        
        self.titlePrefix= titlePrefix if not(titlePrefix==None) else "Image Search:";
        self.titlePrefix+= ' ';
        self.homeText= "";
        self.headerImage= ("style=\"background: url('%s') no-repeat;\"" % headerImage) if not(headerImage==None or len(headerImage)<3) else "";
        self.topLink= topLink if not(topLink==None) else "page0";
        self.bottomText= bottomText if not(bottomText==None) else '<div style="text-align: center;font-size:small;">This image search tool is based on <a href="http://www.robots.ox.ac.uk/~vgg/software/vise/">VGG Image Search Engine (VISE)</a> and has been developed at the <a href="http://www.robots.ox.ac.uk/~vgg/">Visual Geometry Group (VGG)</a>.</div>';
        self.haveLogout= haveLogout;
        self.enableUpload= enableUpload;
        
        self.ballads= ballads;
    
    
    def __str__(self):
        return self.get();

    def get( self, title= "", headExtra= "", body= "", outOfContainer= False ):
        
        res= """

<html><head>
    <title>%s%s</title>
    
    <link rel="stylesheet" type="text/css" href="static/vise.css">
    
    <script language="javascript">
        
        function startDrop(){
            var dropWin= window.open( "drop", "Upload", "width=100,height=100" );
        }
        
        var maxSizeVal= 300;
        function updateMaxSize(){
            if (maxSizeVal==1024){
                maxSizeVal= 300;
            } else {
                maxSizeVal= 1024;
            }
            document.getElementById("maxSize").value= maxSizeVal;
        }
    </script>
    
    %s
</head>
<body>
    """ % ( self.titlePrefix, title, headExtra );
        
        res+= '<div id="page"><div id="header">';
        res+= """<div style="display:block;margin-bottom:1em;"><a href="page0" title="Show home page"><img src="static/images/headers/15cbt_logo.png" alt="15cBOOKTRADE Logo"></a></div>
        <table width="100%%" cellpadding="0" cellspacing="0" border="0">""";
        
        if self.haveLogout:
            res+= """<tr>
            <td width="100" align="center">
                <a href="logout">
                <div id="navHome">
                <h3>Log out</h3>
                </dov>
                </a>
            </td></tr>
            """;
        
        if self.enableUpload:
            res+= """
        <tr>
            <td colspan="%d" align="center">
              <div id="file_upload_bar" style="display:table; width:96%%; padding:0rem; line-height:2rem">
              <div style="display: table-cell;text-align:left;">Upload and Search</div>
              <div style="display: table-cell;text-align:left;">
              <form action="upload" method="POST" name="upload" id="upload" enctype="multipart/form-data">
              <input name="uploadFile" type="file" size="5">
              <input name="uploadURL" id="uploadURL" type="text" size="10" value="or, enter url" onclick="this.value=\'\';">
              <input value="Upload and Search" type="submit">&nbsp;&nbsp;%s
              </form></div>
              </div>
            </td>
        </tr>
            """ % ( 2 if not(self.haveLogout) else 3, \
            '<a href="javascript:startDrop();"><img src="static/images/popup.png"></a>' if not(self.ballads) else """
                Whole sheet <input type="checkbox" onchange="javascript:updateMaxSize();">
                <input type="hidden" name="maxSize" id="maxSize" value="1024">
            """
             );
        
        
        res+= "</table>";
        res+= "</div> <!-- end of header -->"

        if outOfContainer:
          res+= '</div> <!-- end of page -->';
          res+= '<div id="wide_content" style="width: 100%%;">%s</div>' % (body);
          res+= '<div style="text-align:center; display: block; padding:3rem 0;">%s</div>' %(self.bottomText);
        else:
          res+= '<div id="content">%s</div>' % (body);
          res+= '<div style="text-align:center; display: block; padding:3rem 0;">%s</div>' %(self.bottomText);
          res+= '</div> <!-- end of page -->';

        res+= "</body></html>";
        
        return res;
