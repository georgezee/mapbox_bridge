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
    $options['mapbox_mapping_field'] = array('default' => '');
    $options['mapping'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    drupal_add_css(drupal_get_path('module', 'mapbox_views') . '/css/mapbox_views.css');

    $form['cache'] = false;
    $form['label']['#type'] = 'hidden';

    $form['mapbox_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Mapbox ID'),
      '#description' => t('Enter the ID of your Mapbox project: https://www.mapbox.com/projects/'),
      '#default_value' => $this->options['mapbox_id'],
      '#mandatory' => TRUE
    );

    $form['mapbox_field'] = array(
      '#type' => 'textfield',
      '#title' => t('Mapbox Geofield'),
      '#description' => t('Geofield which contains the coordinates'),
      '#default_value' => $this->options['mapbox_field'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/geofield',
      '#mandatory' => TRUE
    );

    $form['mapbox_mapping_field'] = array(
      '#type' => 'textfield',
      '#title' => t('Term reference field'),
      '#description' => t('Field containing terms used for mapping.'),
      '#default_value' => $this->options['mapbox_mapping_field'],
      '#autocomplete_path' => 'mapbox_views_field_matching/autocomplete/taxonomy_term_reference',
    );

    // Count the existing mappings or reset to 1 when new.
    if (!isset($form_state['view']->mapping) && count($this->options['mapping']) > 0) {
      $form_state['view']->mapping = count($this->options['mapping']) - 1;
    }

    $form['mapping'] = array(
      '#title' => t('Marker Mappings'),
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#attributes' => array('class' => array('mapbox-views', 'mapbox-views-mapping-wrapper')),
      '#states' => array(
        'visible' => array(
          ':input[name="options[mapbox_mapping_field]"]' => array('filled' => TRUE),
        ),
      ),
    );

    if ($form_state['view']->mapping !== false) {
      for ($i = 0; $i <= $form_state['view']->mapping; $i++) {

        $form['mapping'][$i] = array(
          '#type' => 'container',
          '#tree' => TRUE,
          '#prefix' => '<div id="mapping-' . $i . '" class="mapping">',
          '#suffix' => '</div>',
        );

        $form['mapping'][$i]['marker_type'] = array(
          '#title' => t('Marker Type'),
          '#type' => 'textfield',
          '#default_value' => $this->options['mapping'][$i]['marker_type'],
          '#description' => t('The name of the marker type, Mapbox defaults like "bus, ect." can be used or custom ones which can be defined inside Mapbox Studio.'),
        );

        $form['mapping'][$i]['taxonomy_term'] = array(
          '#title' => t('Taxonomy Term'),
          '#type' => 'textfield',
          '#autocomplete_path' => 'taxonomy/autocomplete/field_office_type',
          '#default_value' => $this->options['mapping'][$i]['taxonomy_term'],
          '#description' => t('Taxonomy term which decides to use the marker type.'),
        );

        if ($i > 0) {
          $form['mapping'][$i]['remove_mapping'] = array(
            '#type' => 'submit',
            '#value' => t('Remove mapping'),
            '#name' => $i,
            '#ajax' => array(
              'path' => views_ui_build_form_url($form_state),
            ),
            '#submit' => array('mapbox_views_ajax_remove_mapping', 'views_ui_config_item_form_submit_temporary'),
            '#executes_submit_callback' => TRUE,
          );
        }

      }
    }

    $form['mapping']['add_mapping'] = array(
      '#type' => 'submit',
      '#value' => t('Add mapping'),
      '#ajax' => array(
        'wrapper' => 'mapping',
        'path' => views_ui_build_form_url($form_state),
      ),
      '#attributes' => array('class' => array('mapbox-views-add-mapping')),
      '#submit' => array('mapbox_views_ajax_add_mapping', 'views_ui_config_item_form_submit_temporary'),
      '#executes_submit_callback' => TRUE,
    );

    /*$form['mapping']['add_participant'] = array(
      '#type' => 'submit',
      '#value' => t('Add mapping'),
      '#ajax' => array(
        'callback' => 'mapbox_views_ajax_add_mapping',
        'wrapper' => 'mapping',
      ),
      '#attributes' => array('class' => array('mapbox-views-add-mapping')),
    );*/

    return $form;
  }

  function render($empty = FALSE) {
    if (!$empty || !empty($this->options['empty'])) {
      module_load_include('php', 'mapbox_views', 'views/mapbox_views_area_builder');
      $mapBuilder = new MapboxViewsAreaBuilder(
        $this->view,
        $this->options['mapbox_id'],
        $this->options['mapbox_field'],
        $this->options['mapbox_mapping_field']
      );
      try {
        return $mapBuilder->getMap($this->options['mapping']);
      }
      catch(Exception $e) {
        watchdog_exception('mapbox_views', $e);
      }
    }

    return '';
  }

}