# Performance Record

## ox5k
(time in minutes)

|          | trainDescs | cluster | trainAssign | trainHamm |   index |
|----------+------------+---------+-------------+-----------+---------|
| original |       5:50 |   48:21 |             |           | 2:56:32 |
|          |            |         |             |           |         |

### trainDescs
```
trainDescs : 
Tue 28 Mar 16:26:43 BST 2017
numProc= 1
numWorkerThreads = 8
trainDescsManager(descs): 2017-Mar-28 16:26:51 1 / 800000
trainDescsManager(descs): 2017-Mar-28 16:26:51 2 / 800000; time 00:00:00; left 00:01:52; avg 0.000141113 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 3 / 800000; time 00:00:00; left 00:01:10; avg 8.80127e-05 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 4 / 800000; time 00:00:00; left 00:00:50; avg 6.37207e-05 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 8 / 800000; time 00:00:00; left 00:00:23; avg 2.92969e-05 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 16 / 800000; time 00:00:00; left 00:00:11; avg 1.45996e-05 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 32 / 800000; time 00:00:00; left 00:00:05; avg 7.4896e-06 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 64 / 800000; time 00:00:00; left 00:00:03; avg 3.92175e-06 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 128 / 800000; time 00:00:00; left 00:00:01; avg 2.07231e-06 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 256 / 800000; time 00:00:00; left 00:00:00; avg 1.1106e-06 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 512 / 800000; time 00:00:00; left 00:00:00; avg 6.04857e-07 s
trainDescsManager(descs): 2017-Mar-28 16:26:51 1024 / 800000; time 00:00:00; left 00:00:00; avg 3.39363e-07 s
trainDescsManager(descs): 2017-Mar-28 16:26:54 2048 / 800000; time 00:00:02; left 00:19:11; avg 0.00144267 s
trainDescsManager(descs): 2017-Mar-28 16:26:54 4096 / 800000; time 00:00:02; left 00:09:34; avg 0.000721233 s
trainDescsManager(descs): 2017-Mar-28 16:26:57 8192 / 800000; time 00:00:05; left 00:08:15; avg 0.000625627 s
trainDescsManager(descs): 2017-Mar-28 16:26:57 16384 / 800000; time 00:00:05; left 00:04:05; avg 0.000312874 s
trainDescsManager(descs): 2017-Mar-28 16:27:06 32768 / 800000; time 00:00:14; left 00:05:31; avg 0.000432359 s
trainDescsManager(descs): 2017-Mar-28 16:27:21 65536 / 800000; time 00:00:29; left 00:05:35; avg 0.000457026 s
trainDescsManager(descs): 2017-Mar-28 16:27:22 80000 / 800000; time 00:00:30; left 00:04:34; avg 0.000380592 s
trainDescsManager(descs): 2017-Mar-28 16:27:47 131072 / 800000; time 00:00:55; left 00:04:45; avg 0.000427189 s
trainDescsManager(descs): 2017-Mar-28 16:27:56 160000 / 800000; time 00:01:04; left 00:04:18; avg 0.000404415 s
trainDescsManager(descs): 2017-Mar-28 16:28:31 240000 / 800000; time 00:01:39; left 00:03:51; avg 0.000413516 s
trainDescsManager(descs): 2017-Mar-28 16:28:37 262144 / 800000; time 00:01:45; left 00:03:37; avg 0.000403511 s
trainDescsManager(descs): 2017-Mar-28 16:29:08 320000 / 800000; time 00:02:16; left 00:03:24; avg 0.00042555 s
trainDescsManager(descs): 2017-Mar-28 16:29:33 400000 / 800000; time 00:02:41; left 00:02:41; avg 0.000403037 s
trainDescsManager(descs): 2017-Mar-28 16:30:05 480000 / 800000; time 00:03:13; left 00:02:09; avg 0.00040337 s
trainDescsManager(descs): 2017-Mar-28 16:30:30 524288 / 800000; time 00:03:38; left 00:01:54; avg 0.000415962 s
trainDescsManager(descs): 2017-Mar-28 16:30:42 560000 / 800000; time 00:03:51; left 00:01:39; avg 0.00041253 s
trainDescsManager(descs): 2017-Mar-28 16:31:11 640000 / 800000; time 00:04:19; left 00:01:04; avg 0.000405471 s
trainDescsManager(descs): 2017-Mar-28 16:31:41 720000 / 800000; time 00:04:49; left 00:00:32; avg 0.000402094 s
trainDescsManager(descs): 2017-Mar-28 16:32:23 800000 / 800000; time 00:05:31; left 00:00:00; avg 0.000414496 s
Tue 28 Mar 16:32:33 BST 2017
```

