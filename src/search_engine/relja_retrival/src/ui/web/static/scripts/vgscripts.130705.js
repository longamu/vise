function Browser() {

  var ua, s, i;

  this.isIE    = false;
  this.isNS    = false;
  this.version = null;

  ua = navigator.userAgent;

  s = "MSIE";
  if ((i = ua.indexOf(s)) >= 0) {
    this.isIE = true;
    this.version = parseFloat(ua.substr(i + s.length));
    return;
  }

  s = "Netscape6/";
  if ((i = ua.indexOf(s)) >= 0) {
    this.isNS = true;
    this.version = parseFloat(ua.substr(i + s.length));
    return;
  }

  // Treat any other "Gecko" browser as NS 6.1.

  s = "Gecko";
  if ((i = ua.indexOf(s)) >= 0) {
    this.isNS = true;
    this.version = 6.1;
    return;
  }
}

var browser = new Browser();

// Returns the object with this id.
function el(id) {
  if (document.getElementById) {
    return document.getElementById(id);
  } else if (window[id]) {
    return window[id];
  }
  return null;
}

// Returns this objects y coord relative to the top-left of the html 'canvas'
function gpt(obj) { // Return the y-location of object o.
  var top = 0;
  var o = obj;
  while (o.offsetParent) {
    top += o.offsetTop;
    o = o.offsetParent;
  }
  return top;
}

// Returns this objects x coord relative to the ...
function gpl(obj) {
  var left = 0;
  var o = obj;
  while (o.offsetParent) {
    left += o.offsetLeft;
    o = o.offsetParent;
  }
  return left;
}

// Toggles the visibility of id - perfect for popups.
function toggle(id) {
  var o = el(id);
  if (o.style.visibility=='hidden') {
    o.style.visibility = 'visible';
  }
  else {
    o.style.visibility = 'hidden';
  }
}

// Draws a horizontal line relative to id1 inside the html of id2.
// If id1 is "null", then the lines are drawn relative to (0,0)
// (useful for some special cases).
function drawHLine(id1, id2, y, x1, x2, col, opt) {
  var t = 0;
  var l = 0;
  
  var o = el(id2);
  o.innerHTML +=
    '<div style="position: absolute; margin: 0px; overflow: hidden; height: 1px; ' +
    'background-color: ' + col + '; ' +
    'width: ' + String(x2 - x1) + 'px; ' +
    'left: ' + String(x1) + 'px; ' +
    'top: ' + String(y) + 'px;"' + (opt ? (' '+opt) : '') + '></div>';
}

// Draws a vertical line (with id id) relative to id1 inside the html of id2.
function drawVLine(id1, id2, x, y1, y2, col, opt) {
  var t = 0;
  var l = 0;
 
  var o = el(id2);
  o.innerHTML +=
    '<div style="position: absolute; clear: both; margin: 0px; overflow: hidden; width: 1px; ' +
    'background-color: ' + col + '; ' +
    'height: ' + String(y2 - y1) + 'px; ' +
    'left: ' + String(x) + 'px; ' +
    'top: ' + String(y1) + 'px;"' + (opt ? (' '+opt) : '') + 
    '></div>';
}

// Draws a rectangle from (x1,y1)->(x2, y2). Optional event strings can be
// places in opt, if required.
function drawBox(id1, id2, x1, y1, x2, y2, col, opt) {
  if (opt) {
    drawHLine(id1, id2, y1, x1, x2, col, opt);
    drawHLine(id1, id2, y2-1, x1, x2, col, opt);
    drawVLine(id1, id2, x1, y1, y2, col, opt);
    drawVLine(id1, id2, x2-1, y1, y2, col, opt);
  }
  else {
    drawHLine(id1, id2, y1, x1, x2, col);
    drawHLine(id1, id2, y2-1, x1, x2, col);
    drawVLine(id1, id2, x1, y1, y2, col);
    drawVLine(id1, id2, x2-1, y1, y2, col);
  } 
}

// Draws text with class at x, y.
function drawText(id1, id2, x, y, str) {
  var o = el(id1);
  var t = gpt(o);
  var l = gpl(o);
  
  var o = el(id2);
  o.innerHTML +=
    '<div ' + 
    'style="position: absolute; ' +
    'left: ' + String(x + l) + 'px; ' +
    'top: ' + String(y + t) + 'px;">' + str + '</div>';
}

// Timeline stuff...

