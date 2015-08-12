<?php

/**
 * Helper class that contains the implementation of the field hooks.
 */

class MapboxField {

  /**
   * Implements the hook_field_info().
   */
  public static function fieldInfo() {
    return array(
      'mapbox_bridge' => array(
        'label' => t('Mapbox'),
        'description' => t('Stores the settings for a mapbox field.'),
        'settings' => array(),
        'instance_settings' => array(),
        'default_widget' => 'mapbox_bridge',
        'default_formatter' => 'mapbox_bridge_default',
      ),
    );
  }

  /**
   * Implements the hook_widget_info().
   */
  public static function widgetInfo() {
    return array(
      'mapbox_bridge' => array(
        'label' => 'Mapbox',
        'field types' => array('mapbox_bridge'),
        'multiple values' => FIELD_BEHAVIOR_DEFAULT,
      ),
    );
  }

  /**
   * Implements hook_field_formatter_info().
   */
  public static function formatterInfo() {
    return array(
      'mapbox_bridge_default' => array(
        'label' => t('Mapbox (default)'),
        'field types' => array('mapbox_bridge'),
        'multiple values' => FIELD_BEHAVIOR_DEFAULT,
      ),
    );
  }

  /**
   * Implements hook_field_widget_form().
   */
  public static function widgetForm(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
    $element += array(
      '#type' => $instance['widget']['type'],
      '#default_value' => isset($items[$delta]) ? $items[$delta] : '',
    );
    return $element;
  }

  /**
   * Process handler for the Mapbox field.
   */
  public static function fieldProcess($element, $form_state, $complete_form) {
    // On the field edit form, we do not really want to have the fields required,
    // so the $make_required variable will be TRUE in all the cases when we are
    // not on the field edit form.
    // @todo: the problem here is when you have multiple values, then the last one
    // will also require input. Disable for the moment.
    //$make_required = ($complete_form['#form_id'] == 'field_ui_field_edit_form') ? FALSE : TRUE;
    $make_required = FALSE;
    // The mapbox id.
    $t_args = array('!mapbox' => l('www.mapbox.com/projects', 'https://www.mapbox.com/projects/', array('attributes' => array('target' => '_blank'))));
    $element['mapbox_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Mapbox ID'),
      '#description' => t('ID of your Mapbox project: !mapbox', $t_args),
      '#default_value' => !empty($element['#value']['mapbox_id']) ? $element['#value']['mapbox_id'] : '',
      '#required' => $make_required,
    );

    // The data source option.
    $data_source_options = array('' => t('--Select data source type--'));
    $data_source_types = _mapbox_bridge_data_source_types();
    foreach ($data_source_types as $key => $data_source) {
      $data_source_options[$key] = $data_source['title'];
    }
    $data_source_id = user_password(20);
    $data_source_type = !empty($element['#value']['data_source_type']) ? $element['#value']['data_source_type'] : '';
    $element['data_source_type'] = array(
      '#type' => 'select',
      '#title' => t('Data source type'),
      '#description' => t('Please select the type of the data source for this map.'),
      '#options' => $data_source_options,
      '#default_value' => $data_source_type,
      '#required' => $make_required,
      '#ajax' => array(
        'callback' => 'mapbox_bridge_data_source_type_ajax_callback',
        'wrapper' => $data_source_id,
        'method' => 'html',
      ),
    );

    $element['data_source_id'] = array(
      '#type' => 'item',
    );
    // Depending on which data source type we have selected, we will display a
    // different element for the source id.
    // @todo: this works, but there is an issue that when you change the data
    // source type from views <=> current node an error message appears. Has to be
    // fixed.
    if (!empty($data_source_types[$data_source_type]) && function_exists($data_source_types[$data_source_type]['element_callback'])) {
      $element['data_source_id'] = $data_source_types[$data_source_type]['element_callback']($element);
    }
    $element['data_source_id']['#prefix'] = '<div id="' . $data_source_id . '">';
    $element['data_source_id']['#suffix'] = '</div>';