### cluster
```
tlm@0x02:~/dev/vise/build$ date && python ../src/v2/indexing/compute_clusters.py oxc1 /data/datasets/vise/data/image/ox5k/oxc1_visedata/conf.txt && date
Wed 29 Mar 18:06:39 BST 2017
Pypar (version 2.1.4) initialised MPI OK with 1 processors
Computing clusters:
Using a (800000 x 128) uint8 array for the datapoints
Using a (10000 x 128) float32 array for the clusters
Iteration 1, sse = 143190, mem = 41.21MB, took 111.14s
Iteration 2, sse = 102917, mem = 43.32MB, took 95.98s
Iteration 3, sse = 99744.9, mem = 48.21MB, took 96.36s
Iteration 4, sse = 98483.7, mem = 48.21MB, took 97.92s
Iteration 5, sse = 97785.6, mem = 48.21MB, took 100.67s
Iteration 6, sse = 97331.5, mem = 48.21MB, took 95.86s
Iteration 7, sse = 97016.9, mem = 48.21MB, took 95.45s
Iteration 8, sse = 96790.9, mem = 48.21MB, took 95.40s
Iteration 9, sse = 96601.4, mem = 48.21MB, took 93.65s
Iteration 10, sse = 96453.3, mem = 48.21MB, took 97.58s
Iteration 11, sse = 96339.7, mem = 48.21MB, took 95.34s
Iteration 12, sse = 96249, mem = 48.21MB, took 95.89s
Iteration 13, sse = 96164.1, mem = 48.21MB, took 92.81s
Iteration 14, sse = 96084.2, mem = 48.21MB, took 98.13s
Iteration 15, sse = 96023.8, mem = 48.21MB, took 98.43s
Iteration 16, sse = 95954.2, mem = 48.21MB, took 97.93s
Iteration 17, sse = 95912, mem = 48.21MB, took 98.72s
Iteration 18, sse = 95865.9, mem = 48.21MB, took 96.41s
Iteration 19, sse = 95823.3, mem = 48.21MB, took 97.74s
Iteration 20, sse = 95789.7, mem = 48.21MB, took 102.32s
Iteration 21, sse = 95751.8, mem = 48.21MB, took 101.71s
Iteration 22, sse = 95714.5, mem = 48.21MB, took 97.58s
Iteration 23, sse = 95683.2, mem = 49.72MB, took 95.48s
Iteration 24, sse = 95654, mem = 49.72MB, took 92.27s
Iteration 25, sse = 95639.5, mem = 49.72MB, took 91.90s
Iteration 26, sse = 95613.2, mem = 49.72MB, took 94.61s
Iteration 27, sse = 95585.5, mem = 49.72MB, took 93.25s
Iteration 28, sse = 95561.4, mem = 49.72MB, took 94.88s
Iteration 29, sse = 95548.8, mem = 49.72MB, took 92.20s
Iteration 30, sse = 95538, mem = 49.72MB, took 94.03s
Wed 29 Mar 18:55:02 BST 2017
```

