<?php


class MapboxViewsAreaBuilder {

  /**
   * @var view
   *  View object
   */
  private $view = null;

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
   * Class constructor
   *
   * @param view $view
   *  View object
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
   */
  public function __construct(view $view, $mapboxId, $geofield, $markerTypeField = '', $legend = FALSE, $symbolName = '', $symbolIcon = '') {
    $this->view = $view;
    $this->mapboxId = $mapboxId;
    $this->geofield = $geofield;
    $this->markerTypeField = $markerTypeField;
    $this->legend = $legend;
    $this->fieldSymbolIcon = $symbolIcon;
    $this->fieldSymbolName = $symbolName;
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
    $allViewResults = $this->getAllViewResults();
    $allEntityIds = $this->extractEntityIds($allViewResults);
    $mapMarkers = $this->extractMarkersInfo($allEntityIds);
    $mapMarkers = $this->processMarkersSymbol($mapMarkers);
    return mapbox_bridge_render_map($this->mapboxId, $mapMarkers, 'views', $this->legend);
  }

  /**
   * Return all results (ignore the pager)
   *
   * @return array
   *
   * @throws \Exception
   */
  private function getAllViewResults() {
    $tempView = views_get_view($this->view->name, TRUE);
    if (is_object($tempView)) {
      // Change pager settings to return all values
      $this->removeViewPaginationSettings($tempView, $this->view->current_display);

      // @todo: remove sort and useless fields

      $tempView->set_arguments($this->view->args);
      $tempView->set_display($this->view->current_display);
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
      }

      $q->leftJoin("taxonomy_term_data", "ttd", "ttd.tid = m.{$this->markerTypeField}_target_id");
      $q->addField('ttd', 'name', 'name');
    }
    else {
      $q->addExpression('NULL', 'type');
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
}