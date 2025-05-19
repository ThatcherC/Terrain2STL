var map;
var gridlines;
var rectangle;
var latBox;
var lngBox;
var mapCenter;
var minBoxWidth = 0.03333; //width of box in degrees = 40 arc seconds
var boxWidth = 0.03333;
var boxHeight = 0.03333;
var boxRotation = 0;
var sizeSlider;
var sizeLabel;
var scaleSlider;
var scaleLabel;
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
    mapTypeId:google.maps.MapTypeId.TERRAIN,
    gestureHandling: 'greedy'
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
    {lat: mapCenter.lat()-boxHeight/2, lng:mapCenter.lng()-boxWidth/2},
    {lat: mapCenter.lat()-boxHeight/2, lng:mapCenter.lng()+boxWidth/2},
    {lat: mapCenter.lat()+boxHeight/2, lng:mapCenter.lng()+boxWidth/2},
    {lat: mapCenter.lat()+boxHeight/2, lng:mapCenter.lng()-boxWidth/2},
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

  widthSlider = document.getElementsByName("boxWidth")[0];
  widthLabel = document.getElementById("boxWidthLabel");
  heightSlider = document.getElementsByName("boxHeight")[0];
  heightLabel = document.getElementById("boxHeightLabel");
  scaleSlider = document.getElementsByName("boxScale")[0];
  scaleLabel = document.getElementById("boxScaleLabel");
  rotationSlider = document.getElementsByName("rotation")[0];
  rotationLabel = document.getElementById("rotationLabel");
  vScaleSlider = document.getElementsByName("vScale")[0];
  vScaleLabel = document.getElementById("vScaleLabel");
  waterDropSlider = document.getElementsByName("waterDrop")[0];
  waterDropLabel = document.getElementById("waterDropLabel");
  baseHeightSlider = document.getElementsByName("baseHeight")[0];
  baseHeightLabel = document.getElementById("baseHeightLabel");

  google.maps.event.addListener(rectangle, 'dragend', postDrag);	//call function after rect is dragged

  initializeForm();

  ingestURLParams();
  updateLatLng();
  changeSize();
  changeRotation();
  changeVScale();
  changeWaterDrop();
  changeBaseHeight();
}

// set form values from any URL parameters that may be present
function ingestURLParams(){
  // Get URL parameters
  const urlParams = new URLSearchParams(window.location.search);
			
  // Get form element
  const form = document.getElementById('paramForm');
  
  // If no form is found, exit
  if (!form) return;
  
  // Get all form inputs, selects, and textareas
  // const formElements = form.querySelectorAll('input, select, textarea');
  const formElements = form.querySelectorAll('input');

  // Keep track of which update functions we'll need to call
  const functionsToCall = new Set();
  
  // Loop through all form elements
  formElements.forEach(function(element) {
    // Try to match by name attribute first
    let paramName = element.name;

    // If no name, try id attribute (removing any prefix like 'c-')
    if (!paramName && element.id) {
    paramName = element.id.replace(/^[a-z]-/i, '');
    }
    
    // Skip elements without a usable identifier
    if (!paramName) return;
    
    // Check if this parameter exists in the URL
    if (urlParams.has(paramName)) {
    // Set the value
    element.value = urlParams.get(paramName);
    
    // Trigger change event
    const event = new Event('change');
    element.dispatchEvent(event);
    
    }
  });
}

//https://developer.mozilla.org/en-US/docs/Learn/HTML/Forms/Sending_forms_through_JavaScript#Sending_form_data
function initializeForm() {
  var form = document.getElementById("paramForm");
  var downloadButton = document.getElementById("downloadbtn");
  var genButton = document.getElementById("genButton");
  
  // Replace jQuery form submit handler with vanilla JS
  form.addEventListener("submit", function(event) {
    // Create URL-encoded form data string manually instead of jQuery serialize
    const formData = new FormData(form);
    const serializedData = new URLSearchParams(formData).toString();
    
    sendData(serializedData);
    event.preventDefault();
  });

  function sendData(str) {
    genButton.classList.add("disabled");
    genButton.innerHTML = "<i>Generating...</i>";
    
    downloadButton.style.visibility = "hidden";
    var XHR = new XMLHttpRequest();

    // Define what happens on successful data submission
    XHR.addEventListener("load", function(event) {
      var modelNumber = event.target.responseText;
      var modelName = "stls/terrain-" + modelNumber + ".zip";

      // Make download button visible
      downloadButton.style.visibility = "visible";
      // Set the href attribute
      downloadButton.href = modelName;
      
      genButton.classList.remove("disabled");
      genButton.innerHTML = "Generate Model";
    });

    // Define what happens in case of error
    XHR.addEventListener("error", function(event) {
      console.log('Oops! Something went wrong.');
    });

    // Set up our request
    XHR.open("POST", "gen", true);
    XHR.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');

    // The data sent is what the user provided in the form
    XHR.send(str);
  }
}


function centerToView(){
  mapCenter = map.getCenter();
  var _lat = mapCenter.lat()+boxHeight/2;
  var _lng = mapCenter.lng()-boxWidth/2;

  updateRectangle(
    [ {lat: _lat-boxHeight, lng: _lng},
      {lat: _lat-boxHeight, lng: _lng+boxWidth},
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
  var boxScale = scaleSlider.value;
  scaleLabel.innerHTML = scaleSlider.value;

  boxWidth=minBoxWidth*widthSlider.value*boxScale/120;
  boxHeight=minBoxWidth*heightSlider.value*boxScale/120;
  widthLabel.innerHTML =  (widthSlider.value*boxScale /3600).toFixed(2)+"\xB0";
  heightLabel.innerHTML = (heightSlider.value*boxScale/3600).toFixed(2)+"\xB0";
  centerToView();
}

function changeRotation(){
  rotationLabel.innerHTML = rotationSlider.value;
  boxRotation = rotationSlider.value*Math.PI/180;

  var base = rectangle.getPath().getAt(3);

  var rotLat = {lat: Math.sin(boxRotation)*boxHeight,lng:Math.cos(boxRotation)*boxWidth};
  var rotLng = {lat: Math.cos(boxRotation)*boxHeight, lng:Math.sin(boxRotation)*boxWidth};
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
  var _lat = Math.min(Math.max(parseFloat(document.getElementById('c-lat').value),-69),69);
  var _lng = Math.min(Math.max(parseFloat(document.getElementById('c-lng').value),-179),180);
  document.getElementById('c-lat').value = _lat;
  document.getElementById('c-lng').value = _lng;
  if(_lat && _lng){
    updateRectangle([ {lat: _lat-boxHeight, lng: _lng},
      {lat: _lat-boxHeight, lng: _lng+boxWidth},
      {lat: _lat, lng:_lng+boxWidth},
      {lat: _lat, lng: _lng},
    ]);
    map.setCenter({lat: _lat, lng: _lng});
  }
}
