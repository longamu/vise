# -*- coding: utf-8 -*-
##
## User Interface template for 15cILLUSTRATION
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 3 Aug. 2018
##

import cherrypy;

class contribution:
  def __init__(self, pageTemplate):
    self.pT= pageTemplate;
    self.body = '''
<div class="home_page pagerow">
<p>The 15cBOOKTRADE Project would like to thank the following libraries, who have digitized their collections and made them publicly available online, and whose images have been used to test and implement the database:</p>
<ul>
  <li>Berlin, Staatsbibliothek</li>
  <li>Bologna, Biblioteca Comunale dell’Archiginnasio (via BEIC)</li>
  <li>Firenze, Biblioteca Nazionale Centrale (via BEIC)</li>
  <li>Gent, Universiteitsbibliotheek Gent</li>
  <li>Kansas University Spencer Library (via BEIC)</li>
  <li>London, The British Library</li>
  <li>Madrid, Biblioteca Nacional de Españ;</li>
  <li>Milano, Biblioteca Trivulziana (via BEIC)</li>
  <li>Modena, Gallerie Estensi, Biblioteca Estense Universitaria (through BEIC);</li>
  <li>Munich, Bayerische Staatsbibliothek;</li>
  <li>Oxofrd, Bodleian Library;</li>
  <li>Paris, Bibliothèque nationale de France (through Gallica);</li>
  <li>Parma, Biblioteca Palatina (through BEIC);</li>
  <li>Philadelphia, University of Pennsylvania Library (through BEIC);</li>
  <li>Roma, Biblioteca Alessandrina (through BEIC);</li>
  <li>Roma, Biblioteca Nazionale Centrale Vittorio Emanuele (through BEIC);</li>
  <li>Roma, Biblioteca Casanatense (through BEIC);</li>
  <li>Roma, Biblioteca Corsiniana (through BEIC);</li>
  <li>Roma, Biblioteca Vallicelliana (through BEIC);</li>
  <li>Sevilla, Biblioteca de la Universidad de Sevilla;</li>
  <li>Venezia, Biblioteca della Fondazione Giorgio Cini;</li>
  <li>Venezia, Biblioteca Nazionale Marciana;</li>
  <li>Washington DC, Library of Congress;</li>
  <li>Wien,Österreichische Nationalbibliothek;</li>
  <li>Wolfenbüttel, Herzog August Bibliothek.</li>
</ul>

<p>The 15cBOOKTRADE Project would also like to thank the following libraries, who have allowed members of the team to take photographs of illustrations which were not already available through digitised copies: </p>
<ul>
  <li>Siena, Biblioteca Comunale degli Intronati</li>
  <li>London, The British Library</li>
  <li>Oxofrd, Bodleian Library</li>
  <li>Venezia, Biblioteca della Fondazione Giorgio Cini</li>
  <li>San Marino, CA, The Huntington Library</li>
  <li>Berlin, Kupferstichkabinett</li>
</ul>
<p>More information about the 15cIllustration database can be found at: <a href="http://15cbooktrade.ox.ac.uk/illustration/">http://15cbooktrade.ox.ac.uk/illustration/</a></p>
<p>For queries, or if any library or scholar would like to contribute images or metatadata, please do get in touch with the 15cBOOKTRADE Project team at 15cbooktrade@gmail.com. </p>
</div>'''.decode('utf8')
  @cherrypy.expose
  def index(self):
    return self.pT.get( title="Contributing Libraries", body=self.body, headExtra='' );
      
