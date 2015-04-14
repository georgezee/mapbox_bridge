(function ($) {

  Drupal.Mapbox = {};

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

          // Wait till Mapbox is loaded
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
      L.marker([markerData.lat, markerData.lon]).addTo(Drupal.Mapbox.map);
    }

    // end Drupal.behaviors.mapboxBridge
  };

})(jQuery);