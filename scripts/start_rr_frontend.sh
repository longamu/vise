#!/bin/sh
source /ssd/adutta/build_deps/python_virtualenv/vise/bin/activate
python /data/adutta/vggdemo/vise/src/ui/web/webserver2.py 9981 BL_bindings 65001 /ssd/adutta/data/vggdemo/BL_bindings/vise_search_engine_data/training_data/vise_config.cfg true /BL_bindings

