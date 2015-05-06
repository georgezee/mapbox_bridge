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
  'title' => t('Mapbox Views Pane'),
  'description' => t('Mapbox Views as a Pane'),
  'single' => TRUE,
  'content_types' => array('mapbox_views_pane'),
  'render callback' => 'mapbox_views_pane_render_pane',
  'edit form' => 'mapbox_views_pane_content_type_edit_form',
  'category' => array(t('Mapbox')),
);

/**
 * Render callback for the mapbox views pane.
 */
function mapbox_views_pane_render_pane($subtype, $conf, $args, $context) {

  // setup the block
  $block = new stdClass();

  // block title
  $block->title = t('Mapbox Views Pane: @mapbox_id', array('@mapbox_id' => $conf['mapbox_id']));
  if ($conf['override_title']) {
    $block->title = $conf['override_title_text'];
  }

  // block content
  module_load_include('php', 'mapbox_views', 'views/mapbox_views_area_builder');
  try {
    // first value is the view_id, the second is the display_id
    $view_attributes = explode('__', $conf['mapbox_view']);

    // get the view object
    $view = views_get_view($view_attributes[0]);
    $view->set_display($view_attributes[1]);

    // create the mapbox
    $mapBuilder = new MapboxViewsAreaBuilder(
      $view,
      $conf['mapbox_id'],
      $conf['geo'],
      $conf['type'],
      $conf['legend'],
      $conf['name'],
      $conf['icon']
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
function mapbox_views_pane_content_type_edit_form($form, &$form_state) {

  $conf = $form_state['conf'];

  $t_args = array('!mapbox' => l('www.mapbox.com/projects', 'https://www.mapbox.com/projects/', array('attributes' => array('target' => '_blank'))));
  $form['mapbox_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Mapbox ID'),
    '#description' => t('ID of your Mapbox project: !mapbox', $t_args),
    '#default_value' => $conf['mapbox_id'],
    '#mandatory' => TRUE
  );
  $form['mapbox_view'] = array(
    '#type' => 'textfield',
    '#title' => t('View ID'),
    '#description' => t('View containing the data for Mapbox'),
    '#default_value' => $conf['mapbox_view'],
    '#autocomplete_path' => 'mapbox_panels_views/autocomplete/',
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
  $form['mapbox_marker']['legend'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mapbox Legend'),
    '#description' => t('Will show a legend below the map with all marker type'),
    '#default_value' => $conf['legend'],
    '#mandatory' => TRUE
  );

  $form['mapbox_type'] = array(
    '#type' => 'fieldset',
    '#title' => t('Symbol'),
    '#description' => t('Fields used to determine marker\'s type symbol'),
  );
  $form['mapbox_type']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Field used to get standard symbol name (e.g.: bus, heart etc) for the marker (field types: text, list_text)'),
    '#default_value' => $conf['name'],
    '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term/list_text,text/',
  );
  $form['mapbox_type']['icon'] = array(
    '#type' => 'textfield',
    '#title' => t('Icon'),
    '#description' => t('Field used to get custom icon file (field types: image, file)'),
    '#default_value' => $conf['icon'],
    '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term/image,file/',
  );

  return $form;
}

/**
 * Content type submit handler.
 */
function mapbox_views_pane_content_type_edit_form_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    $form_state['conf'][$key] = $value;
  }
}