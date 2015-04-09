<?php

/**
 * Implements hook_views_data().
 *
 */
function mapbox_views_views_data() {

  $data['mapbox_views']['table']['group'] = t('Global');
  $data['mapbox_views']['table']['join'] = array(
    // #global is a special flag which let's a table appear all the time.
    '#global' => array(),
  );

  $data['mapbox_views']['mapbox'] = array(
    'title' => t('Mapbox Views'),
    'help' => t('Mapbox with equal results but no limit.'),
    'area' => array(
      'handler' => 'views_handler_area_mapbox_views',
    ),
  );

  return $data;

}