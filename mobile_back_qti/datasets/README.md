# Datasets

This Makefile will create the calibration data required for the DLCs.

If you already have the dataset assets downloaded, you can copy them to:

```
mobile_app_open/output/
├── ade20k
│   └── downloads
│       ├── ADE20K_2016_07_26.zip
│       └── ADEChallengeData2016.zip
├── coco
│   └── downloads
│       ├── annotations_trainval2017.zip
│       ├── val2014.zip
│       └── val2017.zip
└── imagenet
    └── downloads
        └── LSVRC2012_img_val.tar
```

The Makefile can also download the assets except for Imagenet, which must be
provided manually.
