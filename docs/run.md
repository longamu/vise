mpirun -np 8 compute_index_v2 trainDescs ox5k /home/tlm/dset/vise/ox5k_100/search_engine_config.txt
mpirun -np 8 python /home/tlm/dev/vise/src/search_engine/relja_retrival/src/v2/indexing/compute_clusters.py ox5k /home/tlm/dset/vise/ox5k_100/search_engine_config.txt 10
mpirun -np 8 compute_index_v2 trainAssign ox5k /home/tlm/dset/vise/ox5k_100/search_engine_config.txt
mpirun -np 8 compute_index_v2 trainHamm ox5k /home/tlm/dset/vise/ox5k_100/search_engine_config.txt
mpirun -np 8 compute_index_v2 index ox5k /home/tlm/dset/vise/ox5k_100/search_engine_config.txt

LD_LIBRARY_PATH="/home/tlm/deps/vise/lib/lib" ./api_v2 9999 ox5k /home/tlm/dset/vise/ox5k_100/search_engine_config.txt

python webserver.py 8080 ox5k1 9999 0 /home/tlm/dset/vise/ox5k_100/search_engine_config.txt


curl -X POST localhost:9973/vise/repo/ox5k/1/add_image?filename="all_souls_00001.jpg"


