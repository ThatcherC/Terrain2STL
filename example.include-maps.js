// use Javascript Dynamic Script Loading 
var script = document.createElement('script');
script.type= 'text/javascript';
script.src = "https://maps.googleapis.com/maps/api/js?key=GOOGLE_API_KEY&libraries=places,maps&callback=initializeMap";
script.async=true;
script.defer=true;
document.head.appendChild(script)
