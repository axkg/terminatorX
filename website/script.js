imgOn=new Array();
imgOff=new Array();

function on(id) {
  if(imgOn[id]) document.images[id].src=imgOn[id].src;
}

function off(id) {
  if(imgOff[id]) document.images[id].src=imgOff[id].src;
}

function load(id,on,off) {
  imgOn[id]=new Image();
  imgOn[id].src=on;
  imgOff[id]=new Image();
  imgOff[id].src=off;
}
