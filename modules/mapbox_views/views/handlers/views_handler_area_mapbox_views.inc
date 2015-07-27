<?php

/**
 * Views area text custom handler.
 *
 * @ingroup views_area_handlers
 */
class views_handler_area_mapbox_views extends views_handler_area {

  /**
   * @var array
   *  Available zoom levels
   */
  private static $ZOOM_LEVELS = array(
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
  );

  /**
   * Handler name in view configuration page
   *
   * @return string
   */
  public function ui_name($short = false) {
    return empty($this->options['ui_name']) ? t('Mapbox map') : $this->options['ui_name'];
  }

  public function option_definition() {
    $options = parent::option_definition();
    unset($options['label']);
    $options['mapbox_id'] = array('default' => '');
    $options['mapbox_marker'] = array('default' => array('geo' => '', 'type' => '', 'legend' => ''));
    $options['mapbox_type'] = array('default' => array('name' => '', 'icon' => ''));
    $options['mapbox_max_zoom'] = array('default' => 12);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // prepare a list of viewmodes
    $view_modes = variable_get('entity_view_modes', array());
    $view_mode_options = array();

    foreach ($view_modes as $type => $view_mode) {
      foreach ($view_mode as $key => $mode) {
        $view_mode_options[$type][$key] = $mode['label'];
      }
    }

    drupal_add_css(drupal_get_path('module', 'mapbox_views') . '/css/mapbox_views.css');

    $form['cache'] = false;
    $form['label']['#type'] = 'hidden';

    $t_args = array('!mapbox' => l('www.mapbox.com/projects', 'https://www.mapbox.com/projects/', array('attributes' => array('target' => '_blank'))));
    $form['mapbox_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Mapbox ID'),
      '#description' => t('ID of your Mapbox project: !mapbox', $t_args),
      '#default_value' => $this->options['mapbox_id'],
      '#mandatory' => TRUE
    );
    $form['mapbox_max_zoom'] = array(
      '#type' => 'select',
      '#title' => t('Max zoom level'),
      '#description' => t('The maximum level of zoom (the higher the level, the closer)'),
      '#default_value' => $this->options['mapbox_max_zoom'] ? $this->options['mapbox_max_zoom'] : 12,
      '#options' => self::$ZOOM_LEVELS,
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
      '#default_value' => $this->options['mapbox_marker']['geo'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/node/geofield',
      '#mandatory' => TRUE
    );
    $form['mapbox_marker']['type'] = array(
      '#type' => 'textfield',
      '#title' => t('Type'),
      '#description' => t('Field used to determine marker type and symbol (field type: taxonomy term reference)'),
      '#default_value' => $this->options['mapbox_marker']['type'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/node/entityreference/',
    );
    $form['mapbox_marker']['legend'] = array(
      '#type' => 'checkbox',
      '#title' => t('Mapbox Legend'),
      '#description' => t('Will show a legend below the map with all marker type'),
      '#default_value' => $this->options['mapbox_marker']['legend'],
      '#mandatory' => TRUE
    );
    $form['mapbox_marker']['popup_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Popup settings'),
      '#collapsible' => TRUE,
    );
    $form['mapbox_marker']['popup_settings']['popup'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable popups'),
      '#description' => t('Show a popup when clicking the marker'),
      '#default_value' => $conf['popup'],
    );
    $form['mapbox_marker']['popup_settings']['popup_viewmode'] = array(
      '#type' => 'select',
      '#title' => t('Viewmode'),
      '#options' => $view_mode_options,
      '#description' => t('Viewmode to be used when displaying the popup'),
      '#default_value' => $conf['popup_viewmode'],
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
      '#default_value' => $this->options['mapbox_type']['name'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term/list_text,text',
    );
    $form['mapbox_type']['icon'] = array(
      '#type' => 'textfield',
      '#title' => t('Icon'),
      '#description' => t('Field used to get custom icon file (field types: image, file)'),
      '#default_value' => $this->options['mapbox_type']['icon'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term/image',
    );

    // Count the existing mappings or reset to 1 when new.
    if (!isset($form_state['view']->mapping) && count($this->options['mapping']) > 0) {
      $form_state['view']->mapping = count($this->options['mapping']) - 1;
    }

    return $form;
  }

  function render($empty = FALSE) {
    if (!$empty || !empty($this->options['empty'])) {
      module_load_include('php', 'mapbox_bridge', 'inc/mapbox_area_builder');
      try {
        $mapBuilder = new MapboxAreaBuilder(
          $this->view,
          $this->options['mapbox_id'],
          $this->options['mapbox_marker']['geo'],
          $this->options['mapbox_marker']['type'],
          $this->options['mapbox_marker']['legend'],
          $this->options['mapbox_type']['name'],
          $this->options['mapbox_type']['icon'],
          $this->options['mapbox_max_zoom'],
          array('enabled' => $this->options['mapbox_marker']['popup_settings']['popup'], 'popup_viewmode' => $this->options['mapbox_marker']['popup_settings']['popup_viewmode'])
        );
        return $mapBuilder->getMap();
      }
      catch(Exception $e) {
        watchdog_exception('mapbox_views', $e);
      }
    }

    return '';
  }

}