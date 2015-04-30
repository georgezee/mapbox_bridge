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
   */
  public function __construct(view $view, $mapboxId, $geofield, $markerTypeField = '') {
    $this->view = $view;
    $this->mapboxId = $mapboxId;
    $this->geofield = $geofield;
    $this->markerTypeField = $markerTypeField;
  }

  /**
   * Get the map
   *
   * @param array $markersTypeRemap
   *  Remap markertypes value returned from database
   *
   * @return string
   *
   * @throws \Exception
   * @throws \PDOException
   */
  public function getMap($markersTypeRemap = array()) {
    $allViewResults = $this->getAllViewResults();
    $allEntityIds = $this->extractEntityIdsFromViewResults($allViewResults);
    $mapPins = $this->extractPinsPositionFromEntities($allEntityIds);
    if ($markersTypeRemap !== array()) {
      $this->remapPinTypes($mapPins, $markersTypeRemap);
    }
    return mapbox_bridge_render_map($this->mapboxId, $mapPins, 'views');
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
  private function extractEntityIdsFromViewResults($results) {
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
  private function extractPinsPositionFromEntities($entityIds) {
    // Start with geofield data
    $q = db_select("field_data_{$this->geofield}", 'g');
    $q->addField('g', "{$this->geofield}_lat", 'lat');
    $q->addField('g', "{$this->geofield}_lon", 'lon');

    $q->condition('g.entity_type', 'node');
    $q->condition('g.entity_id', $entityIds, 'IN');

    // Add the mapping field
    if ($this->markerTypeField) {
      $join_conditions = 'm.entity_type = g.entity_type AND m.entity_id = g.entity_id';
      $q->leftJoin("field_data_{$this->markerTypeField}", 'm', $join_conditions);
      $q->leftJoin('taxonomy_term_data', 'td', 'td.tid = m.' . "{$this->markerTypeField}_tid");
      $q->addField('td', 'name', 'type');
      // @todo: left join marker type field from term entity instead of tid
      // ...
    }
    else {
      $q->addExpression('NULL', 'type');
    }

    return $q->execute()->fetchAll();
  }

  /**
   * Remap pin types
   *
   * @param array $mapPins
   * @param array $markersTypeRemap
   */
  private function remapPinTypes(array &$mapPins, array $markersTypeRemap) {
    $keyedRemapArray = array();
    foreach($markersTypeRemap as $remap) {
      if (!empty($remap['marker_type']) && !empty($remap['taxonomy_term'])) {
        $keyedRemapArray[$remap['taxonomy_term']] = $remap['marker_type'];
      }
    }

    foreach($mapPins as $i => $pin) {
      if (isset($keyedRemapArray[$pin->type])) {
        $mapPins[$i]->type = $keyedRemapArray[$pin->type];
      }
      else {
        $mapPins[$i]->type = '';
      }
    }
  }

}