##
## VISE User Interface template
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 24 Jan. 2018
##

class template_15cbt:
    def __init__( self ):
        self.html_title_prefix = "15cbt";

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
  <meta name="description" content="User interface of VGG Image Search Engine (VISE)">
  <link rel="shortcut icon" type="image/x-icon" href="./static/favicon.ico"/>
  <link rel="stylesheet" type="text/css" href="./static/vise2.css" />
  <script src="./static/vise.js"></script>
  %s
</head>
<body>
<div class="page">
  <div style="display:block;margin-bottom:1em;background-color:#002147;"><a href="page0" title="Show home page"><img src="static/images/headers/15cbt_logo.png" alt="15cBOOKTRADE Logo"></a></div>
  <div class="pagerow pageheader">
    <div class="header_control_panel">
      <div class="nav_panel">
        <ul class="hlist">
          <li><a href="./page0">Home</a></li>
          <li><a href="./file_index">File Index</a></li>
          <li><a href="./istc">ISTC Index</a></li>
        </ul>
      </div>

      <div class="search_panel">
        <form action="text_search" method="GET" id="text_search">
          <input id="text_search_keyword" type="text" name="keyword" placeholder="Search keyword" size="10">
          <select id="text_search_target" name="target">
            <!--<option value="All" selected="true">All</option>-->
            <option value="Region Attributes" selected="true">Region Attributes</option>
            <option value="ISTC Metadata">ISTC Metadata</option>
            <option value="Image Filename">Image Filename</option>
          </select>
          <button type="submit" value="Search">Search</button>
        </form>
      </div>

    </div>
    <div style="clear: both">&nbsp;</div>
  </div>

  %s

  <div class="page_footer">This image search tool is based on <a href="http://www.robots.ox.ac.uk/~vgg/software/vise/">VGG Image Search Engine (VISE)</a> and has been developed at the <a href="http://www.robots.ox.ac.uk/~vgg/">Visual Geometry Group (VGG)</a>.</div>
</div>
</body>
</html>''' % ( self.html_title_prefix, title, headExtra, body );

        return res;
