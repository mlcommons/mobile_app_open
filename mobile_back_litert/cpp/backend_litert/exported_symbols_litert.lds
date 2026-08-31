# Export the symbols in backend_c.h, plus the LiteRT C API.
#
# The LiteRt* line mirrors the Android version_script.lds. Inspecting the
# prebuilt 2.1.5 libLiteRtMetalAccelerator.dylib shows it is self-contained --
# it defines the LiteRt* symbols it needs and imports none of them from its
# loader -- so this is insurance for a future accelerator build that does
# import them, not a present-day requirement. It costs binary size (the LiteRT
# surface can no longer be dead-stripped); drop it once a device run confirms
# the Metal accelerator still registers without it.
_mlperf_backend_*
_LiteRt*
