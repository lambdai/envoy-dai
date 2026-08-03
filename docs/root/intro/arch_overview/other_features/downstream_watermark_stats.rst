.. _arch_overview_downstream_watermark_stats:

Downstream watermark stats
==========================

The :ref:`downstream watermark stats filter
<config_network_filters_downstream_watermark_stats>` tracks whether each downstream connection is
currently above the write buffer high watermark and emits the state as a gauge. Configure a distinct
``stat_prefix`` when the metric needs dimensions that can be extracted by Envoy's stats tag
configuration.
