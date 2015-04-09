(function ($) {

  Drupal.Mapbox = {};

  /**
   * Mapbox with very basic setup
   */
  Drupal.behaviors.mapboxBridge = {
    attach: function (context, setting) {

      L.mapbox.accessToken = setting.mapboxBridge.publicToken;
      var map = L.mapbox.map('map', setting.mapboxBridge.mapId);

    }
  };

})(jQuery);