function LZ(x) { // Formats a number to take up 2 digits (eg 1 -> 01)
    return (x < 0 || x >= 10 ? "" : "0") + x;
}

function tlFrmFrmt(fnum) {
  var secs = fnum / FFPS;
  T1 = LZ(Math.floor(secs / 3600)) + ":" +
       LZ(Math.floor((secs / 60) % 60)) + ":" +
       LZ(Math.floor(secs % 60));
  return T1;
}

function fstr(str, num) {
  zeroToAdd = num - str.length;
  var i=0;
  ret = '';
  for (i=0; i<zeroToAdd; i++) {
    ret += '0';
  }
  ret += str;
  return ret;
}

function tlGetThumbLoc(frm) {
  return BASE
       + '/'
       + fstr(String(Math.floor(frm/10000)), 2)
       + '/'
       + fstr(String(frm), 6)
       + '.jpg';
}

var tlim1num = 0;
var tlim2num = 0;
var tlim3num = 0;
SNIMGARR = 0

function tlInit() {
  var id = "tlcont";
  if (!FFRM) alert("FFRM not defined!");
  if (!FFPS) alert("FFPS not defined!");
  if (!TWID) alert("TWID not defined!");
  if (!THGT) alert("THGT not defined!");
  if (!TBCL) alert("TBCL not defined!");
  if (!TGRD) alert("TGRD not defined!");
  if (!TGR2) alert("TGR2 not defined!");
  if (!TGHT) alert("TGHT not defined!");
  if (!TGH2) alert("TGH2 not defined!");

  drawVLine("tl", "tl", 0, 0, THGT, TBCL); // Left border.
  drawVLine("tl", "tl", TWID, 0, THGT, TBCL); // Right border.
  drawHLine("tl", "tl", THGT/2, 0, TWID, TBCL); // Center line.
  // Gradations
  var frameStep = TGRD * FFPS;
  var i = frameStep;
  while (i<FFRM) {
    perc = i / FFRM;
    if ((Math.floor(i) % TGR2) != 0) {
      // Minor grad.
      drawVLine("tl", "tl", perc*TWID, THGT/2 - TGHT/2, THGT/2 + TGHT/2 + 1, TBCL);
    }
    else {
      // Major grad.
      drawVLine("tl", "tl", perc*TWID, THGT/2 - TGH2/2, THGT/2 + TGH2/2 + 1, TBCL);
      // Major grad text.
      drawText("tl", "tl", perc*TWID - 10, THGT/2 - TGH2/2 - 20, 
               String((i/(FFPS*60)).toFixed(0))+'m');
    }
    i += frameStep;
  }

  if (SNIMGARR) {
    tlim1num = SNIMGARR[0][0];
    tlim2num = SNIMGARR[0][1];
    tlim3num = SNIMGARR[0][2];
    setInterval(refreshImages, 70);
  }
}



// This initializes the second kind of timeline - the result timeline.
// Calls the normal initializer, then draws results bars on the tl.
function tlInit2() {
  tlInit();
  // Now draw the result bars.
   
  var i = 0;
  // If the image is not external, draw a black bar for the original image.
  if (!EXTIMG) {
    perc = RESARR[i][2] / FFRM;
    h = RESARR[i][0];
    drawVLine("tl", "tl", perc*TWID, THGT/2 - h/2, THGT/2 + h/2 + 1, ORIGCOL, 'id="res0"');
    i = 1;
  }

  // Now draw the red result bars.
  for (; i<RESARR.length; i++) {
    perc = RESARR[i][2] / FFRM;
    h = Math.floor(RESARR[i][0]/2);
    drawVLine("tl", "tl", perc*TWID, THGT/2 - h/2, THGT/2 + h/2 + 1, RESCOL, 'id="res' + String(i) + '"');
  }
}

var lastframe = 0;
function tlDrawSearch() {
  mov = document.movie1;
  movieTime = mov.GetTime();
  frameMT = tlInvStreamConvert(movieTime, mov.GetTimeScale());
  frameMT = intb(frameMT);
  el("tlsearchlink").innerHTML = '<a href="' + SIL + '?movie+name=' + MOVN + '&nframe+nb=' + String(frameMT) + '">Search</a>';
  lastframe = frameMT;
}

function tlClearSearch() {
  el("tlsearchlink").innerHTML = "&nbsp;";
}

