rolloverImagesOn=new Array();
rolloverImagesOff=new Array();

function rolloverOn(id) {
  if(rolloverImagesOn[id]){
    document.images[id].src=rolloverImagesOn[id].src;
  }
}

function rolloverOff(id) {
  if(rolloverImagesOff[id]){
      document.images[id].src=rolloverImagesOff[id].src;
  }
}

function rolloverLoad(id,on,off) {
  rolloverImagesOn[id]=new Image();
  rolloverImagesOn[id].src=on;
  rolloverImagesOff[id]=new Image();
  rolloverImagesOff[id].src=off;
}
