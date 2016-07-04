var map;
var gridlines;
var rectangle;
var latBox;
var lngBox;
var mapCenter;
var minBoxWidth = 0.03333; //width of box in degrees = 40 arc seconds
var boxWidth = 0.03333;
var boxRotation = 0;
var sizeSlider;
var sizeLabel;
var rotationSlider;
var rotationLabel;
var vScaleSlider;
var vScaleLabel;
var waterDropSlider;
var waterDropLabel;
var baseHeightSlider;
var baseHeightLabel;


function initializeMap(){
  mapCenter = new google.maps.LatLng(44.191442, -69.074608);

  var mapOptions = {
    center: mapCenter,
    zoom: 6,
    minZoom: 3,
    maxZoom: 13,
    streetViewControl: false,
    mapTypeControlOptions: {
      mapTypeIds: [google.maps.MapTypeId.TERRAIN,"moon-visible","moon-elevation","mars-visible","mars-elevation"]
    },
    mapTypeId:google.maps.MapTypeId.TERRAIN
  };



  latBox = document.getElementById("c-lat");
  lngBox = document.getElementById("c-lng");

  google.maps.visualRefresh = true;
  map = new google.maps.Map(document.getElementById("gmap"),
    mapOptions);
  //addMoonMaps(map);

  google.maps.event.addListener(map, 'maptypeid_changed', function() {
    console.log(map.getMapTypeId());
  });

  var rectCorners = [
    {lat: mapCenter.lat()-boxWidth/2, lng:mapCenter.lng()-boxWidth/2},
    {lat: mapCenter.lat()-boxWidth/2, lng:mapCenter.lng()+boxWidth/2},
    {lat: mapCenter.lat()+boxWidth/2, lng:mapCenter.lng()+boxWidth/2},
    {lat: mapCenter.lat()+boxWidth/2, lng:mapCenter.lng()-boxWidth/2},
  ];

  rectangle = new google.maps.Polygon({
    strokeColor: '#FF0000',
    strokeOpacity: 0.8,
    strokeWeight: 2,
    fillColor: '#FF0000',
    fillOpacity: 0.35,
    map: map,
    paths: rectCorners,
    draggable:true,
    geodesic:true
  });


  sizeSlider = document.getElementsByName("boxSize")[0];
  sizeLabel = document.getElementById("boxSizeLabel");
  rotationSlider = document.getElementsByName("rotation")[0];
  rotationLabel = document.getElementById("rotationLabel");
  vScaleSlider = document.getElementsByName("vScale")[0];
  vScaleLabel = document.getElementById("vScaleLabel");
  waterDropSlider = document.getElementsByName("waterDrop")[0];
  waterDropLabel = document.getElementById("waterDropLabel");
  baseHeightSlider = document.getElementsByName("baseHeight")[0];
  baseHeightLabel = document.getElementById("baseHeightLabel");

  google.maps.event.addListener(rectangle, 'dragend', postDrag);	//call function after rect is dragged
}


function centerToView(){
  mapCenter = map.getCenter();
  var _lat = mapCenter.lat()+boxWidth/2;
  var _lng = mapCenter.lng()-boxWidth/2;

  updateRectangle(
    [ {lat: _lat-boxWidth, lng: _lng},
      {lat: _lat-boxWidth, lng: _lng+boxWidth},
      {lat: _lat, lng:_lng+boxWidth},
      {lat: _lat, lng: _lng},
    ]);
}

function updateRectangle(corners){
  rectangle.setPath(corners);
  postDrag();
}

function postDrag(){		//called after rectangle is dragged
  var _lat = rectangle.getPath().getAt(3).lat();
  var _lng = rectangle.getPath().getAt(3).lng();
  latBox.value = _lat.toFixed(4);
  lngBox.value = _lng.toFixed(4);
}

function changeSize(){
  boxWidth=minBoxWidth*sizeSlider.value/120;
  sizeLabel.innerHTML = sizeSlider.value;
  centerToView();
}

function changeRotation(){
  rotationLabel.innerHTML = rotationSlider.value;
  boxRotation = rotationSlider.value*Math.PI/180;

  var base = rectangle.getPath().getAt(3);

  var boxHeight = boxWidth;
  var rotLat = {lat: Math.sin(boxRotation)*boxWidth,lng:Math.cos(boxRotation)*boxWidth};
  var rotLng = {lat: Math.cos(boxRotation)*boxHeight, lng:Math.sin(boxRotation)*boxHeight};
  updateRectangle(
    [ {lat: base.lat()+rotLat.lat, lng: base.lng()+rotLat.lng},
      {lat: base.lat()+rotLat.lat-rotLng.lat, lng: base.lng()+rotLat.lng+rotLng.lng},
      {lat: base.lat()-rotLng.lat, lng:base.lng()+rotLng.lng},
      {lat: base.lat(), lng: base.lng()},
    ]);
}

function changeVScale(){
  vScaleLabel.innerHTML = vScaleSlider.value;
}

function changeWaterDrop(){
  waterDropLabel.innerHTML = waterDropSlider.value;
}

function changeBaseHeight(){
  baseHeightLabel.innerHTML = baseHeightSlider.value;
}

function updateLatLng(){
  var _lat = Math.min(Math.max(parseFloat(document.getElementById('c-lat').value),-56),60);
  var _lng = Math.min(Math.max(parseFloat(document.getElementById('c-lng').value),-179),180);
  document.getElementById('c-lat').value = _lat;
  document.getElementById('c-lng').value = _lng;
  if(_lat && _lng){
    updateRectangle([ {lat: _lat-boxWidth, lng: _lng},
      {lat: _lat-boxWidth, lng: _lng+boxWidth},
      {lat: _lat, lng:_lng+boxWidth},
      {lat: _lat, lng: _lng},
    ]);
  }
}