function tlRelWatcher() {
  mov = document.movie1;
  tlDrawSearch();
  
  sf = RESARR[curshot][8];
  ef = RESARR[curshot][9];

  movieTime = mov.GetTime();
  efmt = tlStreamConvert(ef, mov.GetTimeScale());

  frameMT = tlInvStreamConvert(movieTime, mov.GetTimeScale());

  el("tltimeinfo").innerHTML = tlFrmFrmt(frameMT);

  x = frameMT / FFRM * TWID;

  el("tllocator").style.left = x + gpl(el("tl"));
  
  if (movieTime>efmt) {
    if (curshot<9)
      curshot = curshot + 1;
    else
      curshot = 0;
    tlStreamGoto(curshot);
  }
}

function tlMinShot(framestart) {
  min = 1000000;
  argmin = -1;
  for (i=0; i<10; i++) {
    if (i!=curshot && RESARR[i][2]>framestart) {
      if (RESARR[i][2]<min) {
        min = RESARR[i][2];
        argmin = i;
      }
    }
  }
  return argmin;
}
  
function tlTimWatcher() {
  tlDrawSearch();
  mov = document.movie1;
  sf = RESARR[curshot][8];
  ef = RESARR[curshot][9];

  movieTime = mov.GetTime();
  efmt = tlStreamConvert(ef, mov.GetTimeScale());

  frameMT = tlInvStreamConvert(movieTime, mov.GetTimeScale());

  el("tltimeinfo").innerHTML = tlFrmFrmt(frameMT);

  x = frameMT / FFRM * TWID;

  el("tllocator").style.left = x + gpl(el("tl"));
  
  if (movieTime>efmt) {
    // Find next in time shot.
    curshot = tlMinShot(frameMT);
    if (curshot>=10 || curshot==-1) {
      curshot = tlMinShot(0);
    }
    tlStreamGoto(curshot);
  }
}

function tlConWatcher() {
  tlDrawSearch();
  mov = document.movie1;
  sf = RESARR[curshot][8];
  ef = RESARR[curshot][9];

  movieTime = mov.GetTime();

  frameMT = tlInvStreamConvert(movieTime, mov.GetTimeScale());

  el("tltimeinfo").innerHTML = tlFrmFrmt(frameMT);

  x = frameMT / FFRM * TWID;

  el("tllocator").style.left = x + gpl(el("tl"));
}

var curHandler = 0;
function tlStreamSwitchMode(mode) {
  clearInterval(curHandler);
  if (mode==0) {
    el("tlmodeinfo").innerHTML = 'Relevance mode';
    tlStreamGoto(0);
    curshot = 0;
    curHandler = setInterval('tlRelWatcher()', 100);
  }
  else if (mode==1) {
    el("tlmodeinfo").innerHTML = 'Time mode';
    curshot = tlMinShot(0);
    tlStreamGoto(curshot); 
    curHandler = setInterval('tlTimWatcher()', 100);
  }
  else {
    el("tlmodeinfo").innerHTML = 'Continuous mode';
    curHandler = setInterval('tlConWatcher()', 100);
  }
}

function tlInit3() {
  tlInit2();
  drawVLine("tl", "tlcont", 0, 0, THGT, '#0f0', 'id="tllocator"');
  setTimeout('tlStreamGoto(0)', 1500);
  setTimeout('curHandler = setInterval("tlRelWatcher()", 100)', 1550);
//  curHandler = setInterval('tlRelWatcher()', 100);
  el("tlmodeinfo").innerHTML = 'Relevance mode';
}

function searchDrawImageRects() {
  // Draws rectangle matches on all the images img0-imgn with container img0cont-imgncont
  var i = 0;
  for (; i<IMGRECARR.length; i++) {
    drawBox("null", "img"+String(i)+"cont", IMGRECARR[i][0], IMGRECARR[i][1], IMGRECARR[i][2], IMGRECARR[i][3], '#fff');
  }
}

function inta(i) {
  return Math.floor(i);
}

function intb(i) {
  return parseInt(i.toFixed(0));
}

SRV = 0;
function facesDrawImageRects(id) {
  // Draws face rectangles.
  if (el(id) && !el(id).complete) {
    setTimeout('facesDrawImageRects("'+id+'")', 100);
    return;
  }
  var i = 0;
  for (; i<IMGFACERECARR.length; i++) {
    if (IMGFACERECARR[i]!=0) {
      var cur = IMGFACERECARR[i];
      var j = 0;
      for (; j<cur.length; j++) {
        var xoff = 0;
        if (!browser.isIE && SRV) {
          xoff = 0; //Set to -11 if there are problems
        }
        if (id=="norm") {
          drawBox("img"+String(i), "imgs", intb(cur[j][0]*XRATIO) + xoff, intb(cur[j][1]*YRATIO), intb(cur[j][2]*XRATIO) + xoff, intb(cur[j][3]*YRATIO), '#ff0');
        } else {
          drawBox(id, "img"+String(i)+"cont", intb(cur[j][0]*XRATIO) + xoff, intb(cur[j][1]*YRATIO), intb(cur[j][2]*XRATIO) + xoff, intb(cur[j][3]*YRATIO), '#ff0');
        }
      }
    }
  }
}

