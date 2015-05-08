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
          Drupal.Mapbox.map.scrollWheelZoom.disable();

          // Wait until Mapbox is loaded
          Drupal.Mapbox.map.on('load', function() {
            if (typeof setting.mapboxBridge.data != 'undefined' && setting.mapboxBridge.data) {
              var data = $.parseJSON(setting.mapboxBridge.data);

              // add markers
              $.each(data, function(index, markerData){
                Drupal.behaviors.mapboxBridge.addMarker(markerData);
              });

              // add the legend if necessary
              if (setting.mapboxBridge.legend) {
                Drupal.behaviors.mapboxBridge.addLegend(setting, data);
              }
            }
          });

        });
      }
    },

    /**
    * Add marker to the map
    * */
    addMarker: function(markerData) {

      // for custom icons provided by drupal
      if (markerData.icon) {
        if (typeof Drupal.Mapbox.icons[markerData.name] == 'undefined') {
          Drupal.Mapbox.icons[markerData.name] = {
            name: markerData.name,
            iconUrl: markerData.icon
          };
        }

        if (typeof Drupal.Mapbox.icons[markerData.name]['marker'] == 'undefined') {
          // load image to get its size
          $('<img/>').attr('src', markerData.icon).load(function(){

            // create leaflet marker and save it
            Drupal.Mapbox.icons[markerData.name]['marker'] = L.icon({
              'iconUrl': markerData.icon,
              'iconSize': [this.width, this.height],
              'iconAnchor': [0, 0],
              'popupAnchor': [0, -this.height],
              'className': 'custom-marker'
            });

            L.marker([markerData.lat, markerData.lon], {
              icon: Drupal.Mapbox.icons[markerData.name]['marker']
            }).addTo(Drupal.Mapbox.map);

          });
        } else {

          // set the marker with the custom icon
          L.marker([markerData.lat, markerData.lon], {
            icon: Drupal.Mapbox.icons[markerData.name]['marker']
          }).addTo(Drupal.Mapbox.map);

        }

      // for icons based on mapbox
      } else if (markerData.type) {

        var icon = L.mapbox.marker.icon({
          'marker-symbol': markerData.type
        });

        if (typeof Drupal.Mapbox.icons[markerData.name] == 'undefined') {
          Drupal.Mapbox.icons[markerData.name] = {
            name: markerData.name,
            iconUrl: icon.options.iconUrl
          };
        }

        // set the marker using the mapbox icons
        L.marker([markerData.lat, markerData.lon], {
          icon: icon
        }).addTo(Drupal.Mapbox.map);

      // fallback for when nothing is provided.
      } else {
        L.marker([markerData.lat, markerData.lon]).addTo(Drupal.Mapbox.map);
      }

    },
    // end Drupal.behaviors.mapboxBridge

    /*
    * Add a container with all the used markers
    * */
    addLegend: function(setting) {

      // add legend container
      $('<div id="mapbox-legend" class="mapbox-legend"><ul class="legends"></ul></div>').insertAfter('#map');

      // loop through the stored icons
      $.each(Drupal.Mapbox.icons, function(i, legend){
        if (legend.iconUrl && legend.name) {
          $('<li class="legend">' +
            '<div class="legend-icon"><img src="' + legend.iconUrl + '"></div>' +
            '<div class="legend-name">' + legend.name + '</div>' +
          '</li>').appendTo('.mapbox-legend .legends');
        }
      });
    }
  };

})(jQuery);
