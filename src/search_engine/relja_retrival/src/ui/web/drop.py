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

class drop:
    
    @cherrypy.expose
    def index(self):
        return """
<html>
<head>
<title>Upload</title>

<script language="javascript">
    var timeStep= 500;
    function checkDrop(){
        var val= document.getElementById('uploadURL').value;
        if (val){
            self.opener.document.getElementById("uploadURL").value= val;
            self.opener.document.getElementById("upload").submit();
            document.getElementById('uploadURL').value= "";
        }
        setTimeout("checkDrop()",timeStep);
    }
</script>
</head>
<body>

<form>
<textarea style="width:100%; height:100%" id="uploadURL" name="uploadURL">
</textarea>
</form>

<script language="javascript">
setTimeout("checkDrop()",timeStep);
</script>

</body>
</html>
""";