<?php

/**
 * Module settings form.
 */
function mapbox_bridge_admin_settings($form, &$form_state) {

  $form['mapbox_bridge_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Secret Token'),
    '#default_value' => variable_get('mapbox_bridge_secret', NULL),
    '#size' => 60,
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  $form['mapbox_bridge_public'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Public Token'),
    '#default_value' => variable_get('mapbox_bridge_public', NULL),
    '#size' => 60,
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  return system_settings_form($form);
}