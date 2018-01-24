##
## VISE User Interface template
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 24 Jan. 2018
##

class template_search_result:
    def __init__( self ):
        self.html_title_prefix = "Search Result";

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
  <div id="page_header" class="pagerow">
    <h1>15cBOOKTRADE Project</h1>
  </div>

  %s

  <div class="page_footer">
    VGG Image Search Engine (<a href="http://www.robots.ox.ac.uk/~vgg/software/vise/">VISE</a>) is an open source project developed by the Visual Geometry Group (<a href="http://www.robots.ox.ac.uk/~vgg/">VGG</a>).
  </div>
</div>
</body>
</html>''' % ( self.html_title_prefix, title, headExtra, body );

        return res;