function refreshImages() {
  var im1 = el("tlim1");
  var im2 = el("tlim2");
  var im3 = el("tlim3");
  
  im1.src = tlGetThumbLoc(tlim1num);
  im2.src = tlGetThumbLoc(tlim2num);
  im3.src = tlGetThumbLoc(tlim3num);

  im1.style.visibility = 'visible';
  im2.style.visibility = 'visible';
  im3.style.visibility = 'visible';
}

function tlMouseMove(evt) {
  if (!evt) {
    evt = event; // Workaround for some(?) browsers.
  }

  var o = el("tlcont");
  var l = gpl(o);
  
  if (browser.isIE) {
    var x = evt.clientX - l - 2;
  } else {
    var x = evt.clientX - l;
  }
  
  var fnum = x/TWID * FFRM;

  var o = el("tlinfo");
  o.style.left = evt.clientX;
  o.style.visibility = 'visible';
  
  o.innerHTML =
    'Time: ' + tlFrmFrmt(fnum) + '<br/>' + 'Shot: ' + String(PXSNARR[x]);
  
  tlim1num = SNIMGARR[x][0];
  tlim2num = SNIMGARR[x][1];
  tlim3num = SNIMGARR[x][2];
}

var curSel = 0

function tlMouseMove2(evt) {
  if (!evt) {
    evt = event;
  }

  var o = el("tlcont");
  var l = gpl(o);
  
  if (browser.isIE) {
    var x = evt.clientX - l - 2;
  } else {
    var x = evt.clientX - l;
  }

  var fnum = x/TWID * FFRM;
  
  // Find closest result bar to here...
  var argmin = 0;
  var min = 100000;
  var i=0;
  for (; i<RESARR.length; i++) {
    if (Math.abs(RESARR[i][2] - fnum)<min) {
      min = Math.abs(RESARR[i][2] - fnum);
      argmin = i;
    }
  }

  var rn = argmin;
  curSel = rn;

  var o = el("tlinfo");
  o.style.left = evt.clientX; // RESARR[rn][2]*TWID / FFRM;
  o.style.visibility = "visible";
  o.innerHTML = 'Time: ' + tlFrmFrmt(RESARR[rn][2]) +
                '<br/>Shot: ' + String(RESARR[rn][1]);

  el("tlim1").src = RESARR[rn][3];
  el("tlim2").src = RESARR[rn][4];
  el("tlim3").src = RESARR[rn][5];
  el("tlim1").style.visibility = "visible";
  el("tlim2").style.visibility = "visible";
  el("tlim3").style.visibility = "visible";
 
  for (i=0; i<RESARR.length; i++) {
    var resBar = el("res" + String(i));
    if (i==0&&!EXTIMG) {
      if (i==rn) {
        resBar.style.background = ORIGSELCOL;
      } else {
        resBar.style.background = ORIGCOL;
      }
    } else {
      if (i==rn) {
        resBar.style.background = SELCOL;
      } else {
        resBar.style.background = RESCOL;
      }
    }
  }

  el("tlim2cont").innerHTML = "";
  // Draw the matched box on the middle image.
  drawBox("tlim2", "tlim2cont", IMGRECARR[rn][0], IMGRECARR[rn][1], IMGRECARR[rn][2], IMGRECARR[rn][3], "#fff");
  return rn;
}

function tlGotoNearest() {
  window.location.href = RESARR[curSel][6];
}
  
function tlMouseMove3(evt) {
  if (!evt) {
    evt = event;
  }
  var rn = tlMouseMove2(evt);
  el("tlinfo").innerHTML += '<br/>Relevance: ' + RESARR[rn][7];
}

