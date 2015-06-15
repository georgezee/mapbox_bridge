(function ($) {

  Drupal.Mapbox = {};
  Drupal.Mapbox.icons = {};
  Drupal.Mapbox.layers = {};
  Drupal.Mapbox.featureGroup;

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

          // create feature group that will contain all the layers
          Drupal.Mapbox.featureGroup = L.featureGroup().addTo(Drupal.Mapbox.map);

          // Wait until Mapbox is loaded
          Drupal.Mapbox.map.on('load', function() {
            if (typeof setting.mapboxBridge.data != 'undefined' && setting.mapboxBridge.data) {
              var data = $.parseJSON(setting.mapboxBridge.data);

              // add markers
              $.each(data, function(index, markerData){
                Drupal.behaviors.mapboxBridge.addMarker(markerData, setting.mapboxBridge);
              });

              // set the pan & zoom of them map to show all visible markers.
              Drupal.Mapbox.map.fitBounds(Drupal.Mapbox.featureGroup.getBounds(), { maxZoom: setting.mapboxBridge.maxZoom });

              // add the legend if necessary
              if (setting.mapboxBridge.legend) {
                Drupal.behaviors.mapboxBridge.addLegend(setting, data);
              }

              // create the popups
              if (setting.mapboxBridge.popup.enabled) {
                Drupal.MapboxPopup.popups(Drupal.Mapbox.layers, setting.mapboxBridge.popup.popup_viewmode);
              }

              // check for touch devices and disable pan and zoom
              if ('ontouchstart' in document.documentElement) {
                Drupal.behaviors.mapboxBridge.panAndZoom(false);
              }
            }
          });

        });
      }
    },
    // end Drupal.behaviors.attach

    /**
    * Add marker to the map
    * */
    addMarker: function(markerData, setting) {

      // for custom icons provided by drupal
      if (markerData.icon) {
        if (typeof Drupal.Mapbox.layers[markerData.name] == 'undefined') {

          // create an icon
          Drupal.Mapbox.icons[markerData.name] = {
            name: markerData.name,
            iconUrl: markerData.icon,
            marker: L.icon({
              'iconUrl': markerData.icon,
              'iconSize': [markerData.iconWidth, markerData.iconHeight],
              'iconAnchor': [markerData.iconWidth / 2, markerData.iconHeight],
              'popupAnchor': [0, -(markerData.iconHeight+3)],
              'className': 'custom-marker' + (setting.popup.enabled ? ' clickable' : '')
            })
          };

          // setup a new layer, one layer per type
          Drupal.Mapbox.layers[markerData.name] = L.featureGroup().addTo(Drupal.Mapbox.featureGroup);
        }

      // for icons based on mapbox
      } else if (markerData.type) {

        if (typeof Drupal.Mapbox.layers[markerData.name] == 'undefined') {
          Drupal.Mapbox.icons[markerData.name] = {
            name: markerData.name,
            marker: L.mapbox.marker.icon({
              'marker-symbol': markerData.type
            })
          };

          // setup a new layer, one layer per type
          Drupal.Mapbox.layers[markerData.name] = L.featureGroup().addTo(Drupal.Mapbox.featureGroup);
        }
      }

      if (markerData.lat && markerData.lon && typeof Drupal.Mapbox.icons[markerData.name]['marker'] != 'undefined') {

        // set the marker with the custom icon
        L.marker([markerData.lat, markerData.lon], {
          icon: Drupal.Mapbox.icons[markerData.name]['marker'],
          clickable: (setting.popup.enabled ? true : false),
          nid: markerData.nid
        }).addTo(Drupal.Mapbox.layers[markerData.name]);
      }
    },
    // end Drupal.behaviors.addMarker

    /*
    * Add a legend container with all the used markers
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
    },
    // end Drupal.behaviors.addLegend

    /*
    * This disables the pan and zoom controls via input devices (mouse, touch, ect.)
    * and replaces it with controls layed over the map.
    * */
    panAndZoom: function(enable) {
      if (!enable) {
        // Disable drag and zoom handlers.
        Drupal.Mapbox.map.dragging.disable();
        Drupal.Mapbox.map.touchZoom.disable();
        Drupal.Mapbox.map.doubleClickZoom.disable();

        // Disable tap handler, if present.
        if (Drupal.Mapbox.map.tap) Drupal.Mapbox.map.tap.disable();

        // Enable pan controls
        Drupal.MapboxPan.controls(true);
      } else {
        // Enable drag and zoom handlers.
        Drupal.Mapbox.map.dragging.enable();
        Drupal.Mapbox.map.touchZoom.enable();
        Drupal.Mapbox.map.doubleClickZoom.enable();

        // Enable tap handler, if present.
        if (Drupal.Mapbox.map.tap) Drupal.Mapbox.map.tap.enable();

        // Disable pan controls
        Drupal.MapboxPan.controls(false);
      }
    }
  };

})(jQuery);
