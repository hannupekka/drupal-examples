<?php
/**
 * Implementation of hook_drush_command().
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing each command.
 */
function drush_batch_example_drush_command() {
  $items = array();

  $items['list-all-node-titles'] = array(
    'description' => dt('Lists all node titles')
  );  

  return $items;
}

/**
 * Implements drush_COMMANDFILE_COMMANDNAME()
 */
function drush_drush_batch_example_list_all_node_titles() {
  $languages = array();
  // Get all languages in array, keyed by langcode
  foreach (language_list() as $lang) {
    $languages[$lang->language] = $lang->name;
  }
  // Ask which language nodes to list
  $langcode = drush_choice($languages, 'List node titles from which language?');

  $batch = array(
    'title' => t('Listing titles'),
    'operations' => array(
      array('drush_batch_example_list_all_node_titles_process', array($langcode))
    ),
    'finished' => 'drush_batch_example_list_all_node_titles_finish'
  );

  $batch['progressive'] = FALSE;
  batch_set($batch);
  drush_backend_batch_process();
}

/**
 * Batch processing
 */
function drush_batch_example_list_all_node_titles_process($langcode, &$context) {
  // Format context variables
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_nid'] = 0;
    $context['sandbox']['max'] = db_query("SELECT COUNT(n.nid) FROM {node} n WHERE n.language = :language", array(':language' => $langcode))->fetchField();
  }

  if (empty($context['sandbox']['max'])) {
    $context['finished'] = TRUE;
    return;
  }

  // Get 10 nodes at a time
  $result = db_query("SELECT n.nid, n.title FROM {node} n WHERE n.language = :language AND n.nid > :current_nid ORDER BY n.nid ASC LIMIT 10", array(
    ':language' => $langcode,
    ':current_nid' => $context['sandbox']['current_nid'],
  ))->fetchAll();

  // Process those fetched rows
  foreach ($result as $row) {
    $context['sandbox']['current_nid'] = $row->nid;
    $context['sandbox']['progress']++;
    $context['results'][] = $row->title;
    drush_log('Processing ' . $context['sandbox']['progress'] . ' of ' . $context['sandbox']['max'] . ': ' . $row->title, 'ok');
  }

  // Check if we are finished yet
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = ($context['sandbox']['progress'] >= $context['sandbox']['max']);
  }  
}

/**
 * Gets called when batch is finished
 */
function drush_batch_example_list_all_node_titles_finish($success, $results, $operations) {
  $count = count($results);
  drush_log("Finished {$count} nodes", 'ok');
}