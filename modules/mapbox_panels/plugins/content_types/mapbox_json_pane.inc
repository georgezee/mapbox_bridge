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

  $block->content = mapbox_bridge_render_map(
    $conf['mapbox_id'],
    $conf['mapbox_json'],
    'json',
    $conf['legend'],
    $conf['mapbox_max_zoom'],
    array('enabled' => $conf['popup'], 'popup_viewmode' => $conf['popup_viewmode']),
    $conf['filter']
  );

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
  $form['mapbox_marker']['filter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mapbox Filter'),
    '#description' => t('Filter markers based on the "filter" attribute within the JSON'),
    '#default_value' => $conf['filter'],
    '#mandatory' => TRUE
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