function tlMouseClick(evt) {
  alert("boo");

  if (!evt) {
    evt = event;
  }

  var o = el("tlcont");
  var l = gpl(o);
  
  if (browser.isIE) {
    var x = evt.clientX - l - 2;
  } else {
    var x = evt.clientX - l;
  }
  
  var fnum = x/TWID * FFRM;
  var snum = PXSNARR[x];

  var ros = fstr(String(Math.floor((snum-1) / 10)), 2);

  href = VGES + "?range+of+shots=" + ros + "&movie+name=" + MOVN;

  window.location.href = href;
}

function tlMouseClick2(evt) {
  window.location.href = RESARR[curSel][6];
}

var STRMOFF = 0;

function tlStreamConvert(fn, ts) {
  return (fn - STRMOFF - 0)*(ts/FFPS);
}

function tlInvStreamConvert(mt, ts) {
  return (mt * (FFPS/ts)) + STRMOFF;
}

function tlStreamGoto(shotnb) {
  mov = document.movie1;
  tm = tlStreamConvert(RESARR[shotnb][8], mov.GetTimeScale());
  mov.Stop();
  mov.SetTime(tm);
  mov.Play();
  
  el("tlim1").src = RESARR[shotnb][3];
  el("tlim2").src = RESARR[shotnb][4];
  el("tlim3").src = RESARR[shotnb][5];
  el("tlim1").style.visibility = "visible";
  el("tlim2").style.visibility = "visible";
  el("tlim3").style.visibility = "visible";
  
  el("tlim2cont").innerHTML = "";
  // Draw the matched box on the middle image.
  drawBox("tlim2", "tlim2cont", IMGRECARR[shotnb][0], IMGRECARR[shotnb][1], IMGRECARR[shotnb][2], IMGRECARR[shotnb][3], "#fff");

  el("tlstreaminfo").innerHTML = "Shot number: " + String(RESARR[shotnb][1]) + ", relevance = " + 
                                 String(RESARR[shotnb][7]);
}

function tlMouseClick3(evt) {
//  window.location.href = RESARR[curSel][6];
  curshot = curSel;
  tlStreamGoto(curshot);
}
// Animate image stuff.
function animGenAdd(fnum) {
  var dir = Math.floor(fnum/10000.0);
  var sdir = (dir<10) ? '0' + String(dir) : String(dir);
  
  var snum1 = String(fnum);
  var i = 0;
  var snum2 = '';
  for (i=0; i<6 - snum1.length; i++) {
    snum2 += '0';
  }
  snum2 += snum1;
  return THUMBS+sdir+'/'+snum2+'.jpg';
}

function animAdvanceImg() {
  if (!playing) {
    return;
  }
  if (curFrame<EFRAME) {
    var o = el("mainimg");
    if ((o.readyState && o.readyState=='loaded')||o.complete) {
      o.src = animGenAdd(curFrame);
      curFrame++;
    } 
  }
  else {
    curFrame = SFRAME;
  }
}

function animSetFirstImage() {
  var o = el("mainimg");
  o.src = animGenAdd(SFRAME);
}

function animClick(evt) {
  if (playing) playing = 0;
  else playing = 1;
}

// Image searching stuff
var selDragging = 0;
var selX1 = 0;
var selY1 = 0;
var selX2 = 0;
var selY2 = 0;
var selXMin = 0;
var selXMax = 0;
var selYMin = 0;
var selYMax = 0;
var selWidth = 0;
var selHeight = 0;
var URLBASE = '';

function selFindMins() {
  selXMin = Math.min(selX1, selX2);
  selXMax = Math.max(selX1, selX2);
  selYMin = Math.min(selY1, selY2);
  selYMax = Math.max(selY1, selY2);
}

// === Sick of making this stuff cross browser, use the cross-browser libraries ===//
function xDef()
{
  for(var i=0; i<arguments.length; ++i){if(typeof(arguments[i])=='undefined') return false;}
  return true;
}

function xNum()
{
  for(var i=0; i<arguments.length; ++i){if(isNaN(arguments[i]) || typeof(arguments[i])!='number') return false;}
  return true;
}

function xStr(s)
{
  for(var i=0; i<arguments.length; ++i){if(typeof(arguments[i])!='string') return false;}
  return true;
}

function xGetElementById(e)
{
  if(typeof(e)=='string') {
    if(document.getElementById) e=document.getElementById(e);
    else if(document.all) e=document.all[e];
    else e=null;
  }
  return e;
}

function xCamelize(cssPropStr)
{
  var i, c, a = cssPropStr.split('-');
  var s = a[0];
  for (i=1; i<a.length; ++i) {
    c = a[i].charAt(0);
    s += a[i].replace(c, c.toUpperCase());
  }
  return s;
}

