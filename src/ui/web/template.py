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
        self.homeText= homeText if not(homeText==None) else "Oxford Image<br> Search";
        self.headerImage= ("style=\"background: url('%s') no-repeat;\"" % headerImage) if not(headerImage==None or len(headerImage)<3) else "";
        self.topLink= topLink if not(topLink==None) else "page0";
        self.bottomText= bottomText if not(bottomText==None) else "";
        self.haveLogout= haveLogout;
        self.enableUpload= enableUpload;
        
        self.ballads= ballads;
    
    
    def __str__(self):
        return self.get();

    def get( self, title= "", headExtra= "", body= "", outOfContainer= False ):
        
        res= """

<html><head>
    <title>%s%s</title>
    
    <link rel="stylesheet" type="text/css" href="static/style.130705.css">
    
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
        
        
        if not(outOfContainer):
            
            res+= """
<div id="page">


    <div id="header">
            """;
    
        res+= """
        <table width="100%%" cellpadding="0" cellspacing="10" border="0">
        
        <tr>
            <td colspan="%d" align="center">
            <a href="http://www.robots.ox.ac.uk/~vgg">
            <div id="VGG">
            Visual Geometry Group, University of Oxford
            </div>
            </a>
            </td>
        </tr>
        
        <tr>
        
            <td width="200" align="center">
                <a href="page0">
                <div id="navHome">
                <h3>%s</h3>
                </div>
                </a>
            </td>
        
            <td align="center">
                <a href="%s">
                <div id="navName" %s>
                <!--<h1>Ballad</h1>-->
                </div>
                </a>
            </td>
        """ % ( 2 if not(self.haveLogout) else 3, self.homeText, self.topLink, self.headerImage );
        
        if self.haveLogout:
            
            res+= """
            <td width="100" align="center">
                <a href="logout">
                <div id="navHome">
                <h3>Log out</h3>
                </dov>
                </a>
            </td>
            """;
        
        res+= "</tr>";
            
        
        
        if self.enableUpload:
            res+= """
        <tr>
            <td colspan="%d" align="center">
                <form action="upload?" method="post" name="upload" id="upload" enctype="multipart/form-data">
                    File: <input name="uploadFile" type="file" size="5">
                    or URL: <input name="uploadURL" id="uploadURL" type="text" size="10">
                    &nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;
                    <input value="Upload and Search" type="submit">
                    &nbsp;&nbsp;%s
                </form>
            </td>
        </tr>
            """ % ( 2 if not(self.haveLogout) else 3, \
            '<a href="javascript:startDrop();"><img src="static/images/popup.png"></a>' if not(self.ballads) else """
                Whole sheet <input type="checkbox" onchange="javascript:updateMaxSize();">
                <input type="hidden" name="maxSize" id="maxSize" value="1024">
            """
             );
        
        
        res+= "</table>";
        
        if not(outOfContainer):
            
            res+= """
    
    </div>
    
    <div id="content">
            """;
        
        res+= "%s<br>" % body;
        
        if not(outOfContainer):
            res+= "</div>";
        
        res+= "%s<br><br>" % self.bottomText;
        
        if not(outOfContainer):
            res+= "</div>";

        res+= "</body></html>";
        
        return res;
