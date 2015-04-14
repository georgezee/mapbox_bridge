<?php

/**
 * Views area text custom handler.
 *
 * @ingroup views_area_handlers
 */
class views_handler_area_mapbox_views extends views_handler_area {

  function option_definition() {
    $options = parent::option_definition();
    unset($options['label']);
    $options['mapbox_id'] = array('default' => '');
    $options['mapbox_field'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['label']['#type'] = 'hidden';

    $form['mapbox_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Mapbox ID'),
      '#description' => t('Enter the ID of your Mapbox project: https://www.mapbox.com/projects/'),
      '#default_value' => $this->options['mapbox_id'],
    );

    $form['mapbox_field'] = array(
      '#type' => 'textfield',
      '#title' => t('Mapbox Geofield'),
      '#description' => t('Geofield which contains the coordinates'),
      '#default_value' => $this->options['mapbox_field'],
      '#autocomplete_path' => 'mapbox_views/field_matching/autocomplete',
    );

    return $form;
  }

  function render($empty = FALSE) {
    if (!$empty || !empty($this->options['empty'])) {
      return $this->render_mapbox($this->options['mapbox_id'], $this->options['mapbox_field']);
    }

    return '';
  }

  /**
   * Render Mapbox
   */
  function render_mapbox($mapbox_id, $mapbox_field) {
    if ($mapbox_id && $mapbox_field) {
      return mapbox_views_build($mapbox_id, $mapbox_field, $this->view);
    }

    return '';
  }
}