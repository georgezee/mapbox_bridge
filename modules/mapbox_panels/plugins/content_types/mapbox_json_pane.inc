<?php

/**
 * @file
 * Mapbox content pane.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t('Mapbox JSON Pane'),
  'description' => t('Mapbox Pane via provided JSON'),
  'single' => TRUE,
  'content_types' => array('mapbox_json_pane'),
  'render callback' => 'mapbox_json_pane_render_pane',
  'edit form' => 'mapbox_json_pane_content_type_edit_form',
  'category' => array(t('Mapbox')),
);

/**
 * Render callback for the mapbox json pane.
 */
function mapbox_json_pane_render_pane($subtype, $conf, $args, $context) {

  // setup the block
  $block = new stdClass();

  // block title
  $block->title = t('Mapbox JSON Pane: @mapbox_id', array('@mapbox_id' => $conf['mapbox_id']));
  if ($conf['override_title']) {
    $block->title = $conf['override_title_text'];
  }

  // block content
  module_load_include('php', 'mapbox_bridge', 'inc/mapbox_area_builder');
  try {
    // create the mapbox
    $mapBuilder = new MapboxAreaBuilder(
      $conf['mapbox_json'],
      $conf['mapbox_id'],
      $conf['geo'],
      NULL,
      $conf['legend'],
      $conf['name'],
      $conf['icon'],
      $conf['mapbox_max_zoom'],
      array('enabled' => $conf['popup'], 'popup_viewmode' => str_replace('node_', '', $conf['popup_viewmode'])),
      array('src' => $conf['custom_icon_src'], 'width' => $conf['custom_icon_width'], 'height' => $conf['custom_icon_height']),
      $conf['marker_anchor'],
      array('enabled' => $conf['filter'], 'filter_fields' => explode(',', $conf['filter_fields'])),
      $conf['cluster'],
      $conf['proximity']
    );

    // save to block content
    $block->content = $mapBuilder->getMap();
  }
  catch(Exception $e) {
    watchdog_exception('mapbox_views', $e);
  }

  return $block;
}

/*
 * Settings form for pane
 * */
