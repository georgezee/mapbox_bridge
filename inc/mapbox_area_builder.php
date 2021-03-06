<?php


class MapboxAreaBuilder {

  /**
   * @var view
   *  View object
   */
  private $object = null;

  /**
   * @var string
   *  Mapbox ID
   */
  private $mapboxId = '';

  /**
   * @var string
   *  Geofield name
   */
  private $geofield = '';

  /**
   * @var string
   *  Marker type field (taxonomy reference field)
   */
  private $markerTypeField = '';

  /**
   * @var string
   *  Symbol name field (list, list_text)
   */
  private $fieldSymbolName = '';

  /**
   * @var string
   *  Icon upload field (image,file)
   */
  private $fieldSymbolIcon = '';

  /**
   * @var array
   *  Default icon specs
   */
  private $defaultIcon = array(
    'name' => '',
    'src' => '',
    'width' => 0,
    'height' => 0,
  );

  /**
   * @var string
   *  Marker's anchor position
   */
  private $markerAnchor = '';

  /**
   * Class constructor
   *
   * @param $object
   *  View or Node object
   * @param string $mapboxId
   *  Map Id
   * @param string $geofield
   *  Geofield name
   * @param string $markerTypeField
   *  Taxonomy reference field used to get marker type
   * @param string $symbolName
   *  Field name used to store symbol name when standar one is used
   * @param string $symbolIcon
   *  Field name used to upload custom icons
   * @param array $defaultIcon
   *  Default icon specifications (value keys: name, src, width, height)
   * @param array $markerAnchor
   *  Marker's anchor position
   */
  public function __construct($object, $mapboxId, $geofield, $markerTypeField = '', $legend = false, $symbolName = '', $symbolIcon = '', $max_zoom = 12, $popup = false, array $defaultIcon = array(), $markerAnchor = 'center_center', $filter = array('enabled' => false), $cluster = false, $proximity = false, $center = FALSE) {
    $this->object = $object;
    $this->mapboxId = $mapboxId;
    $this->geofield = $geofield;
    $this->markerTypeField = $markerTypeField;
    $this->legend = $legend;
    $this->fieldSymbolIcon = $symbolIcon;
    $this->fieldSymbolName = $symbolName;
    $this->max_zoom = $max_zoom;
    $this->popup = $popup;
    $this->translated_legend = array();
    $this->defaultIcon = $defaultIcon + $this->defaultIcon;
    $this->markerAnchor = $markerAnchor;
    $this->filter = $filter;
    $this->cluster = $cluster;
    $this->proximity = $proximity;
    $this->center = $center;
  }

  /**
   * Get the map
   *
   * @return string
   *
   * @throws \Exception
   * @throws \PDOException
   */
  public function getMap() {
    if (isset($this->object->nid)) {
      $allEntityIds = array($this->object->nid);
      $type = 'node';

      $mapMarkers = $this->extractMarkersInfo($allEntityIds);
      $mapMarkers = $this->processMarkersSymbol($mapMarkers);

    } else if (is_string($this->object)) {
      $mapMarkers = file_get_contents(url($this->object));
      $mapMarkers = json_decode($mapMarkers);
      $type = 'json';

      $mapMarkers = $this->processJsonMarkers($mapMarkers);
    } else {
      $allViewResults = $this->getAllViewResults();
      $allEntityIds = $this->extractEntityIds($allViewResults);
      $type = 'views';

      $mapMarkers = $this->extractMarkersInfo($allEntityIds);
      $mapMarkers = $this->processMarkersSymbol($mapMarkers);
    }

    if ($this->legend) {
      $mapMarkers = $this->extractLegendsInfo($mapMarkers);
    }

    return mapbox_bridge_render_map($this->mapboxId, $mapMarkers, $type, $this->legend, $this->max_zoom, $this->popup, $this->markerAnchor, $this->filter, $this->cluster, $this->proximity, $this->center);
  }

  /**
   * Return all results (ignore the pager)
   *
   * @return array
   *
   * @throws \Exception
   */
  private function getAllViewResults() {
    $tempView = views_get_view($this->object->name, TRUE);
    if (is_object($tempView)) {
      // Change pager settings to return all values
      $this->removeViewPaginationSettings($tempView, $this->object->current_display);

      // @todo: remove sort and useless fields

      $tempView->set_arguments($this->object->args);
      $tempView->set_display($this->object->current_display);
      $tempView->live_preview = TRUE; // Disables some caches
      $tempView->pre_execute();
      $tempView->execute();
      return $tempView->result;
    }
    else {
      throw new Exception('Could not fetch all location results from view!');
    }
  }

  /**
   * Remove pagination from a view display to make it return all values
   *
   * @param view $view
   * @param string $displayId
   */
  private function removeViewPaginationSettings(view $view, $displayId) {
    // Get pager settings, to override
    $pager = &$view->display['default']->display_options['pager'];
    if (isset($view->display[$displayId]->display_options['pager'])) {
      $pager = &$view->display[$displayId]->display_options['pager'];
    }

    $pager['type'] = 'none';
    $pager['options'] = array('offset' => 0);
  }

