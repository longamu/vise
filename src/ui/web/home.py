##
## User Interface template for 15cILLUSTRATION
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 3 Aug. 2018
##

import cherrypy;

import random;

class home:
  def __init__(self, pageTemplate, docMap):
    self.pT= pageTemplate;
    self.dset_image_count = len(docMap[ docMap.keys()[0] ]);
    self.body = '''
<div class="home_page pagerow">
  <div class="top_description pagerow">15cILLUSTRATION is a searchable database of 15th century printed illustrations developed by the <a href="http://15cbooktrade.ox.ac.uk/">15cBOOKTRADE</a> project in collaboration with the <a href="http://www.robots.ox.ac.uk/~vgg/">Visual Geometry Group</a>.</div>

  <div class="task pagerow">
    <a href="page0">
    <div class="task_i task1_bg_color">
      <div class="task_title">Search Using Database Images</div>
      <div class="task_description">This searchable database contains %d printed illustrations from the 15th century which can be search using one of the images in this database.</div>
    </div>
    </a>

    <a href="text_search">
    <div class="task_i task2_bg_color">
      <div class="task_title">Search Metadata</div>
      <div class="task_description">The 15th century printed illustrations present in this database are linked to ISTC metadata and manual annotations of these illustrations created by scholars.</div>
    </div>
    </a>

    <div class="task_i task3_bg_color">
      <div class="task_title">Upload and Search</div>
      <div class="task_description">The %d printed illustrations from 15th century can also be searched using a new image uploaded from your computer. Click "Browse" button in top right corner to upload an image and press "Upload and Search" button.</div>
    </div>

    <div style="clear:both"></div>
  </div>
</div>''' % (self.dset_image_count, self.dset_image_count)
  @cherrypy.expose
  def index(self):
    return self.pT.get( title="Home", body=self.body, headExtra='' );
      
