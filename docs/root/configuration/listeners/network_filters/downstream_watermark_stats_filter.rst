.. _config_network_filters_downstream_watermark_stats:

Downstream watermark stats
==========================

* Downstream watermark stats :ref:`architecture overview <arch_overview_downstream_watermark_stats>`
* :ref:`v3 API reference <envoy_v3_api_msg_extensions.filters.network.downstream_watermark_stats.v3.DownstreamWatermarkStats>`

The downstream watermark stats filter emits downstream connection write-buffer watermark state.
The filter does not modify network traffic.

Statistics
----------

The downstream watermark stats filter emits statistics under the configured ``stat_prefix``.

.. csv-table::
  :header: Name, Type, Description
  :widths: 1, 1, 2

  downstream_cx_above_high_watermark, Gauge, Total active connections currently above the write buffer high watermark