  /**
   * Extract entity ids from results array
   *
   * @param array $results
   *
   * @return array
   *  Node ids
   */
  private function extractEntityIds($results) {
    $nodeIds = array();

    foreach($results as $item) {
      // SearchApi and Database Query return object, don't accept anything else
      if (!is_object($item)) {
        continue;
      }

      if (isset($item->nid)) { // Database query result
        $nodeIds[] = $item->nid;
      }
      elseif (isset($item->entity)) { // SearchAPI result
        $nodeIds[] = $item->entity;
      }
    }

    return $nodeIds;
  }

  /**
   * Extract all pin position data (geofield and marker type) from entity ids
   *
   * @param array $entityIds
   * @return array
   *
   * @throws \PDOException
   */
  private function extractMarkersInfo($entityIds) {
    // Start with geofield data
    $q = db_select("field_data_{$this->geofield}", 'g');
    $q->addField('g', "{$this->geofield}_lat", 'lat');
    $q->addField('g', "{$this->geofield}_lon", 'lon');
    $q->addField('g', "entity_id", 'nid');

    $q->condition('g.entity_type', 'node');
    $q->condition('g.entity_id', $entityIds, 'IN');

    // Add the mapping field
    if ($this->markerTypeField && ($this->fieldSymbolName || $this->fieldSymbolIcon)) {
      $join_conditions = 'm.entity_type = g.entity_type AND m.entity_id = g.entity_id';
      $q->leftJoin("field_data_{$this->markerTypeField}", "m", $join_conditions);
      //$q->leftJoin('taxonomy_term_data', 'td', 'td.tid = m.' . "{$this->markerTypeField}_tid");

      if ($this->fieldSymbolName) {
        $q->leftJoin("field_data_{$this->fieldSymbolName}", "fsn", "fsn.entity_type = 'taxonomy_term' AND fsn.entity_id = m.{$this->markerTypeField}_target_id");
        $q->addField('fsn', "{$this->fieldSymbolName}_value", 'type');
      }
      else {
        $q->addExpression("''", 'symbolName');
      }

      if ($this->fieldSymbolIcon) {
        $q->leftJoin("field_data_{$this->fieldSymbolIcon}", "fsi", "fsi.entity_type = 'taxonomy_term' AND fsi.entity_id = m.{$this->markerTypeField}_target_id");
        $q->leftJoin("file_managed", "fm", "fm.fid = fsi.{$this->fieldSymbolIcon}_fid");
        $q->addField("fm", "uri", "icon");
        $q->addField("fsi", "{$this->fieldSymbolIcon}_width", "iconWidth");
        $q->addField("fsi", "{$this->fieldSymbolIcon}_height", "iconHeight");
      }

      $q->leftJoin("taxonomy_term_data", "ttd", "ttd.tid = m.{$this->markerTypeField}_target_id");
      $q->addField('ttd', 'name', 'name');
      $q->addField('ttd', 'tid', 'tid');
    }
    elseif ($this->defaultIcon['src']) {
      $q->addExpression("'custom'", 'type');
      $q->addExpression("'custom'", 'name');
      $q->addExpression("0", 'tid');
      $q->addExpression("'{$this->defaultIcon['src']}'", 'icon');
      $q->addExpression("'{$this->defaultIcon['width']}'", 'iconWidth');
      $q->addExpression("'{$this->defaultIcon['height']}'", 'iconHeight');
    }
    else {
      $q->addExpression("'" . variable_get('mapbox_default_marker_name', 'marker') ."'", 'type');
    }

    return $q->execute()->fetchAll();
  }

  private function processMarkersSymbol(array $markers) {
    foreach($markers as $marker) {
      if (!isset($marker->type)) {
        $marker->type = '';
      }

      $marker->icon = isset($marker->icon) ? file_create_url($marker->icon) : '';
    }
    return $markers;
  }

  private function processJsonMarkers($markers) {
    foreach($markers as $marker) {
      if (!isset($marker->type)) {
        $marker->type = '';
      }

      $marker->icon = file_create_url($this->defaultIcon['src']);
      $marker->iconWidth = $this->defaultIcon['width'];
      $marker->iconHeight = $this->defaultIcon['height'];
    }

    return $markers;
  }

  private function extractLegendsInfo(array $markers) {
    foreach($markers as $marker) {
      if (!isset($this->translated_legend[$marker->tid])) {
        $this->getLegendTranslation($marker->tid);
      }

      $marker->name = $this->translated_legend[$marker->tid];
    }
    return $markers;
  }

  private function getLegendTranslation($tid) {
    global $language;

    $q = db_select('field_data_name_field', 'fdnf');

    $q->addField('fdnf', 'name_field_value', 'translated_name');
    $q->addField('fdnf', 'language', 'language');
    $q->condition('fdnf.entity_type', 'taxonomy_term');
    $q->condition('fdnf.entity_id', $tid);

    $results = $q->execute()->fetchAll();

    foreach ($results as $result) {
      if ($result->language == $language->language) {
        $this->translated_legend[$tid] = $result->translated_name;
      } else if (!isset($this->translated_legend[$tid]) && ($result->language == LANGUAGE_NONE || $result->language == 'en')) {
        $this->translated_legend[$tid] = $result->translated_name;
      }
    }
  }
}