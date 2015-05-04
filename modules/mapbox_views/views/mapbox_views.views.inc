<?php

/**
 * Implements hook_views_data().
 */
function mapbox_views_views_data() {

  $data['mapbox_views']['table']['group'] = t('Mapbox');
  $data['mapbox_views']['table']['join'] = array(
    // #global is a special flag which let's a table appear all the time.
    '#global' => array(),
  );

  $data['mapbox_views']['mapbox'] = array(
    'title' => t('Map with markers'),
    'help' => t('Displays results as markers on map.'),
    'area' => array(
      'handler' => 'views_handler_area_mapbox_views',
    ),
  );

  return $data;
}