function xGetComputedStyle(e, p, i)
{
  if(!(e=xGetElementById(e))) return null;
  var s, v = 'undefined', dv = document.defaultView;
  if(dv && dv.getComputedStyle){
    s = dv.getComputedStyle(e,'');
    if (s) v = s.getPropertyValue(p);
  }
  else if(e.currentStyle) {
    v = e.currentStyle[xCamelize(p)];
  }
  else return null;
  return i ? (parseInt(v) || 0) : v;
}

function xLeft(e, iX)
{
  if(!(e=xGetElementById(e))) return 0;
  var css=xDef(e.style);
  if (css && xStr(e.style.left)) {
    if(xNum(iX)) e.style.left=iX+'px';
    else {
      iX=parseInt(e.style.left);
      if(isNaN(iX)) iX=xGetComputedStyle(e,'left',1);
      if(isNaN(iX)) iX=0;
    }
  }
  else if(css && xDef(e.style.pixelLeft)) {
    if(xNum(iX)) e.style.pixelLeft=iX;
    else iX=e.style.pixelLeft;
  }
  return iX;
}

function xTop(e, iY)
{
  if(!(e=xGetElementById(e))) return 0;
  var css=xDef(e.style);
  if(css && xStr(e.style.top)) {
    if(xNum(iY)) e.style.top=iY+'px';
    else {
      iY=parseInt(e.style.top);
      if(isNaN(iY)) iY=xGetComputedStyle(e,'top',1);
      if(isNaN(iY)) iY=0;
    }
  }
  else if(css && xDef(e.style.pixelTop)) {
    if(xNum(iY)) e.style.pixelTop=iY;
    else iY=e.style.pixelTop;
  }
  return iY;
}

function xHeight(e,h)
{
  if(!(e=xGetElementById(e))) return 0;
  if (xNum(h)) {
    if (h<0) h = 0;
    else h=Math.round(h);
  }
  else h=-1;
  var css=xDef(e.style);
  if (e == document || e.tagName.toLowerCase() == 'html' || e.tagName.toLowerCase() == 'body') {
    h = xClientHeight();
  }
  else if(css && xDef(e.offsetHeight) && xStr(e.style.height)) {
    if(h>=0) {
      var pt=0,pb=0,bt=0,bb=0;
      if (document.compatMode=='CSS1Compat') {
        var gcs = xGetComputedStyle;
        pt=gcs(e,'padding-top',1);
        if (pt !== null) {
          pb=gcs(e,'padding-bottom',1);
          bt=gcs(e,'border-top-width',1);
          bb=gcs(e,'border-bottom-width',1);
        }
        // Should we try this as a last resort?
        // At this point getComputedStyle and currentStyle do not exist.
        else if(xDef(e.offsetHeight,e.style.height)){
          e.style.height=h+'px';
          pt=e.offsetHeight-h;
        }
      }
      h-=(pt+pb+bt+bb);
      if(isNaN(h)||h<0) return;
      else e.style.height=h+'px';
    }
    h=e.offsetHeight;
  }
  else if(css && xDef(e.style.pixelHeight)) {
    if(h>=0) e.style.pixelHeight=h;
    h=e.style.pixelHeight;
  }
  return h;
}


function xWidth(e,w)
{
  if(!(e=xGetElementById(e))) return 0;
  if (xNum(w)) {
    if (w<0) w = 0;
    else w=Math.round(w);
  }
  else w=-1;
  var css=xDef(e.style);
  if (e == document || e.tagName.toLowerCase() == 'html' || e.tagName.toLowerCase() == 'body') {
    w = xClientWidth();
  }
  else if(css && xDef(e.offsetWidth) && xStr(e.style.width)) {
    if(w>=0) {
      var pl=0,pr=0,bl=0,br=0;
      if (document.compatMode=='CSS1Compat') {
        var gcs = xGetComputedStyle;
        pl=gcs(e,'padding-left',1);
        if (pl !== null) {
          pr=gcs(e,'padding-right',1);
          bl=gcs(e,'border-left-width',1);
          br=gcs(e,'border-right-width',1);
        }
        // Should we try this as a last resort?
        // At this point getComputedStyle and currentStyle do not exist.
        else if(xDef(e.offsetWidth,e.style.width)){
          e.style.width=w+'px';
          pl=e.offsetWidth-w;
        }
      }
      w-=(pl+pr+bl+br);
      if(isNaN(w)||w<0) return;
      else e.style.width=w+'px';
    }
    w=e.offsetWidth;
  }
  else if(css && xDef(e.style.pixelWidth)) {
    if(w>=0) e.style.pixelWidth=w;
    w=e.style.pixelWidth;
  }
  return w;
}


