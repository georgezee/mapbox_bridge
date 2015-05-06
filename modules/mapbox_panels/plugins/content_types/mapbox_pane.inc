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

  $block->content = mapbox_bridge_render_map($conf['mapbox_id'], FALSE, 'mapbox-pane');

  return $block;
}

/*
 * Settings form for pane
 * */
function mapbox_pane_content_type_edit_form($form, &$form_state) {

  $conf = $form_state['conf'];

  $t_args = array('!mapbox' => l('www.mapbox.com/projects', 'https://www.mapbox.com/projects/', array('attributes' => array('target' => '_blank'))));
  $form['mapbox_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Mapbox ID'),
    '#description' => t('ID of your Mapbox project: !mapbox', $t_args),
    '#default_value' => $conf['mapbox_id'],
    '#mandatory' => TRUE
  );


  return $form;
}

/**
 * Content type submit handler.
 */
function mapbox_pane_content_type_edit_form_submit($form, &$form_state) {
  foreach (array('mapbox_id') as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}