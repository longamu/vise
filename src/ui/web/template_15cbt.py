##
## User Interface template for 15cILLUSTRATION
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 24 Jan. 2018
##

class template_15cbt:
    def __init__( self ):
        self.html_title_prefix = "15cILLUSTRATION";

    def __str__(self):
        return self.get();

    def get( self, title= "", headExtra= "", body= "", outOfContainer= False ):
        res= '''
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>%s : %s</title>
  <meta name="author" content="Abhishek Dutta">
  <meta name="description" content="15cILLUSTRATION : searchable database of 15th century printed illustrations.">
  <link rel="shortcut icon" type="image/x-icon" href="./static/favicon.ico"/>
  <link rel="stylesheet" type="text/css" href="./static/vise2.css" />
  <script>
    (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
    (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
    m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
    })(window,document,'script','https://www.google-analytics.com/analytics.js','ga');

    ga('create', 'UA-20555581-3', 'auto');
    ga('set', 'page', '/vise/15cillustration/vise_15cbt-1.0.0');
    ga('send', 'pageview');
  </script>
  <script src="./static/vise.js"></script>
  %s
</head>
<body>
<div class="page">
  <div class="top_header">
    <a href="home" title="Show home page"><img src="static/images/headers/15cILLUSTRATION_logo.png" alt="15cILLUSTRATION Logo"></a>
    <img style="float:right;" src="static/images/headers/oxford_logo.png" alt="University of Oxford Logo">
  </div>
  <div class="pagerow pageheader">
    <div class="header_control_panel">
      <div class="nav_panel" style="padding-top:0.2rem;">
        <ul class="hlist">
          <li><a href="home">Home</a></li>
          <li><a href="page0">Search Using Database Images</a></li>
          <li><a href="text_search">Search Metadata</a></li>
          <li><a href="file_index">Index of Illustrations</a></li>
          <li><a href="istc">Illustrations Grouped by ISTC</a></li>
          <li><a href="contribution">Contributing Libraries</a></li>
        </ul>
      </div>
      <div>
        <form action="upload" method="POST" name="upload" id="upload" enctype="multipart/form-data">
          <input style="width:14rem;" name="uploadFile" type="file" onchange="document.getElementById('button_upload_and_search').removeAttribute('disabled')">
<!--          <input name="uploadURL" id="uploadURL" type="text" value="or, enter url" onclick="this.value=\'\';">-->
          <input id="button_upload_and_search" disabled value="Upload and Search" type="submit">
        </form>
      </div>
    </div>
    <div style="clear: both">&nbsp;</div>
  </div>

  %s


  <div class="acknowledgement pagerow">
    <a href="http://15cbooktrade.ox.ac.uk/"><img height="60" src="static/images/headers/15cBOOKTRADE_logo.png" alt="15cBOOKTRADE project" title="15cBOOKTRADE project"></a>
    <a href="http://www.robots.ox.ac.uk/~vgg/projects/seebibyte/"><img height="60" src="static/images/headers/seebibyte_logo.png" alt="Seebibyte: Visual Search for the Era of Big Data (EP/M013774/1)" title="Seebibyte: Visual Search for the Era of Big Data (EP/M013774/1)"></a>
    <p style="font-size:0.8rem">Development and maintenance of 15cILLUSTRATION is supported by <a href="http://15cbooktrade.ox.ac.uk/">15cBOOKTRADE</a> and EPSRC programme grant Seebibyte: Visual Search for the Era of Big Data (EP/M013774/1)</p>
  </div>
</div>
</body>
</html>''' % ( self.html_title_prefix, title, headExtra, body );

        return res;
