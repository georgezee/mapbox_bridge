(function ($) {

  /**
   * Mapbox / Leaflet popups
   *
   * @see http://leafletjs.com/reference.html#popup
   */
  Drupal.MapboxFilter = {
    filter: function (layers, filters) {
      $('#map').once('mapbox-filters', function(){

        // setup the filter for the user
        $('<div id="mapbox-filters" class="mapbox-filters"></div>').insertBefore('#map');

        $.each(Drupal.Mapbox.filters, function(filter, options){
          $('<div class="mapbox-bridge-filter filter-id-' + filter + '" id="filter-id-' + filter + '"><h3>' + Drupal.t(filter) + '</h3></div>').appendTo('#mapbox-filters');

          $.each(options, function(option, enabled){
            if (enabled) {
              $('#filter-id-' + filter).append('<input id="' + option + '" name="' + filter + '" type="checkbox" value="' + option + '"><label for="' + option + '">' + Drupal.t(option) + '</label>');
            }
          });
        });

        // setup the filter behavior
        $('.mapbox-bridge-filter input').on('change', function(){
          var activeFilter = {},
              $this = $(this),
              filtermatch = false;

          $('.mapbox-filters input:checked').each(function(){
            activeFilter[$(this).attr('name')] = $(this).val();
          });

          if ($.isEmptyObject(activeFilter)) {
            filtermatch = true;
          }

          Drupal.Mapbox.featureLayer.setFilter(function(feature) {
            // no filter is set, show everything
            if (filtermatch) {
              return true;
            } else {
              if (feature.properties.filter[$this.attr('name')] == $this.val()) {
                return true;
              } else {
                return false;
              }
            }
          });
        });
      });
    }
  };
})(jQuery);