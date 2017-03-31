# Performance Record

## ox5k
(time in minutes)

|          | trainDescs | cluster | trainAssign | trainHamm |   index |
|----------+------------+---------+-------------+-----------+---------|
| original |       5:50 |   48:21 |             |           | 2:56:32 |
|          |            |         |             |           |         |


```
tlm@0x02:~/dev/vise$ ls
Acknowledgements.md  Contributing.md  LICENSE                    vise_train.sh
allstart.sh          debug            README.md                  web_run.sh
api_run.sh           docs             README_relja_retrival.txt
build                forbuild         src
CHANGELOG            install_scripts  vise_demo.sh
tlm@0x02:~/dev/vise$ ./vise_train.sh 
tlm@0x02:~/dev/vise$ ./vise_train.sh 
Script to train the VGG Image Search Engine (VISE)
For more details, see www.robots.ox.ac.uk/~vgg/software/vise

Enter the location of images (default: ox5k): /data/datasets/vise/data/image/ox5k/oxc1
	VISE_ROOTDIR = /home/tlm/dev/vise
	Image location = /data/datasets/vise/data/image/ox5k/oxc1
	DATASET = oxc1
	VISE_DATADIR= /data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/
	imlist= /data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/imlist.txt
trainDescs : 
Thu 30 Mar 14:59:24 BST 2017
numProc= 1
numWorkerThreads = 8
trainDescsManager(descs): 2017-Mar-30 14:59:33 1 / 800000
trainDescsManager(descs): 2017-Mar-30 14:59:33 2 / 800000; time 00:00:00; left 00:02:54; avg 0.000218018 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 3 / 800000; time 00:00:00; left 00:01:55; avg 0.000144531 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 4 / 800000; time 00:00:00; left 00:01:30; avg 0.000113688 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 8 / 800000; time 00:00:00; left 00:00:44; avg 5.58733e-05 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 16 / 800000; time 00:00:00; left 00:00:23; avg 2.94596e-05 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 32 / 800000; time 00:00:00; left 00:00:12; avg 1.59715e-05 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 64 / 800000; time 00:00:00; left 00:00:07; avg 8.76194e-06 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 128 / 800000; time 00:00:00; left 00:00:03; avg 4.81168e-06 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 256 / 800000; time 00:00:00; left 00:00:02; avg 2.6348e-06 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 512 / 800000; time 00:00:00; left 00:00:01; avg 1.44239e-06 s
trainDescsManager(descs): 2017-Mar-30 14:59:33 1024 / 800000; time 00:00:00; left 00:00:00; avg 8.01631e-07 s
trainDescsManager(descs): 2017-Mar-30 14:59:35 2048 / 800000; time 00:00:02; left 00:17:56; avg 0.00134927 s
trainDescsManager(descs): 2017-Mar-30 14:59:35 4096 / 800000; time 00:00:02; left 00:08:56; avg 0.000674528 s
trainDescsManager(descs): 2017-Mar-30 14:59:38 8192 / 800000; time 00:00:05; left 00:08:18; avg 0.000629418 s
trainDescsManager(descs): 2017-Mar-30 14:59:38 16384 / 800000; time 00:00:05; left 00:04:06; avg 0.000314755 s
trainDescsManager(descs): 2017-Mar-30 14:59:46 32768 / 800000; time 00:00:13; left 00:05:21; avg 0.000419406 s
trainDescsManager(descs): 2017-Mar-30 15:00:03 65536 / 800000; time 00:00:29; left 00:05:34; avg 0.000455677 s
trainDescsManager(descs): 2017-Mar-30 15:00:04 80000 / 800000; time 00:00:31; left 00:04:39; avg 0.00038788 s
trainDescsManager(descs): 2017-Mar-30 15:00:27 131072 / 800000; time 00:00:54; left 00:04:39; avg 0.000417705 s
trainDescsManager(descs): 2017-Mar-30 15:00:38 160000 / 800000; time 00:01:04; left 00:04:19; avg 0.00040559 s
trainDescsManager(descs): 2017-Mar-30 15:01:12 240000 / 800000; time 00:01:38; left 00:03:50; avg 0.000412279 s
trainDescsManager(descs): 2017-Mar-30 15:01:18 262144 / 800000; time 00:01:45; left 00:03:35; avg 0.000400593 s
trainDescsManager(descs): 2017-Mar-30 15:01:49 320000 / 800000; time 00:02:16; left 00:03:24; avg 0.000425493 s
trainDescsManager(descs): 2017-Mar-30 15:02:14 400000 / 800000; time 00:02:41; left 00:02:41; avg 0.000404071 s
trainDescsManager(descs): 2017-Mar-30 15:02:46 480000 / 800000; time 00:03:13; left 00:02:08; avg 0.000402714 s
trainDescsManager(descs): 2017-Mar-30 15:03:09 524288 / 800000; time 00:03:36; left 00:01:54; avg 0.000413496 s
trainDescsManager(descs): 2017-Mar-30 15:03:21 560000 / 800000; time 00:03:48; left 00:01:37; avg 0.000407506 s
trainDescsManager(descs): 2017-Mar-30 15:03:48 640000 / 800000; time 00:04:15; left 00:01:03; avg 0.000398767 s
trainDescsManager(descs): 2017-Mar-30 15:04:18 720000 / 800000; time 00:04:45; left 00:00:31; avg 0.000396914 s
trainDescsManager(descs): 2017-Mar-30 15:04:59 800000 / 800000; time 00:05:25; left 00:00:00; avg 0.000407421 s
Thu 30 Mar 15:05:08 BST 2017

compute_clusters : 
Thu 30 Mar 15:05:08 BST 2017
Pypar (version 2.1.4) initialised MPI OK with 1 processors
Computing clusters:
Using a (800000 x 128) uint8 array for the datapoints
Using a (10000 x 128) float32 array for the clusters
Iteration 1, sse = 143190, mem = 41.07MB, took 98.74s
Iteration 2, sse = 102917, mem = 43.18MB, took 92.52s
Iteration 3, sse = 99744.9, mem = 48.07MB, took 93.71s
Iteration 4, sse = 98483.7, mem = 48.07MB, took 93.07s
Iteration 5, sse = 97785.6, mem = 48.07MB, took 95.63s
Iteration 6, sse = 97331.5, mem = 48.07MB, took 92.13s
Iteration 7, sse = 97016.9, mem = 48.07MB, took 91.89s
Iteration 8, sse = 96790.9, mem = 48.07MB, took 90.52s
Iteration 9, sse = 96601.4, mem = 48.07MB, took 93.39s
Iteration 10, sse = 96453.3, mem = 48.07MB, took 91.20s
Iteration 11, sse = 96339.7, mem = 48.07MB, took 92.21s
Iteration 12, sse = 96249, mem = 48.07MB, took 90.34s
Iteration 13, sse = 96164.1, mem = 48.07MB, took 91.61s
Iteration 14, sse = 96084.2, mem = 48.07MB, took 90.43s
Iteration 15, sse = 96023.8, mem = 48.07MB, took 91.13s
Iteration 16, sse = 95954.2, mem = 48.07MB, took 91.06s
Iteration 17, sse = 95912, mem = 48.07MB, took 91.49s
Iteration 18, sse = 95865.9, mem = 48.07MB, took 90.12s
Iteration 19, sse = 95823.3, mem = 48.07MB, took 91.58s
Iteration 20, sse = 95789.7, mem = 48.07MB, took 90.21s
Iteration 21, sse = 95751.8, mem = 48.07MB, took 91.46s
Iteration 22, sse = 95714.5, mem = 48.07MB, took 90.07s
Iteration 23, sse = 95683.2, mem = 48.07MB, took 91.90s
Iteration 24, sse = 95654, mem = 48.07MB, took 90.14s
Iteration 25, sse = 95639.5, mem = 48.07MB, took 90.82s
Iteration 26, sse = 95613.2, mem = 49.57MB, took 88.93s
Iteration 27, sse = 95585.5, mem = 49.57MB, took 88.42s
Iteration 28, sse = 95561.4, mem = 49.57MB, took 88.87s
Iteration 29, sse = 95548.8, mem = 49.57MB, took 88.80s
Iteration 30, sse = 95538, mem = 49.57MB, took 90.01s
Thu 30 Mar 15:50:51 BST 2017

trainAssign : 
Thu 30 Mar 15:50:51 BST 2017
numProc= 1
buildIndex::computeTrainAssigns: Loading cluster centres
buildIndex::computeTrainAssigns: Loading cluster centres - DONE (1.45508 ms)
buildIndex::computeTrainAssigns: Constructing NN search object
buildIndex::computeTrainAssigns: Constructing NN search object - DONE (63.304 ms)
buildIndex::computeTrainAssigns: numTrainDescs= 800000
trainAssignsManager: 2017-Mar-30 15:50:52 1 / 80
trainAssignsManager: 2017-Mar-30 15:50:52 2 / 80; time 00:00:00; left 00:00:00; avg 0.00106299 s
trainAssignsManager: 2017-Mar-30 15:50:52 3 / 80; time 00:00:00; left 00:00:00; avg 0.00232703 s
trainAssignsManager: 2017-Mar-30 15:50:52 4 / 80; time 00:00:00; left 00:00:00; avg 0.00233496 s
trainAssignsManager: 2017-Mar-30 15:50:53 8 / 80; time 00:00:00; left 00:00:09; avg 0.125718 s
trainAssignsManager: 2017-Mar-30 15:50:55 16 / 80; time 00:00:02; left 00:00:11; avg 0.173337 s
trainAssignsManager: 2017-Mar-30 15:50:57 24 / 80; time 00:00:04; left 00:00:10; avg 0.187503 s
trainAssignsManager: 2017-Mar-30 15:50:58 32 / 80; time 00:00:06; left 00:00:09; avg 0.194846 s
trainAssignsManager: 2017-Mar-30 15:51:00 40 / 80; time 00:00:07; left 00:00:08; avg 0.200759 s
trainAssignsManager: 2017-Mar-30 15:51:02 48 / 80; time 00:00:09; left 00:00:06; avg 0.202693 s
trainAssignsManager: 2017-Mar-30 15:51:03 56 / 80; time 00:00:11; left 00:00:04; avg 0.204172 s
trainAssignsManager: 2017-Mar-30 15:51:05 64 / 80; time 00:00:12; left 00:00:03; avg 0.205162 s
trainAssignsManager: 2017-Mar-30 15:51:07 72 / 80; time 00:00:14; left 00:00:01; avg 0.206696 s
trainAssignsManager: 2017-Mar-30 15:51:09 80 / 80; time 00:00:16; left 00:00:00; avg 0.20736 s
Thu 30 Mar 15:51:09 BST 2017

trainHamm : 
Thu 30 Mar 15:51:09 BST 2017
numProc= 1
buildIndex::computeHamming: Loading cluster centres
buildIndex::computeHamming: Loading cluster centres - DONE (1.40894 ms)
buildIndex::computeHamming: Reading training 100000 descriptors for PCA
buildIndex::computeHamming: Computing PCA
buildIndex::computeHamming: Computing random rotation
buildIndex::computeHamming: Done with rotation (28950.4 ms)
trainHammingManager: 2017-Mar-30 15:51:46 1 / 4
trainHammingManager: 2017-Mar-30 15:51:46 2 / 4; time 00:00:00; left 00:00:00; avg 0.319011 s
trainHammingManager: 2017-Mar-30 15:51:46 3 / 4; time 00:00:00; left 00:00:00; avg 0.250788 s
trainHammingManager: 2017-Mar-30 15:51:46 4 / 4; time 00:00:00; left 00:00:00; avg 0.169715 s
Thu 30 Mar 15:51:46 BST 2017

index : 
Thu 30 Mar 15:51:46 BST 2017
numProc= 1
buildIndex::build: beginning
buildIndex::build: Loading cluster centres
buildIndex::build: Loading cluster centres - DONE (1.13501 ms)
buildIndex::build: Constructing NN search object
buildIndex::build: Constructing NN search object - DONE (65.5049 ms)
buildManagerSemiSorted: 2017-Mar-30 15:51:52 1 / 5063
buildManagerSemiSorted: 2017-Mar-30 15:51:52 2 / 5063; time 00:00:00; left 00:25:25; avg 0.301439 s
buildManagerSemiSorted: 2017-Mar-30 15:51:56 3 / 5063; time 00:00:03; left 02:31:49; avg 1.80031 s
buildManagerSemiSorted: 2017-Mar-30 15:51:57 4 / 5063; time 00:00:04; left 02:15:42; avg 1.6095 s
buildManagerSemiSorted: 2017-Mar-30 15:52:02 8 / 5063; time 00:00:10; left 02:02:30; avg 1.4542 s
buildManagerSemiSorted: 2017-Mar-30 15:52:13 16 / 5063; time 00:00:20; left 01:55:39; avg 1.37499 s
buildManagerSemiSorted: 2017-Mar-30 15:52:35 32 / 5063; time 00:00:42; left 01:55:06; avg 1.37285 s
buildManagerSemiSorted: 2017-Mar-30 15:53:20 64 / 5063; time 00:01:28; left 01:56:51; avg 1.40264 s
buildManagerSemiSorted: 2017-Mar-30 15:55:04 128 / 5063; time 00:03:11; left 02:04:06; avg 1.50888 s
buildManagerSemiSorted: 2017-Mar-30 15:58:03 256 / 5063; time 00:06:10; left 01:56:29; avg 1.45393 s
buildManagerSemiSorted: 2017-Mar-30 16:04:16 506 / 5063; time 00:12:24; left 01:51:54; avg 1.47349 s
buildManagerSemiSorted: 2017-Mar-30 16:04:29 512 / 5063; time 00:12:37; left 01:52:22; avg 1.48146 s
buildManagerSemiSorted: 2017-Mar-30 16:16:58 1012 / 5063; time 00:25:05; left 01:40:32; avg 1.48918 s
buildManagerSemiSorted: 2017-Mar-30 16:17:14 1024 / 5063; time 00:25:22; left 01:40:10; avg 1.48811 s
buildManagerSemiSorted: 2017-Mar-30 16:29:38 1518 / 5063; time 00:37:45; left 01:28:14; avg 1.49358 s
buildManagerSemiSorted: 2017-Mar-30 16:41:49 2024 / 5063; time 00:49:56; left 01:15:01; avg 1.48129 s
buildManagerSemiSorted: 2017-Mar-30 16:42:23 2048 / 5063; time 00:50:30; left 01:14:24; avg 1.48064 s
buildManagerSemiSorted: 2017-Mar-30 16:54:15 2530 / 5063; time 01:02:23; left 01:02:29; avg 1.48016 s
buildManagerSemiSorted: 2017-Mar-30 17:06:44 3036 / 5063; time 01:14:52; left 00:50:00; avg 1.48008 s
buildManagerSemiSorted: 2017-Mar-30 17:20:35 3542 / 5063; time 01:28:42; left 00:38:06; avg 1.50324 s
buildManagerSemiSorted: 2017-Mar-30 17:33:02 4048 / 5063; time 01:41:10; left 00:25:22; avg 1.49994 s
buildManagerSemiSorted: 2017-Mar-30 17:34:20 4096 / 5063; time 01:42:27; left 00:24:11; avg 1.50124 s
buildManagerSemiSorted: 2017-Mar-30 17:45:31 4554 / 5063; time 01:53:39; left 00:12:42; avg 1.49771 s
buildManagerSemiSorted: 2017-Mar-30 17:58:14 5060 / 5063; time 02:06:22; left 00:00:04; avg 1.49874 s
buildManagerSemiSorted: 2017-Mar-30 17:58:19 5063 / 5063; time 02:06:26; left 00:00:00; avg 1.49879 s
buildIndex::build: semiSorted

state: semiSorted
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_zjt1XF.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_XnQVIj.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_bzky9l.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_DJCQqu.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_GpkYHs.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_MktsFT.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_i45LSk.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_IMOr36.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_isPbTo.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_bKVVXx.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_QemG2G.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_L3Sq7P.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_WgDbcZ.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_XNvWg8.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_WjDHlh.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_ZISsqq.bin"
totalfeats: 17564128

buildManagerSorted: 2017-Mar-30 18:46:50 1 / 8
buildManagerSorted: 2017-Mar-30 18:47:18 2 / 8; time 00:00:27; left 00:02:45; avg 27.5223 s
buildManagerSorted: 2017-Mar-30 18:47:19 3 / 8; time 00:00:29; left 00:01:12; avg 14.5179 s
buildManagerSorted: 2017-Mar-30 18:47:20 4 / 8; time 00:00:29; left 00:00:39; avg 9.93857 s
buildManagerSorted: 2017-Mar-30 18:47:36 5 / 8; time 00:00:46; left 00:00:34; avg 11.5232 s
buildManagerSorted: 2017-Mar-30 18:47:37 6 / 8; time 00:00:47; left 00:00:18; avg 9.4738 s
buildManagerSorted: 2017-Mar-30 18:47:49 7 / 8; time 00:00:58; left 00:00:09; avg 9.76962 s
buildManagerSorted: 2017-Mar-30 18:48:13 8 / 8; time 00:01:22; left 00:00:00; avg 11.8539 s
buildIndex::build: merged

state: merged
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_9e9zN0.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_z550sk.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_QuP0xf.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_E3WXTV.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_7RaE0Q.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_mFTRpp.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_XXl5Ca.bin"
filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_w1ZiI5.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_isPbTo.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_bKVVXx.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_QemG2G.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_L3Sq7P.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_WgDbcZ.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_XNvWg8.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_WjDHlh.bin"
fidx_filename: "/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_ZISsqq.bin"
totalfeats: 17564128

buildIndex::mergePartialFidx: warning no features detected in imageID=4499
buildIndex::mergePartialFidx: done in 00:00:00
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 1 / 17564128
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 2 / 17564128; time 00:00:00; left 00:24:17; avg 8.30078e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 3 / 17564128; time 00:00:00; left 00:20:22; avg 6.95801e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 4 / 17564128; time 00:00:00; left 00:14:33; avg 4.97233e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 8 / 17564128; time 00:00:00; left 00:08:02; avg 2.74484e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 16 / 17564128; time 00:00:00; left 00:04:24; avg 1.50716e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 32 / 17564128; time 00:00:00; left 00:02:26; avg 8.35591e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 64 / 17564128; time 00:00:00; left 00:01:32; avg 5.28584e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 128 / 17564128; time 00:00:00; left 00:01:04; avg 3.6698e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 256 / 17564128; time 00:00:00; left 00:00:54; avg 3.09436e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 512 / 17564128; time 00:00:00; left 00:00:49; avg 2.80642e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 1024 / 17564128; time 00:00:00; left 00:00:40; avg 2.3216e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 2048 / 17564128; time 00:00:00; left 00:00:39; avg 2.22482e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 4096 / 17564128; time 00:00:00; left 00:00:37; avg 2.15434e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 8192 / 17564128; time 00:00:00; left 00:00:37; avg 2.15139e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 16384 / 17564128; time 00:00:00; left 00:00:36; avg 2.07167e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 32768 / 17564128; time 00:00:00; left 00:00:34; avg 1.97934e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:14 65536 / 17564128; time 00:00:00; left 00:00:33; avg 1.93089e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:15 131072 / 17564128; time 00:00:00; left 00:00:32; avg 1.89059e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:15 262144 / 17564128; time 00:00:00; left 00:00:33; avg 1.94852e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:15 524288 / 17564128; time 00:00:01; left 00:00:32; avg 1.90937e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:16 1048576 / 17564128; time 00:00:01; left 00:00:31; avg 1.88551e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:18 1756412 / 17564128; time 00:00:03; left 00:00:29; avg 1.89507e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:18 2097152 / 17564128; time 00:00:03; left 00:00:29; avg 1.89282e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:21 3512824 / 17564128; time 00:00:06; left 00:00:26; avg 1.88147e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:22 4194304 / 17564128; time 00:00:07; left 00:00:25; avg 1.89293e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:24 5269236 / 17564128; time 00:00:09; left 00:00:23; avg 1.89651e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:28 7025648 / 17564128; time 00:00:13; left 00:00:20; avg 1.89811e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:30 8388608 / 17564128; time 00:00:16; left 00:00:17; avg 1.90905e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:31 8782060 / 17564128; time 00:00:16; left 00:00:16; avg 1.90738e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:34 10538472 / 17564128; time 00:00:20; left 00:00:13; avg 1.91275e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:38 12294884 / 17564128; time 00:00:23; left 00:00:10; avg 1.9141e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:41 14051296 / 17564128; time 00:00:26; left 00:00:06; avg 1.91841e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:45 15807708 / 17564128; time 00:00:30; left 00:00:03; avg 1.91485e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:48 16777216 / 17564128; time 00:00:33; left 00:00:01; avg 2.01183e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:50 17564120 / 17564128; time 00:00:35; left 00:00:00; avg 2.00895e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-30 18:48:50 17564128 / 17564128; time 00:00:35; left 00:00:00; avg 2.00895e-06 s
buildIndex::mergeSortedFiles: done in 00:00:36
buildIndex::build: done in 02:57:03

state: done
totalfeats: 17564128

Thu 30 Mar 18:48:50 BST 2017

Done
tlm@0x02:~/dev/vise$ 

```