function mapbox_json_pane_content_type_edit_form($form, &$form_state) {

  // prepare a list of viewmodes
  $view_modes = variable_get('entity_view_modes', array());
  $options = array();

  foreach ($view_modes as $type => $view_mode) {
    foreach ($view_mode as $key => $mode) {
      $options[$type][$key] = $mode['label'];
    }
  }

  // add the drupal default Node "Teaser" and "Default" viewmodes.
  $options['node']['node_default'] = t('Default');
  $options['node']['node_teaser'] = t('Teaser');

  $conf = $form_state['conf'];

  $t_args = array('!mapbox' => l('www.mapbox.com/projects', 'https://www.mapbox.com/projects/', array('attributes' => array('target' => '_blank'))));
  $form['mapbox_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Mapbox ID'),
    '#description' => t('ID of your Mapbox project: !mapbox', $t_args),
    '#default_value' => $conf['mapbox_id'],
    '#mandatory' => TRUE
  );
  $form['mapbox_json'] = array(
    '#type' => 'textfield',
    '#title' => t('JSON'),
    '#description' => t('Path / URL to JSON file'),
    '#default_value' => $conf['mapbox_json'],
    '#mandatory' => TRUE
  );
  $form['mapbox_max_zoom'] = array(
    '#type' => 'select',
    '#title' => t('Max zoom level'),
    '#description' => t('The maximum level of zoom (the higher the level, the closer)'),
    '#default_value' => $conf['mapbox_max_zoom'] ? $conf['mapbox_max_zoom'] : 12,
    '#options' => array(
      1 => 1,
      2 => 2,
      3 => 3,
      4 => 4,
      5 => 5,
      6 => 6,
      7 => 7,
      8 => 8,
      9 => 9,
      10 => 10,
      11 => 11,
      12 => 12,
      13 => 13,
      14 => 14,
      15 => 15,
      16 => 16,
      17 => 17,
      18 => 18,
    ),
    '#mandatory' => TRUE
  );

  $form['mapbox_marker'] = array(
    '#type' => 'fieldset',
    '#title' => t('Marker'),
    '#description' => t('Fields used to get marker\'s basic information'),
    //'#collapsible' => TRUE,
  );

  $form['mapbox_marker']['popup'] = array(
    '#type' => 'checkbox',
    '#title' => t('Popup'),
    '#description' => t('Show a popup when clicking the marker'),
    '#default_value' => $conf['popup'],
  );

  $form['mapbox_marker']['popup_viewmode'] = array(
    '#type' => 'select',
    '#title' => t('Viewmode'),
    '#options' => $options,
    '#description' => t('Viewmode to be used when displaying the popup'),
    '#default_value' => $conf['popup_viewmode'],
    '#states' => array(
      'invisible' => array(
        'input[name="popup"]' => array('checked' => FALSE),
      ),
    ),
  );

  $form['mapbox_marker']['legend'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mapbox Legend'),
    '#description' => t('Will show a legend below the map with all marker type'),
    '#default_value' => $conf['legend'],
    '#mandatory' => TRUE
  );

  $form['mapbox_marker']['cluster'] = array(
    '#type' => 'checkbox',
    '#title' => t('Cluster'),
    '#description' => t('Enable clustering, example: !link', array('!link' => l(t('here'), 'https://www.mapbox.com/mapbox.js/example/v1.0.0/leaflet-markercluster/', array('attributes' => array('target' => '_blank'))))),
    '#default_value' => $conf['cluster'],
    '#mandatory' => TRUE
  );

  $form['mapbox_marker']['proximity'] = array(
    '#type' => 'checkbox',
    '#title' => t('Proximity Search'),
    '#description' => t('Enables the proximity search feeature, example: !link', array('!link' => l(t('here'), 'https://www.mapbox.com/mapbox.js/example/v1.0.0/geocoding-auto/', array('attributes' => array('target' => '_blank'))))),
    '#default_value' => $conf['proximity'],
    '#mandatory' => TRUE
  );

  $form['mapbox_marker']['marker_anchor'] = array(
    '#type' => 'select',
    '#title' => t('Marker anchor'),
    '#description' => t('What is considered to be the "tip" of the marker icon'),
    '#options' => array(
      'center_center' => t('Center'),
      'bottom_center' => t('Bottom center'),
      'bottom_left' => t('Bottom left'),
      'bottom_right' => t('Bottom right'),
      'top_left' => t('Top left'),
      'top_right' => t('Top right'),
      'top_center' => t('Top center'),
    ),
    '#default_value' => $conf['marker_anchor'],
  );

  $form['mapbox_type'] = array(
    '#type' => 'fieldset',
    '#title' => t('Symbol'),
    '#description' => t('Fields used to determine marker\'s type symbol'),
  );

  $form['mapbox_type']['custom_icon_src'] = array(
    '#type' => 'textfield',
    '#title' => t('Local path'),
    '#description' => t('Path to an image that will be used as a marker pin. E.g.: sites/all/themes/omega/pin'),
    '#default_value' => $conf['custom_icon_src'],
    '#states' => array(
      'invisible' => array(
        ':input[name="type"]' => array('empty' => FALSE),
      )
    )
  );

  $form['mapbox_type']['custom_icon_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Icon width'),
    '#description' => t('Icon width in pixels (needed for positioning)'),
    '#default_value' => $conf['custom_icon_width'],
    '#states' => array(
      'invisible' => array(
        ':input[name="type"]' => array('empty' => FALSE),
      )
    )
  );

  $form['mapbox_type']['custom_icon_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Icon height'),
    '#description' => t('Icon height in pixels (needed for positioning)'),
    '#default_value' => $conf['custom_icon_height'],
    '#states' => array(
      'invisible' => array(
        ':input[name="type"]' => array('empty' => FALSE),
      )
    )
  );

  $form['mapbox_marker']['filter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mapbox Filter'),
    '#description' => t('Filter markers based on the "filter" attribute within the JSON'),
    '#default_value' => $conf['filter'],
    '#mandatory' => TRUE
  );

  $form['mapbox_marker']['filter_fields'] = array(
    '#type' => 'textfield',
    '#title' => t('Filter fields'),
    '#description' => t('Name of the field that acts as a filter from the json and how it should be displayed. E.g. country:select (allowed: select, checkbox, radio) will render a select list with all the values from the "country" attribute inside the JSON, separate multiple fields by a comma.'),
    '#default_value' => $conf['filter_fields'],
    '#states' => array(
      'invisible' => array(
        'input[name="filter"]' => array('checked' => FALSE),
      ),
    ),
  );

  return $form;
}

/**
 * Content type submit handler.
 */
function mapbox_json_pane_content_type_edit_form_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    $form_state['conf'][$key] = $value;
  }
}