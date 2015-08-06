(function ($) {

  Drupal.Mapbox = {};
  Drupal.Mapbox.icons = {};
  Drupal.Mapbox.layers = {};
  Drupal.Mapbox.filters = {};
  Drupal.Mapbox.geojson = [];
  Drupal.Mapbox.featureLayer;
  Drupal.Mapbox.layerGroup;

  /**
   * Mapbox with very basic setup
   */
  Drupal.behaviors.mapboxBridge = {
    attach: function(context, setting) {
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
              Drupal.behaviors.mapboxBridge.init($.parseJSON(setting.mapboxBridge.data), setting);
            }
          });
        });
      }
    },
    // end Drupal.behaviors.attach

    /**
     * Initialize base settings
     * */
    init: function(data, setting) {
      // add markers
      $.each(data, function(index, markerData){
        Drupal.behaviors.mapboxBridge.addMarker(markerData, setting.mapboxBridge);
      });

      // use the created geojson to load all the markers
      Drupal.Mapbox.featureLayer = L.mapbox.featureLayer(Drupal.Mapbox.geojson);

      // wrap everything in a layerGroup
      Drupal.Mapbox.layerGroup = L.layerGroup();

      if (setting.mapboxBridge.cluster) {

        // create clusterGroup and add it to the map
        var clusterGroup = new L.MarkerClusterGroup().addTo(Drupal.Mapbox.layerGroup);

        // add the featureLayer containing all the markers to the clusterGroup (so clustering happens)
        Drupal.Mapbox.featureLayer.addTo(clusterGroup);
      } else {

        // add the featureLayer containing all the markers to the map
        Drupal.Mapbox.featureLayer.addTo(Drupal.Mapbox.layerGroup);
      }

      // add the layerGroup to the map
      Drupal.Mapbox.layerGroup.addTo(Drupal.Mapbox.map);

      // set the pan & zoom of them map to show all visible markers.
      Drupal.Mapbox.map.fitBounds(Drupal.Mapbox.featureLayer.getBounds(), { maxZoom: setting.mapboxBridge.maxZoom });

      // add the legend if necessary
      if (setting.mapboxBridge.legend) {
        Drupal.behaviors.mapboxBridge.addLegend(setting, data);
      }

      // create the popups
      if (setting.mapboxBridge.popup.enabled) {
        Drupal.MapboxPopup.popups(Drupal.Mapbox.featureLayer, setting.mapboxBridge.popup.popup_viewmode);
      }

      // create filters
      if (setting.mapboxBridge.filter.enabled) {
        Drupal.MapboxFilter.filter(Drupal.Mapbox.featureLayer, setting.mapboxBridge.cluster);
      }

      // check for touch devices and disable pan and zoom
      if ('ontouchstart' in document.documentElement) {
        Drupal.behaviors.mapboxBridge.panAndZoom(false);
      }

      // enable proximity search
      if (setting.mapboxBridge.proximity) {
        Drupal.Mapbox.map.addControl(L.mapbox.geocoderControl('mapbox.places', {
          autocomplete: true
        }));
      }
    },

    /**
    * Build marker geojson
    * */
    addMarker: function(markerData, setting) {
      // for custom icons provided by drupal
      if (markerData.icon) {
        if (typeof Drupal.Mapbox.layers[markerData.name] == 'undefined') {

          // Calculate Icon's anchor position based on size and user preferences
          var iconAnchorPosition = Drupal.behaviors.mapboxBridge.getIconAnchor(
            markerData.iconWidth,
            markerData.iconHeight,
            setting.markerAnchor
          );

          // Calculate popup anchor position to be 3px above the marker and
          // always centered on top of the icon
          var popupAnchor = [
            markerData.iconWidth / 2 - parseFloat(iconAnchorPosition[0]),
            -(parseFloat(iconAnchorPosition[1]) + 3)
          ];

          // create an icon
          Drupal.Mapbox.icons[markerData.name] = {
            name: markerData.name,
            iconUrl: markerData.icon,
            marker: L.icon({
              'iconUrl': markerData.icon,
              'iconSize': [markerData.iconWidth, markerData.iconHeight],
              'iconAnchor': iconAnchorPosition,
              'popupAnchor': popupAnchor,
              'className': 'custom-marker' + (setting.popup.enabled ? ' clickable' : '')
            })
          };
        }

      // for icons based on mapbox
      } else if (typeof markerData.type != 'undefined') {

        if (typeof Drupal.Mapbox.layers[markerData.name] == 'undefined') {
          Drupal.Mapbox.icons[markerData.name] = {
            name: markerData.name,
            marker: L.mapbox.marker.icon({
              'marker-symbol': markerData.type
            })
          };
        }
      }

      if (markerData.lat && markerData.lon && typeof Drupal.Mapbox.icons[markerData.name]['marker'] != 'undefined') {
        // setup the filter properties
        if (setting.filter.enabled) {

          var filter = {};
          $.each(setting.filter.filter_fields, function(i, filter_field){
            // filter_field are entered with a type definition ":type", we need to cut this off here.
            var filter_type = filter_field.split(':')[1];
            filter_field = filter_field.split(':')[0];

            // used for geojson
            if (typeof filter[filter_field] == 'undefined') {
              filter[filter_field] = [];
            }

            // values might be separated by a comma
            markerData[filter_field] = markerData[filter_field].split(', ');

            filter[filter_field] = filter[filter_field].concat(markerData[filter_field]);

            if (typeof Drupal.Mapbox.filters[filter_field] == 'undefined') {
              Drupal.Mapbox.filters[filter_field] = {};
              Drupal.Mapbox.filters[filter_field]['options'] = {};
              Drupal.Mapbox.filters[filter_field]['type'] = filter_type;
            }

            if (typeof Drupal.Mapbox.filters[filter_field][markerData[filter_field]] == 'undefined' && markerData[filter_field]) {
              $.each(filter[filter_field], function(index, value){
                Drupal.Mapbox.filters[filter_field]['options'][value] = true;
              });
            }
          });
        }

        // create geojson object with the provided attributes
        Drupal.Mapbox.geojson.push({
          'type': 'Feature',
          'geometry': {
            'type': 'Point',
            'coordinates': [markerData.lon, markerData.lat]
          },
          'properties': {
            'icon': Drupal.Mapbox.icons[markerData.name]['marker'],
            'nid': markerData.nid,
            'filter': setting.filter.enabled ? filter : false
          }
        });
      }
    },
    // end Drupal.behaviors.mapboxBridge.addMarker

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
    // end Drupal.behaviors.mapboxBridge.addLegend

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
    },
    // end Drupal.behaviors.mapboxBridge.panAndZoom

    /**
     * Calculate marker's anchor position
     *
     * @param iconWidth
     *  Icon image width
     * @param iconHeight
     *  Icon image height
     * @param iconAnchor
     *  String describing position. E.g.: center_center, bottom_left etc
     *
     * @returns Array
     *  Array with anchor position
     */
    getIconAnchor: function(iconWidth, iconHeight, iconAnchor) {
      // separate offset description and make sure we always have to elements
      var offsets = iconAnchor.split('_').concat(['center']);

      // Calculate Y offset
      switch(offsets[0]) {
        case 'top': offsetY = 0; break;
        case 'bottom': offsetY = iconHeight; break;
        default: offsetY = iconHeight / 2;
      }

      // Calculate X offset
      switch(offsets[1]) {
        case 'left': offsetX = 0; break;
        case 'right': offsetX = iconWidth; break;
        default: offsetX = Math.ceil(iconWidth / 2);
      }

      return [offsetX, offsetY];
    }
  };
  // end Drupal.behaviors.mapboxBridge.getIconAnchor

})(jQuery);
