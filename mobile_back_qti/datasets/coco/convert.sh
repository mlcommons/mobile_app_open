docker run \
          -v ${COCO_OUT}:/coco-out \
          -v $(pwd)/coco \
          -v ${DOWNLOADS}:/downloads \
          -u ${USERID}:${GROUPID} \
          mlcommons/mlperf_qti_backend:0.1 \
            python3 /coco/upscale_coco.py --inputs /coco-out/orig --outputs /coco-out/calibration --size 320 320

