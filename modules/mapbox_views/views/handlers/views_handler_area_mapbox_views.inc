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
    $options['mapbox_marker'] = array('default' => array('geo' => '', 'type' => 'none', 'type_field' => '', 'legend' => '', 'marker_anchor' => 'center_center', 'cluster' => '', 'proximity' => ''));
    $options['mapbox_popup'] = array('default' => array('popup' => FALSE, 'popup_viewmode' => 'teaser'));
    $options['mapbox_type'] = array('default' => array('name' => '', 'icon' => '', 'custom_icon_src' => '', 'custom_icon_width' => '', 'custom_icon_height' => ''));
    $options['mapbox_max_zoom'] = array('default' => 12);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    drupal_add_css(drupal_get_path('module', 'mapbox_views') . '/css/mapbox_views.css');
    $form['cache'] = false;
    $form['label']['#type'] = 'hidden';

    $form = $this->formMapGeneralSettings($form);
    $form = $this->formMarkersSettings($form);
    $form = $this->formMarkersPopupSettings($form);
    $form = $this->formMarkersIconSettings($form);

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
          $this->options['mapbox_marker']['type_field'],
          $this->options['mapbox_marker']['legend'],
          $this->options['mapbox_type']['name'],
          $this->options['mapbox_type']['icon'],
          $this->options['mapbox_max_zoom'],
          array(
            'enabled' => $this->options['mapbox_popup']['popup'],
            'popup_viewmode' => $this->options['mapbox_popup']['popup_viewmode']
          ),
          array(
            'src' => $this->options['mapbox_type']['custom_icon_src'],
            'width' => $this->options['mapbox_type']['custom_icon_width'],
            'height' => $this->options['mapbox_type']['custom_icon_height'],
          ),
          $this->options['mapbox_marker']['marker_anchor'],
          array('enabled' => false),
          $this->options['mapbox_marker']['cluster'],
          $this->options['mapbox_marker']['proximity']
        );
        return $mapBuilder->getMap();
      }
      catch(Exception $e) {
        watchdog_exception('mapbox_views', $e);
      }
    }

    return '';
  }

  /**
   * Add map's general settings
   *
   * @param array $form
   * @return array
   */
  private function formMapGeneralSettings(&$form) {
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
    return $form;
  }

  /**
   * Add markers settings
   *
   * @param array $form
   * @return array
   */
  private function formMarkersSettings(&$form) {
    $form['mapbox_marker'] = array(
      '#type' => 'fieldset',
      '#title' => t('Markers'),
      '#description' => t('Fields used to get marker\'s basic information'),
      //'#collapsible' => TRUE,
    );

    $form['mapbox_marker']['geo'] = array(
      '#type' => 'textfield',
      '#title' => t('Coordinates (geofield)'),
      '#description' => t('Name of the field which contains coordinates (field type: geofield)'),
      '#default_value' => $this->options['mapbox_marker']['geo'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/node/geofield',
      '#mandatory' => TRUE
    );

    $form['mapbox_marker']['type'] = array(
      '#type' => 'select',
      '#title' => t('Marker type'),
      '#options' => array(
        'none' => t('No type'),
        'taxonomy' => t('Taxonomy term'),
      ),
      '#default_value' => $this->options['mapbox_marker']['type'],
    );

    $form['mapbox_marker']['type_field'] = array(
      '#type' => 'textfield',
      '#title' => t('Marker type field'),
      '#description' => t('Field used to determine marker type and symbol (field type: taxonomy term reference)'),
      '#default_value' => $this->options['mapbox_marker']['type_field'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/node/entityreference/',
      '#states' => array(
        'invisible' => array(
          $this->formState('geo_field_empty'),
          $this->formState('marker_type_none'),
        )
      )
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
      '#default_value' => $this->options['mapbox_marker']['marker_anchor'],
    );

    $form['mapbox_marker']['legend'] = array(
      '#type' => 'checkbox',
      '#title' => t('Mapbox Legend'),
      '#description' => t('Will show a legend below the map with all marker type'),
      '#default_value' => $this->options['mapbox_marker']['legend'],
      '#mandatory' => TRUE,
      '#states' => array(
        'invisible' => array(
          $this->formState('geo_field_empty'),
          $this->formState('marker_type_none'),
        ),
      )
    );

    $form['mapbox_marker']['cluster'] = array(
      '#type' => 'checkbox',
      '#title' => t('Cluster'),
      '#description' => t('Enable clustering, example: !link', array('!link' => l(t('here'), 'https://www.mapbox.com/mapbox.js/example/v1.0.0/leaflet-markercluster/', array('attributes' => array('target' => '_blank'))))),
      '#default_value' => $this->options['mapbox_marker']['cluster'],
      '#mandatory' => TRUE
    );

    $form['mapbox_marker']['proximity'] = array(
      '#type' => 'checkbox',
      '#title' => t('Proximity Search'),
      '#description' => t('Enables the proximity search feeature, example: !link', array('!link' => l(t('here'), 'https://www.mapbox.com/mapbox.js/example/v1.0.0/geocoding-auto/', array('attributes' => array('target' => '_blank'))))),
      '#default_value' => $this->options['mapbox_marker']['proximity'],
      '#mandatory' => TRUE
    );

    return $form;
  }

  /**
   * @return array
   */
  private function entityViewModesOptions() {
    $view_modes = variable_get('entity_view_modes', array());
    $view_mode_options = array();

    foreach ($view_modes as $type => $view_mode) {
      foreach ($view_mode as $key => $mode) {
        $view_mode_options[$type][$key] = $mode['label'];
      }
    }
    return $view_mode_options;
  }

  /**
   * Markes icon settings
   *
   * @param array $form
   * @return array
   */
  private function formMarkersIconSettings(&$form) {
    $form['mapbox_type'] = array(
      '#type' => 'fieldset',
      '#title' => t('Marker style'),
      '#description' => t('Fields used to determine marker\'s type symbol'),
      '#states' => array(
        'invisible' => array($this->formState('geo_field_empty')),
      ),
    );
    $form['mapbox_type']['custom_icon_src'] = array(
      '#type' => 'textfield',
      '#title' => t('Local path'),
      '#description' => t('Path to an image that will be used as a marker pin. E.g.: sites/all/themes/omega/pin'),
      '#default_value' => $this->options['mapbox_type']['custom_icon_src'],
      '#states' => array(
        'invisible' => array(
          $this->formState('marker_type_taxonomy'),
        )
      )
    );
    $form['mapbox_type']['custom_icon_width'] = array(
      '#type' => 'textfield',
      '#title' => t('Icon width'),
      '#description' => t('Icon width in pixels (needed for positioning)'),
      '#default_value' => $this->options['mapbox_type']['custom_icon_width'],
      '#states' => array(
        'invisible' => array(
          $this->formState('marker_type_taxonomy'),
        )
      )
    );
    $form['mapbox_type']['custom_icon_height'] = array(
      '#type' => 'textfield',
      '#title' => t('Icon width'),
      '#description' => t('Icon height in pixels (needed for positioning)'),
      '#default_value' => $this->options['mapbox_type']['custom_icon_height'],
      '#states' => array(
        'invisible' => array(
          $this->formState('marker_type_taxonomy'),
        )
      )
    );
    $form['mapbox_type']['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name field'),
      '#description' => t('Field used to get standard symbol name (e.g.: bus, heart etc) for the marker (field types: text, list_text)'),
      '#default_value' => $this->options['mapbox_type']['name'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term/list_text,text',
      '#states' => array(
        'invisible' => array(
          $this->formState('marker_type_none'),
        ),
      )
    );
    $form['mapbox_type']['icon'] = array(
      '#type' => 'textfield',
      '#title' => t('Icon file field'),
      '#description' => t('Field used to get custom icon file (field types: image, file)'),
      '#default_value' => $this->options['mapbox_type']['icon'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term/image',
      '#states' => array(
        'invisible' => array(
          $this->formState('marker_type_none'),
        ),
      )
    );
    return $form;
  }

  /**
   * Popup settings for map markers
   *
   * @param array $form
   * @return array
   */
  private function formMarkersPopupSettings($form) {
    $form['mapbox_popup'] = array(
      '#type' => 'fieldset',
      '#title' => t('Popup settings'),
      '#description' => t('Marker click popup settings'),
      '#states' => array(
        'invisible' => array(
          $this->formState('geo_field_empty'),
        )
      )
    );
    $form['mapbox_popup']['popup'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable popups'),
      '#description' => t('Show a popup when clicking the marker'),
      '#default_value' => $this->options['mapbox_popup']['popup'],
    );
    $form['mapbox_popup']['popup_viewmode'] = array(
      '#type' => 'select',
      '#title' => t('Viewmode'),
      '#options' => $this->entityViewModesOptions(),
      '#description' => t('Viewmode to be used when displaying the popup'),
      '#default_value' => $this->options['mapbox_popup']['popup_viewmode'],
      '#states' => array(
        'invisible' => array(
          $this->formState('popup_disabled'),
        )
      )
    );
    return $form;
  }

  /**
   * Form visibility states
   *
   * @param $name
   *  Name of the remote condition to return
   *
   * @see drupal_process_states()
   */
  private function formState($name) {
    $states = array(
      'geo_field_empty' => array(':input[name="options[mapbox_marker][geo]"]' => array('empty' => TRUE)),
      'marker_type_none' => array(':input[name="options[mapbox_marker][type]"]' => array('value' => 'none')),
      'marker_type_taxonomy' => array(':input[name="options[mapbox_marker][type]"]' => array('value' => 'taxonomy')),
      'popup_disabled' => array(':input[name="options[mapbox_popup][popup]"]' => array('checked' => FALSE)),
    );
    return $states[$name];
  }

}