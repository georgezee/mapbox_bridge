<?php

/**
 * Views area text custom handler.
 *
 * @ingroup views_area_handlers
 */
class views_handler_area_mapbox_views extends views_handler_area_text {

  function option_definition() {
    $options = parent::option_definition();
    unset($options['format']);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['label']['#type'] = 'hidden';
    $form['tokenize']['#type'] = 'hidden';
    $form['token_help']['#type'] = 'hidden';

    // Alter the form element, to be a regular textfield.
    $form['content']['#title'] = 'Mapbox ID';
    $form['content']['#type'] = 'textfield';
  }

  // Empty, so we don't inherit options_submit from the parent.
  function options_submit(&$form, &$form_state) {
  }

  function render($empty = FALSE) {
    if (!$empty || !empty($this->options['empty'])) {
      return $this->render_mapbox($this->options['content']);
    }

    return '';
  }

  /**
   * Render a text area with filter_xss_admin.
   */
  function render_mapbox($value) {
    if ($value) {
      return mapbox_bridge_render_map($this->sanitize_value($value, 'xss_admin'), 'views');
    }
  }

}