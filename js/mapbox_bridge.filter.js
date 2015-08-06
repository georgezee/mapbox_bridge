(function ($) {

  /**
   * Mapbox Filtering
   */
  Drupal.MapboxFilter = {
    filter: function(layers, cluster) {
      $('#map').once('mapbox-filters', function(){

        // setup the forms for filtering
        Drupal.MapboxFilter.setup();

        // handle the form events
        Drupal.MapboxFilter.events(layers, cluster);
      });
    },

    /**
    * Setup of the filtering elements
    * */
    setup: function() {
      // setup the filter for the user
      $('<div id="mapbox-filters" class="mapbox-filters"></div>').insertBefore('#map');

      $.each(Drupal.Mapbox.filters, function(filter, attributes){

        // setup the filter title
        $('<div class="mapbox-bridge-filter filter-id-' + filter + '" id="filter-id-' + filter + '"><h3>' + Drupal.t(filter) + '</h3></div>').appendTo('#mapbox-filters');

        // setup the filter
        $.each(attributes.options, function(option, enabled){

          if (enabled) {
            if (attributes.type == 'select') {

              $('#filter-id-' + filter).once(function(){
                $(this).append('<select id="' + filter + '" name="' + filter + '"><option value="all">' + Drupal.t('Please select a @type', {'@type': Drupal.t(filter)}) + '</option></select>');
              });

              $('#' + filter).append('<option value="' + Drupal.t(option) + '">' + Drupal.t(option) + '</option>');

            } else if (attributes.type == 'checkbox') {

              $('#filter-id-' + filter).append('<input id="' + option + '" name="' + filter + '" type="checkbox" value="' + option + '"><label for="' + option + '">' + Drupal.t(option) + '</label>');
            } else if (attributes.type == 'radio') {

              $('#filter-id-' + filter).append('<input id="' + option + '" name="' + filter + '" type="radio" value="' + option + '"><label for="' + option + '">' + Drupal.t(option) + '</label>');
            }
          }
        });
      });
    },

    /**
    * Change events to the setup filter elements
    * */
    events: function(layers, cluster) {
      // setup the filter behavior
      $('.mapbox-bridge-filter').find('input, select').on('change', function(){
        var activeFilter = {},
          $filters = $('.mapbox-bridge-filter').find('input:checked, select');

        // setup the active filters so we can check back and see what is active
        $filters.each(function(){
          var $this = $(this);

          if ($this.val() != 'all') {
            if (typeof activeFilter[$this.attr('name')] == 'undefined') {
              activeFilter[$this.attr('name')] = [];
            }

            activeFilter[$this.attr('name')].push($this.val());
          }
        });

        // call the actual filtering
        Drupal.MapboxFilter.filtering(activeFilter, layers, cluster);
      });
    },

    /**
    * The filtering of the markers
    * */
    filtering: function(activeFilter, layers, cluster) {
      var filtermatch = false;

      // check if no filters are set!
      if ($.isEmptyObject(activeFilter)) {
        filtermatch = true;
      }

      // clear all the markers from the map
      Drupal.Mapbox.layerGroup.clearLayers();

      var group;
      if (cluster) {
        // crate a new group that we add to the layerGroup which is already added to the map
        group = new L.MarkerClusterGroup().addTo(Drupal.Mapbox.layerGroup);
      } else {
        // use group as the reference to featureLayer
        group = Drupal.Mapbox.layerGroup;
      }

      // such cheating, so wow
      // The filtering takes a bit of time when there are a lot of markers, by adding a delay the checkbox will
      // react more "natural" and quick.
      setTimeout(function(){
        layers.eachLayer(function(layer) {
          // no filter is set, show everything
          if (filtermatch) {
            group.addLayer(layer);
          } else {
            // this is were we store if the current filter matches
            var filterCheck = {},
                show = true;

            // loop through all the filters that are set inside the layer and match them against the currently
            // active filters.
            $.each(layer.feature.properties.filter, function(filter_name, filter_values){
              if (typeof activeFilter[filter_name] != 'undefined') {
                $.each(filter_values, function(index, value){
                  if (activeFilter[filter_name].indexOf(value) !== -1) {
                    filterCheck[filter_name] = true;
                  } else if (!filterCheck[filter_name]) {
                    filterCheck[filter_name] = false;
                  }
                });

                // final check
                if (!filterCheck[filter_name]) {
                  show = false;
                }
              }
            });

            // finally, add the layer that passed all filter checks to the cluster
            if (show) {
              group.addLayer(layer);
            }
          }
        });
      }, 50);
    }
  };
})(jQuery);