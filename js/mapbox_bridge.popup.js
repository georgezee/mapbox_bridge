(function ($) {

  /**
   * Mapbox / Leaflet popups
   *
   * @see http://leafletjs.com/reference.html#popup
   */
  Drupal.MapboxPopup = {
    popups: function (layers, viewmode) {

      // go through each group, then through each layer
      layers.eachLayer(function(layer) {

        // This is an empty container that we will replace via ajax
        var content = '<div class="custom-popup-content loading" id="custom-popup-id-' + layer._leaflet_id + '"><\/div>';

        // setup a minimum with for the popup, see http://leafletjs.com/reference.html#popup for other options
        layer.bindPopup(content, {
          minWidth: 150
        })
          // Bind popups to a click event
          .on('click', function(marker) {

            // load the node with the supplied viewmode
            $( '#custom-popup-id-' + marker.target._leaflet_id ).load('/mapbox_bridge_ajax_content/' + viewmode + '/' + marker.target.feature.properties.nid, function(content){
              var $this = $(this),
                  $content = $('> div:first-child', $this);

              // gracefully slide in the content
              $content
                .css({
                  width: $this.width() + 'px', // to fix jQuery's jumpy sliding effect
                  opacity: 0
                })
                .slideDown('normal').after(function(){
                  $content.animate({
                    opacity: 1
                  }, 'fast');
                });

              // remove loading indicator
              $this.removeClass('loading');

              // center the newly clicked marker
              var px = Drupal.Mapbox.map.project(marker.target._latlng); // find the pixel location on the map where the popup anchor is
                  px.y -= marker.target._popup._container.clientHeight / 2; // find the height of the popup container, divide by 2, subtract from the Y axis of marker location

              Drupal.Mapbox.map.panTo( Drupal.Mapbox.map.unproject(px), { animate: true }); // pan to new center
            });
          });
      });
    }
  };
})(jQuery);