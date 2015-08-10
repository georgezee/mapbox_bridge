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
  'title' => t('Mapbox Pane'),
  'description' => t('Mapbox Pane'),
  'single' => TRUE,
  'content_types' => array('mapbox_pane'),
  'render callback' => 'mapbox_pane_render_pane',
  'edit form' => 'mapbox_pane_content_type_edit_form',
  'category' => array(t('Mapbox')),
  'required context' => new ctools_context_required(t('Node'), 'node'),
);

/**
 * Render callback for the mapbox pane.
 */
function mapbox_pane_render_pane($subtype, $conf, $args, $context) {

  $block = new stdClass();

  $block->title = t('Mapbox Pane: @mapbox_id', array('@mapbox_id' => $conf['mapbox_id']));
  if ($conf['override_title']) {
    $block->title = $conf['override_title_text'];
  }

  // block content
  module_load_include('php', 'mapbox_bridge', 'inc/mapbox_area_builder');
  try {
    // get the node
    $node = $context->data;

    // create the mapbox
    if (!empty($node->{$conf['geo']}[LANGUAGE_NONE][0])) {
      $mapBuilder = new MapboxAreaBuilder(
        $node,
        $conf['mapbox_id'],
        $conf['geo'],
        $conf['type'],
        $conf['legend'],
        $conf['name'],
        $conf['icon'],
        $conf['mapbox_max_zoom'],
        array('enabled' => $conf['popup'], 'popup_viewmode' => str_replace('node_', '', $conf['popup_viewmode'])),
        array('src' => $conf['custom_icon_src'], 'width' => $conf['custom_icon_width'], 'height' => $conf['custom_icon_height']),
        $conf['marker_anchor'],
        array('enabled' => false),
        $conf['cluster'],
        $conf['proximity']
      );

      // save to block content
      $block->content = $mapBuilder->getMap();

    } else {
      $block->title = FALSE;
    }
  }
  catch(Exception $e) {
    watchdog_exception('mapbox_views', $e);
  }

  // get the node
  /*

  dpm($conf);
  dpm($node->{$conf['geo']});

  $block->content = mapbox_bridge_render_map($conf['mapbox_id'], FALSE, 'mapbox-pane');
  */
  return $block;
}

/*
 * Settings form for pane
 * */
function mapbox_pane_content_type_edit_form($form, &$form_state) {

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

  $form['mapbox_marker']['geo'] = array(
    '#type' => 'textfield',
    '#title' => t('Geofield'),
    '#description' => t('Name of the field which contains coordinates (field type: geofield)'),
    '#default_value' => $conf['geo'],
    '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/node/geofield/',
    '#mandatory' => TRUE
  );

  $form['mapbox_marker']['type'] = array(
    '#type' => 'textfield',
    '#title' => t('Type'),
    '#description' => t('Field used to determine marker type and symbol (field type: taxonomy term reference)'),
    '#default_value' => $conf['type'],
    '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/node/entityreference/',
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
    '#title' => t('Icon width'),
    '#description' => t('Icon height in pixels (needed for positioning)'),
    '#default_value' => $conf['custom_icon_height'],
    '#states' => array(
      'invisible' => array(
        ':input[name="type"]' => array('empty' => FALSE),
      )
    )
  );

  $form['mapbox_type']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Field used to get standard symbol name (e.g.: bus, heart etc) for the marker (field types: text, list_text)'),
    '#default_value' => $conf['name'],
    '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term/list_text,text/',
    '#states' => array(
      'invisible' => array(
        ':input[name="type"]' => array('empty' => TRUE),
      )
    )
  );

  $form['mapbox_type']['icon'] = array(
    '#type' => 'textfield',
    '#title' => t('Icon'),
    '#description' => t('Field used to get custom icon file (field types: image, file)'),
    '#default_value' => $conf['icon'],
    '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term/image,file/',
    '#states' => array(
      'invisible' => array(
        ':input[name="type"]' => array('empty' => TRUE),
      )
    )
  );

  return $form;
}

/**
 * Content type submit handler.
 */
function mapbox_pane_content_type_edit_form_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    $form_state['conf'][$key] = $value;
  }
}