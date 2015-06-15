(function ($) {

  /**
   * Mapbox / Leaflet popups
   *
   * @see http://leafletjs.com/reference.html#popup
   */
  Drupal.MapboxPan = {
    controls: function (enable) {
      var eventType = 'ontouchstart' in document.documentElement ? 'touchstart' : 'click';

      var uiControls =  '<div class="pan-control leaflet-control">' +
                          '<div id="pan" class="ui-pan">' +
                            '<a href="#" id="up" class="panner up">&#9650;</a>' +
                            '<a href="#" id="right" class="panner right">&#9654;</a>' +
                            '<a href="#" id="down" class="panner down">&#9660;</a>' +
                            '<a href="#" id="left" class="panner left">&#9664;</a>' +
                          '</div>' +
                        '</div>';

      if (enable) {

        $('#map .leaflet-top.leaflet-right').once(function(){
          $(uiControls).appendTo('#map .leaflet-top.leaflet-right');

          $('.panner').on(eventType, function(e){
            var $this = $(this);

            e.preventDefault();

            if ($this.hasClass('up'))
              Drupal.Mapbox.map.panBy([0, -100]);

            if ($this.hasClass('left'))
              Drupal.Mapbox.map.panBy([-100, 0]);

            if ($this.hasClass('down'))
              Drupal.Mapbox.map.panBy([0, 100]);

            if ($this.hasClass('right'))
              Drupal.Mapbox.map.panBy([100, 0]);
          });
        });

      } else {
        $('.pan-control').remove();
      }
    }
  };
})(jQuery);