function xPageX(e)
{
  var x = 0;
  e = xGetElementById(e);
  while (e) {
    if (xDef(e.offsetLeft)) x += e.offsetLeft;
    e = xDef(e.offsetParent) ? e.offsetParent : null;
  }
  return x;
}

function xPageY(e)
{
  var y = 0;
  e = xGetElementById(e);
  while (e) {
    if (xDef(e.offsetTop)) y += e.offsetTop;
    e = xDef(e.offsetParent) ? e.offsetParent : null;
  }
  return y;
}

function xScrollLeft(e, bWin)
{
  var offset=0;
  if (!xDef(e) || bWin || e == document || e.tagName.toLowerCase() == 'html' || e.tagName.toLowerCase() == 'body') {
    var w = window;
    if (bWin && e) w = e;
    if(w.document.documentElement && w.document.documentElement.scrollLeft) offset=w.document.documentElement.scrollLeft;
    else if(w.document.body && xDef(w.document.body.scrollLeft)) offset=w.document.body.scrollLeft;
  }
  else {
    e = xGetElementById(e);
    if (e && xNum(e.scrollLeft)) offset = e.scrollLeft;
  }
  return offset;
}

function xScrollTop(e, bWin)
{
  var offset=0;
  if (!xDef(e) || bWin || e == document || e.tagName.toLowerCase() == 'html' || e.tagName.toLowerCase() == 'body') {
    var w = window;
    if (bWin && e) w = e;
    if(w.document.documentElement && w.document.documentElement.scrollTop) offset=w.document.documentElement.scrollTop;
    else if(w.document.body && xDef(w.document.body.scrollTop)) offset=w.document.body.scrollTop;
  }
  else {
    e = xGetElementById(e);
    if (e && xNum(e.scrollTop)) offset = e.scrollTop;
  }
  return offset;
}

function xEvent(evt) // object prototype
{
  var e = evt || window.event;
  if (!e) return;
  this.type = e.type;
  this.target = e.target || e.srcElement;
  this.relatedTarget = e.relatedTarget;
  if (xDef(e.pageX)) { this.pageX = e.pageX; this.pageY = e.pageY; }
  else if (xDef(e.clientX)) { this.pageX = e.clientX + xScrollLeft(); this.pageY = e.clientY + xScrollTop(); }
  if (xDef(e.offsetX)) { this.offsetX = e.offsetX; this.offsetY = e.offsetY; }
  else if (xDef(e.layerX)) { this.offsetX = e.layerX; this.offsetY = e.layerY; }
  else { this.offsetX = this.pageX - xPageX(this.target); this.offsetY = this.pageY - xPageY(this.target); }
  this.keyCode = e.keyCode || e.which || 0;
  this.shiftKey = e.shiftKey; this.ctrlKey = e.ctrlKey; this.altKey = e.altKey;
  if (typeof e.type == 'string') {
    if (e.type.indexOf('click') != -1) {this.button = 0;}
    else if (e.type.indexOf('mouse') != -1) {
      this.button = e.button;
    }
  }
}


var selX = 0;
var selY = 0;
function selGetPos(id, evt) {
  var xevt = new xEvent(evt);

  if (browser.isIE) {
    selX = xevt.offsetX;
    selY = xevt.offsetY;
  }
  else {
    var l = xPageX(id);
    var t = xPageY(id);
    selX = xevt.pageX - l;
    selY = xevt.pageY - t;
  }
}

