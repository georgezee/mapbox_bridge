(function ($) {

  Drupal.Mapbox = {};
  Drupal.Mapbox.icons = {};

  /**
   * Mapbox with very basic setup
   */
  Drupal.behaviors.mapboxBridge = {
    attach: function (context, setting) {
      if (typeof L != 'undefined' && $('#map', context).length) {
        $('#map', context).once('mapbox-bridge', function(){

          // access token for mapbox
          L.mapbox.accessToken = setting.mapboxBridge.publicToken;

          // Load Mapbox with supplied ID
          Drupal.Mapbox.map = L.mapbox.map('map', setting.mapboxBridge.mapId);

          // Wait until Mapbox is loaded
          Drupal.Mapbox.map.on('load', function() {
            if (typeof setting.mapboxBridge.data != 'undefined') {
              var data = $.parseJSON(setting.mapboxBridge.data);

              // add markers
              $.each(data, function(index, markerData){
                Drupal.behaviors.mapboxBridge.addMarker(markerData);
              });
            }
          });

        });
      }
    },

    /**
    * Add marker to the map
    * */
    addMarker: function(markerData) {

      if (markerData.icon) {

        // if the icon hasnt been made yet, we create and save it into the global variable.
        if (typeof Drupal.Mapbox.icons[markerData.icon] == 'undefined') {

          // load image to get its size
          $('<img/>').attr('src', markerData.icon).load(function(){

            // create leaflet marker and save it
            Drupal.Mapbox.icons[markerData.icon] = L.icon({
              'iconUrl': markerData.icon,
              'iconSize': [this.width, this.height],
              'iconAnchor': [0, 0],
              'popupAnchor': [0, -this.height],
              'className': 'custom-marker'
            });

            L.marker([markerData.lat, markerData.lon], {
              icon: Drupal.Mapbox.icons[markerData.icon]
            }).addTo(Drupal.Mapbox.map);
          });
        } else {

          // set the marker with the custom icon
          L.marker([markerData.lat, markerData.lon], {
            icon: Drupal.Mapbox.icons[markerData.icon]
          }).addTo(Drupal.Mapbox.map);
        }

      } else if (markerData.type) {

        // set the marker using the mapbox icons
        L.marker([markerData.lat, markerData.lon], {
          icon: L.mapbox.marker.icon({
            'marker-symbol': markerData.type
          })
        }).addTo(Drupal.Mapbox.map);

      } else {

        // this is the fallback which uses the default mapbox marker without an icon
        L.marker([markerData.lat, markerData.lon]).addTo(Drupal.Mapbox.map);

      }

    }
    // end Drupal.behaviors.mapboxBridge
  };

})(jQuery);