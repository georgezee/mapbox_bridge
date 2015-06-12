(function ($) {

  jQuery.fn.animateAuto = function(prop, speed, callback){
    var elem, height, width;
    return this.each(function(i, el){
      el = jQuery(el), elem = el.clone().css({"height":"auto","width":"auto"}).appendTo("body");
      height = elem.css("height"),
        width = elem.css("width"),
        elem.remove();

      if(prop === "height")
        el.animate({"height":height}, speed, callback);
      else if(prop === "width")
        el.animate({"width":width}, speed, callback);
      else if(prop === "both")
        el.animate({"width":width,"height":height}, speed, callback);
    });
  };

  /**
   * Mapbox / Leaflet popups
   *
   * @see http://leafletjs.com/reference.html#popup
   */
  Drupal.MapboxPopup = {
    popups: function (groups, viewmode) {

      // go through each group, then through each layer
      $.each(groups, function(id, layers){
        layers.eachLayer(function(layer) {

          // This is an empty container that we will replace via ajax
          var content = '<div class="custom-popup-content loading" id="custom-popup-id-' + layer._leaflet_id + '"><\/div>';

          // setup a minimum with for the popup, see http://leafletjs.com/reference.html#popup for other options
          layer.bindPopup(content, {
            minWidth: 150
          })
            // Bind popups to a click event
            .on('click', function(e) {
              $( '#custom-popup-id-' + e.target._leaflet_id ).load('/mapbox_bridge_ajax_content/' + viewmode + '/' + e.target.options.nid, function(e){
                var $this = $(this),
                    $content = $('> div:first-child', $this);

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

                $this.removeClass('loading');
              });
            });
        });
      });
    }
  };
})(jQuery);