    // The zoom level.
    $element['max_zoom_level'] = array(
      '#type' => 'select',
      '#title' => t('Max zoom level'),
      '#description' => t('The maximum level of zoom (the higher the level, the closer)'),
      '#default_value' => !empty($element['#value']['max_zoom_level']) ? $element['#value']['max_zoom_level'] : 12,
      '#options' => drupal_map_assoc(range(1, 18)),
    );
    // The marker popup flag.
    $element['marker_popup'] = array(
      '#type' => 'checkbox',
      '#title' => t('Popup'),
      '#description' => t('Show a popup when clicking the marker'),
      '#default_value' => !empty($element['#value']['marker_popup']) ? $element['#value']['marker_popup'] : FALSE,
    );

    // The view mode of the marker popup.
    $view_modes = variable_get('entity_view_modes', array());
    $view_mode_options = array('' => t('--Select view mode--'));

    foreach ($view_modes as $type => $view_mode) {
      foreach ($view_mode as $key => $mode) {
        $view_mode_options[$type][$key] = $mode['label'];
      }
    }
    // add the drupal default Node "Teaser" and "Default" viewmodes.
    $view_mode_options['node']['node_default'] = t('Default');
    $view_mode_options['node']['node_teaser'] = t('Teaser');
    $element['marker_view_mode'] = array(
      '#type' => 'select',
      '#title' => t('View mode'),
      '#options' => $view_mode_options,
      '#description' => t('View mode to be used when displaying the popup.'),
      '#default_value' => !empty($element['#value']['marker_view_mode']) ? $element['#value']['marker_view_mode'] : '',
      '#states' => array(
        'invisible' => array(
          'input[name="' . $element['#name'] .'[marker_popup]"]' => array('checked' => FALSE),
        ),
      ),
    );

    // The legend.
    $element['marker_legend'] = array(
      '#type' => 'checkbox',
      '#title' => t('Mapbox Legend'),
      '#description' => t('Will show a legend below the map with all marker type'),
      '#default_value' => !empty($element['#value']['marker_legend']) ? $element['#value']['marker_legend'] : FALSE,
    );

    // The cluster.
    $element['marker_cluster'] = array(
      '#type' => 'checkbox',
      '#title' => t('Cluster'),
      '#description' => t('Enable clustering, example: !link', array('!link' => l(t('here'), 'https://www.mapbox.com/mapbox.js/example/v1.0.0/leaflet-markercluster/', array('attributes' => array('target' => '_blank'))))),
      '#default_value' => !empty($element['#value']['marker_cluster']) ? $element['#value']['marker_cluster'] : FALSE,
    );

    // The proximity search.
    $element['marker_proximity_search'] = array(
      '#type' => 'checkbox',
      '#title' => t('Proximity Search'),
      '#description' => t('Enables the proximity search feature, example: !link', array('!link' => l(t('here'), 'https://www.mapbox.com/mapbox.js/example/v1.0.0/geocoding-auto/', array('attributes' => array('target' => '_blank'))))),
      '#default_value' => !empty($element['#value']['marker_proximity_search']) ? $element['#value']['marker_proximity_search'] : FALSE,
    );

    // The zoom level.
    $element['marker_anchor'] = array(
      '#type' => 'select',
      '#title' => t('Marker anchor'),
      '#description' => t('What is considered to be the "tip" of the marker icon.'),
      '#default_value' => !empty($element['#value']['marker_anchor']) ? $element['#value']['marker_anchor'] : '',
      '#options' => mapbox_bridge_marker_anchor_options(),
    );

    // The filter.
    $element['marker_filter'] = array(
      '#type' => 'checkbox',
      '#title' => t('Mapbox Filter'),
      '#description' => t('Filter markers based on the "filter" attribute within the JSON'),
      '#default_value' => !empty($element['#value']['marker_filter']) ? $element['#value']['marker_filter'] : FALSE,
    );

    $element['marker_filter_fields'] = array(
      '#type' => 'textfield',
      '#title' => t('Filter fields'),
      '#description' => t('Name of the field that acts as a filter from the json and how it should be displayed. E.g. country:select (allowed: select, checkbox, radio) will render a select list with all the values from the "country" attribute inside the JSON, separate multiple fields by a comma.'),
      '#default_value' => !empty($element['#value']['marker_filter_fields']) ? $element['#value']['marker_filter_fields'] : '',
      '#states' => array(
        'invisible' => array(
          'input[name="' . $element['#name'] .'[marker_filter]"' => array('checked' => FALSE),
        ),
      ),
    );