### index
```
index : 
Wed 29 Mar 13:45:38 BST 2017
numProc= 1
buildIndex::build: beginning
buildIndex::build: Loading cluster centres
buildIndex::build: Loading cluster centres - DONE (1.53296 ms)
buildIndex::build: Constructing NN search object
buildIndex::build: Constructing NN search object - DONE (69.6899 ms)
buildManagerSemiSorted: 2017-Mar-29 13:45:44 1 / 5063
buildManagerSemiSorted: 2017-Mar-29 13:45:44 2 / 5063; time 00:00:00; left 00:28:06; avg 0.333197 s
buildManagerSemiSorted: 2017-Mar-29 13:45:47 3 / 5063; time 00:00:03; left 02:33:21; avg 1.81856 s
buildManagerSemiSorted: 2017-Mar-29 13:45:49 4 / 5063; time 00:00:04; left 02:17:52; avg 1.63527 s
buildManagerSemiSorted: 2017-Mar-29 13:45:54 8 / 5063; time 00:00:10; left 02:01:17; avg 1.43966 s
buildManagerSemiSorted: 2017-Mar-29 13:46:05 16 / 5063; time 00:00:20; left 01:56:15; avg 1.38207 s
buildManagerSemiSorted: 2017-Mar-29 13:46:26 32 / 5063; time 00:00:42; left 01:54:49; avg 1.36947 s
buildManagerSemiSorted: 2017-Mar-29 13:47:14 64 / 5063; time 00:01:30; left 01:59:48; avg 1.43804 s
buildManagerSemiSorted: 2017-Mar-29 13:49:00 128 / 5063; time 00:03:16; left 02:07:14; avg 1.54693 s
buildManagerSemiSorted: 2017-Mar-29 13:52:03 256 / 5063; time 00:06:19; left 01:59:05; avg 1.48648 s
buildManagerSemiSorted: 2017-Mar-29 13:58:23 506 / 5063; time 00:12:39; left 01:54:10; avg 1.50321 s
buildManagerSemiSorted: 2017-Mar-29 13:58:36 512 / 5063; time 00:12:52; left 01:54:38; avg 1.51151 s
buildManagerSemiSorted: 2017-Mar-29 14:11:15 1012 / 5063; time 00:25:30; left 01:42:13; avg 1.51412 s
buildManagerSemiSorted: 2017-Mar-29 14:11:31 1024 / 5063; time 00:25:47; left 01:41:49; avg 1.51264 s
buildManagerSemiSorted: 2017-Mar-29 14:24:07 1518 / 5063; time 00:38:23; left 01:29:41; avg 1.51815 s
buildManagerSemiSorted: 2017-Mar-29 14:36:25 2024 / 5063; time 00:50:40; left 01:16:07; avg 1.50306 s
buildManagerSemiSorted: 2017-Mar-29 14:36:59 2048 / 5063; time 00:51:14; left 01:15:29; avg 1.50216 s
buildManagerSemiSorted: 2017-Mar-29 14:48:51 2530 / 5063; time 01:03:07; left 01:03:13; avg 1.49752 s
buildManagerSemiSorted: 2017-Mar-29 15:01:18 3036 / 5063; time 01:15:33; left 00:50:27; avg 1.4938 s
buildManagerSemiSorted: 2017-Mar-29 15:13:57 3542 / 5063; time 01:28:12; left 00:37:53; avg 1.49473 s
buildManagerSemiSorted: 2017-Mar-29 15:26:18 4048 / 5063; time 01:40:34; left 00:25:13; avg 1.49103 s
buildManagerSemiSorted: 2017-Mar-29 15:27:30 4096 / 5063; time 01:41:45; left 00:24:01; avg 1.49105 s
buildManagerSemiSorted: 2017-Mar-29 15:38:37 4554 / 5063; time 01:52:53; left 00:12:37; avg 1.48764 s
buildManagerSemiSorted: 2017-Mar-29 15:50:56 5060 / 5063; time 02:05:12; left 00:00:04; avg 1.48496 s
buildManagerSemiSorted: 2017-Mar-29 15:51:00 5063 / 5063; time 02:05:16; left 00:00:00; avg 1.48489 s
buildIndex::build: semiSorted

state: semiSorted
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_w7vgOg.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_oFjBQh.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_8SoFTp.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_S2suMo.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_emO7HZ.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_016TkO.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_izLlRM.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/semisortedpart_80EeLj.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_6XSwvm.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_Wg4M4C.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_4Zv3DT.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_4Aakda.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_ua0AMq.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_wNZRlH.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_gum9UX.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_sC4que.bin"
totalfeats: 17564128

buildManagerSorted: 2017-Mar-29 16:40:27 1 / 8
buildManagerSorted: 2017-Mar-29 16:40:31 2 / 8; time 00:00:04; left 00:00:27; avg 4.6194 s
buildManagerSorted: 2017-Mar-29 16:40:37 3 / 8; time 00:00:10; left 00:00:25; avg 5.02458 s
buildManagerSorted: 2017-Mar-29 16:40:58 4 / 8; time 00:00:31; left 00:00:41; avg 10.406 s
buildManagerSorted: 2017-Mar-29 16:41:04 5 / 8; time 00:00:37; left 00:00:28; avg 9.37277 s
buildManagerSorted: 2017-Mar-29 16:41:17 6 / 8; time 00:00:49; left 00:00:19; avg 9.99968 s
buildManagerSorted: 2017-Mar-29 16:41:26 7 / 8; time 00:00:59; left 00:00:09; avg 9.86317 s
buildManagerSorted: 2017-Mar-29 16:41:30 8 / 8; time 00:01:03; left 00:00:00; avg 9.07123 s
buildIndex::build: merged

state: merged
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_mAIi9A.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_QQdyiC.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_i73pEL.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_gQadKr.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_GFvfaW.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_CD0UEq.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_69QMch.bin"
filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/sortedpart_eIh8F6.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_6XSwvm.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_Wg4M4C.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_4Zv3DT.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_4Aakda.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_ua0AMq.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_wNZRlH.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_gum9UX.bin"
fidx_filename: "/home/tlm/data/datasets/vise/data/image/ox5k/oxc1/../oxc1_visedata/tmp/fidxpart_sC4que.bin"
totalfeats: 17564128

buildIndex::mergePartialFidx: warning no features detected in imageID=4499
buildIndex::mergePartialFidx: done in 00:00:00
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 1 / 17564128
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 2 / 17564128; time 00:00:00; left 00:17:52; avg 6.10352e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 3 / 17564128; time 00:00:00; left 00:16:49; avg 5.74951e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 4 / 17564128; time 00:00:00; left 00:14:31; avg 4.96419e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 8 / 17564128; time 00:00:00; left 00:07:39; avg 2.61579e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 16 / 17564128; time 00:00:00; left 00:04:24; avg 1.50716e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 32 / 17564128; time 00:00:00; left 00:03:01; avg 1.03563e-05 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 64 / 17564128; time 00:00:00; left 00:01:45; avg 5.99888e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 128 / 17564128; time 00:00:00; left 00:01:11; avg 4.07926e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 256 / 17564128; time 00:00:00; left 00:00:57; avg 3.28585e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 512 / 17564128; time 00:00:00; left 00:00:48; avg 2.75721e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 1024 / 17564128; time 00:00:00; left 00:00:40; avg 2.32351e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 2048 / 17564128; time 00:00:00; left 00:00:40; avg 2.32679e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 4096 / 17564128; time 00:00:00; left 00:00:38; avg 2.2071e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 8192 / 17564128; time 00:00:00; left 00:00:40; avg 2.32498e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 16384 / 17564128; time 00:00:00; left 00:00:39; avg 2.23293e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 32768 / 17564128; time 00:00:00; left 00:00:37; avg 2.12192e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:31 65536 / 17564128; time 00:00:00; left 00:00:36; avg 2.09011e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:32 131072 / 17564128; time 00:00:00; left 00:00:35; avg 2.02946e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:32 262144 / 17564128; time 00:00:00; left 00:00:35; avg 2.06742e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:32 524288 / 17564128; time 00:00:01; left 00:00:34; avg 2.01429e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:33 1048576 / 17564128; time 00:00:02; left 00:00:32; avg 1.98817e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:35 1756412 / 17564128; time 00:00:03; left 00:00:31; avg 1.98638e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:36 2097152 / 17564128; time 00:00:04; left 00:00:30; avg 1.97855e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:38 3512824 / 17564128; time 00:00:06; left 00:00:27; avg 1.97037e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:40 4194304 / 17564128; time 00:00:08; left 00:00:26; avg 1.97945e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:42 5269236 / 17564128; time 00:00:10; left 00:00:24; avg 1.98915e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:45 7025648 / 17564128; time 00:00:13; left 00:00:20; avg 1.98665e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:48 8388608 / 17564128; time 00:00:16; left 00:00:18; avg 1.99934e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:49 8782060 / 17564128; time 00:00:17; left 00:00:17; avg 1.99935e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:52 10538472 / 17564128; time 00:00:21; left 00:00:14; avg 1.99955e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:56 12294884 / 17564128; time 00:00:24; left 00:00:10; avg 1.99426e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:41:59 14051296 / 17564128; time 00:00:28; left 00:00:07; avg 1.99569e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:42:07 15807708 / 17564128; time 00:00:35; left 00:00:03; avg 2.24564e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:42:09 16777216 / 17564128; time 00:00:37; left 00:00:01; avg 2.22858e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:42:10 17564120 / 17564128; time 00:00:38; left 00:00:00; avg 2.22007e-06 s
buildIndex::mergeSortedFiles: 2017-Mar-29 16:42:10 17564128 / 17564128; time 00:00:38; left 00:00:00; avg 2.22007e-06 s
buildIndex::mergeSortedFiles: done in 00:00:39
buildIndex::build: done in 02:56:32

state: done
totalfeats: 17564128

Wed 29 Mar 16:42:10 BST 2017

Done
```