function selUpdateBox() {
  //var l = xLeft("img0");
  //var t = xTop("img0");
  var l = xPageX("img0");
  var t = xPageY("img0");
 
  //var l2 = 0;
  //var t2 = 0;
  var l2 = xPageX("img0cont");
  var t2 = xPageY("img0cont");
  //el("dbg").innerHTML = "l=" + l + " t=" + t + "   l2=" + l2 + " t2=" + t2 + "  selX=" + selX + " selY=" + selY;

  if (browser.isIE) {
    xLeft("selTop", selXMin);
    xTop("selTop", selYMin);
    xWidth("selTop", (selXMax - selXMin));
    
    xLeft("selBottom", selXMin);
    xTop("selBottom", selYMax);
    xWidth("selBottom", selXMax - selXMin);
    
    xLeft("selLeft", selXMin);
    xTop("selLeft", selYMin);
    xHeight("selLeft", Math.max(selYMax - selYMin, 1));

    xLeft("selRight", selXMax);
    xTop("selRight", selYMin);
    xHeight("selRight", Math.max(selYMax - selYMin, 1)+1);
  }
  else {
    xLeft("selTop", selXMin + l - l2);
    xTop("selTop", selYMin + t - t2);
    xWidth("selTop", (selXMax - selXMin));

    xLeft("selBottom", selXMin + l - l2);
    xTop("selBottom", selYMax + t - t2);
    xWidth("selBottom", selXMax - selXMin);

    xLeft("selLeft", selXMin + l - l2);
    xTop("selLeft", selYMin + t - t2);
    xHeight("selLeft", Math.max(selYMax - selYMin,1));
  
    xLeft("selRight", selXMax + l - l2);
    xTop("selRight", selYMin + t - t2);
    xHeight("selRight", Math.max(selYMax - selYMin,1)+1);
  }
}

function selMouseDown(evt) {
  selGetPos("img0", evt);

  if (!selDragging) {
    selDragging = 1;
    selX1 = selX;
    selY1 = selY;
  } else {
    selDragging = 0;
    selX2 = selX;
    selY2 = selY;
  }
  //alert("x1=" + selX1 + " y1=" + selY1 + " x2=" + selX2 + " y2=" + selY2);
}

function selMouseMove(evt) {
  if (selDragging) {
    selGetPos("img0", evt);
    selX2 = selX;
    selY2 = selY;
    selFindMins();
    selUpdateBox(); 
  }
}

function selClear(evt) {
  selX1 = 0;
  selX2 = 1;
  selY1 = 0;
  selY2 = 1;
  selFindMins();
  selUpdateBox();
}

EXTIMG = 0
function selSearch(evt,scale) {
  selWidth = el("cropbox").width;
  selHeight = el("cropbox").height;
  
//   alert("x1=" + selXMin + " y1=" + selYMin + " x2=" + selXMax + " y2=" + selYMax);
  
//   var use_qe = false;
//   var use_qe_el = el("use_qe");
//   if (use_qe_el) { use_qe = use_qe_el.checked; }
  var xmin = (selXMin / scale).toFixed(0);
  var xmax = (selXMax / scale).toFixed(0);
  var ymin = (selYMin / scale).toFixed(0);
  var ymax = (selYMax / scale).toFixed(0);
  
//   alert("xl=" + xmin + " xu=" + xmax + " yl=" + ymin + " yu=" + ymax);
  
  
  var href = URLBASE + '&xl=' + String(xmin)
                     + '&yl=' + String(ymin)
                     + '&xu=' + String(xmax)
                     + '&yu=' + String(ymax);
//                      + '&use_qe=' + String(use_qe);
  
  window.location.href = href;
  
}

function selInit() {
  if (!el("img0").complete) {
    setTimeout('selInit()', 100);
    return;
  }
  drawVLine("img0", "img0cont", 0, 0, 1, '#ff0', 'id="selTop" onmousemove="selMouseMove(event);" onmousedown="selMouseDown(event);"');
  drawVLine("img0", "img0cont", 1, 0, 1, '#ff0', 'id="selBottom" onmousemove="selMouseMove(event);" onmousedown="selMouseDown(event);"');
  drawHLine("img0", "img0cont", 0, 0, 1, '#ff0', 'id="selLeft" onmousemove="selMouseMove(event);" onmousedown="selMouseDown(event);"');
  drawHLine("img0", "img0cont", 1, 0, 1, '#ff0', 'id="selRight" onmousemove="selMouseMove(event);" onmousedown="selMouseDown(event);"');
  selWidth = el("img0").width;
  selHeight = el("img0").height;
  selUpdateBox();
}

function gotoSel(esl, sil, mn, shots, frames) {
  if (frames=="") {
    window.location.href = esl + "?shots+list=" + shots + "&movie+name=" + mn;
  }
  else {
    window.location.href = sil + "?frames+list=" + frames + "&movie+name=" + mn;
  }
}

function checkEnter(e){
  var characterCode;

  if(e && e.which){ //if which property of event object is supported (NN4)
    e = e;
    characterCode = e.which;
  }
  else{
    e = event;
    characterCode = e.keyCode;
  }

  if(characterCode == 13){
    return true;
  }
  else {
    return false;
  }
}