    // The marker icon: src, width and height.
    $element['marker_icon_src'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom icon'),
      '#description' => t('Path to an image that will be used as a marker pin. E.g.: sites/all/themes/omega/pin'),
      '#default_value' => !empty($element['#value']['marker_icon_src']) ? $element['#value']['marker_icon_src'] : '',
    );

    $element['marker_icon_width'] = array(
      '#type' => 'textfield',
      '#title' => t('Icon width'),
      '#size' => 3,
      '#description' => t('Icon width in pixels (needed for positioning)'),
      '#default_value' => !empty($element['#value']['marker_icon_width']) ? $element['#value']['marker_icon_width'] : 0,
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $element['#name'] .'[marker_icon_src]"]' => array('empty' => TRUE),
        )
      )
    );

    $element['marker_icon_height'] = array(
      '#type' => 'textfield',
      '#title' => t('Icon height'),
      '#size' => 3,
      '#description' => t('Icon height in pixels (needed for positioning)'),
      '#default_value' => !empty($element['#value']['marker_icon_height']) ? $element['#value']['marker_icon_height'] : 0,
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $element['#name'] .'[marker_icon_src]"]' => array('empty' => TRUE),
        )
      )
    );

    return $element;
  }

  /**
   * Implements hook_field_validate().
   *
   * @todo: implement this.
   */
  public static function fieldValidate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {

  }

  /**
   * Implements hook_field_formatter_view().
   */
  public static function formatterView($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
    $elements = array();
    module_load_include('php', 'mapbox_bridge', 'inc/mapbox_area_builder');
    foreach ($items as $delta => $item) {
      try {
        $mapbox_source = mapbox_bridge_get_data_source($item, $entity_type, $entity);
        $filter_fields = !empty($item['marker_filter_fields'])? explode(',', $item['marker_filter_fields']) : NULL;
        // In order to be able to have the filters translated, we need to do it
        // in php, not in javascript. The Drupal.t() function does not work on
        // variables, so something like: "Drupal.t(filter)" will not work.
        if (!empty($filter_fields)) {
          foreach ($filter_fields as $filter_field_key => $filter_field) {
            $words = explode(':', $filter_field);
            // We do this only if we have exactly 2 words, otherwise the string
            // is already considered to be translated.
            if (count($words) == 2) {
              $words[2] = t(trim($words[0]), array(), array('context' => 'mapbox_filter'));
              $filter_fields[$filter_field_key] = implode(':', $words);
            }
          }
        }
        $mapBuilder = new MapboxAreaBuilder(
          $mapbox_source,
          $item['mapbox_id'],
          '', // here we should put the geofield, if any.
          NULL,
          !empty($item['marker_legend']) ? TRUE : FALSE,
          '', // The symbol name.
          '',
          !empty($item['max_zoom_level']) ? $item['max_zoom_level'] : 12,
          array(
            'enabled' => !empty($item['marker_popup']) ? TRUE : FALSE,
            'popup_viewmode' => !empty($item['marker_view_mode']) ? str_replace('node_', '', $item['marker_view_mode']) : NULL),
          array(
            'src' => !empty($item['marker_icon_src']) ? $item['marker_icon_src'] : NULL,
            'width' => !empty($item['marker_icon_width']) ? $item['marker_icon_width'] : NULL,
            'height' => !empty($item['marker_icon_height']) ? $item['marker_icon_height'] : NULL,
          ),
          !empty($item['marker_anchor']) ? $item['marker_anchor'] : 'center_center',
          array(
            'enabled' => !empty($item['marker_filter']) ? TRUE : FALSE,
            'filter_fields' => $filter_fields,
          ),
          !empty($item['marker_cluster']) ? TRUE : FALSE,
          !empty($item['marker_proximity_search']) ? TRUE : FALSE
        );

        $elements[$delta] = array(
          '#markup' => $mapBuilder->getMap(),
          '#element' => $item,
          '#field' => $instance,
          '#display' => $display,
        );
      }
      catch(Exception $e) {
        watchdog_exception('mapbox_views', $e);
      }
    }
    return $elements;
  }

  /**
   * Implements hook_field_is_empty().
   */
  public static function isEmpty($item, $field) {
    return empty($item['mapbox_id']);
  }
}
